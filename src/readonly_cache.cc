/*
Copyright (c) <2012>, <Georgia Institute of Technology> All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted 
provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions 
and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list of 
conditions and the following disclaimer in the documentation and/or other materials provided 
with the distribution.

Neither the name of the <Georgia Institue of Technology> nor the names of its contributors 
may be used to endorse or promote products derived from this software without specific prior 
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
*/


/**********************************************************************************************
 * File         : readonly_cache.cc
 * Author       : Nagesh BL
 * Date         : 03/03/2010 
 * SVN          : $Id: cache.h,
 * Description  : readonly cache
 *********************************************************************************************/


/*
 * Summary: Readonly cache
 *
 * FIXME
 *  better design? readonly_cache and sw_managed_cache have very similar structure
 *  One can be case class
 *
 *  BANK / PORT design? : bank/port is not modeled in readonly cache
 */

#include "assert_macros.h"
#include "core.h"
#include "frontend.h"
#include "memory.h"
#include "memreq_info.h"
#include "readonly_cache.h"
#include "statistics.h"
#include "uop.h"
#include "utils.h"

#include "debug_macros.h"

#include "all_knobs.h"

////////////////////////////////////////////////////////////////////////////////


#define DEBUG_MEM(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_MEM_TRACE, ## args)
#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_DCU_STAGE, ## args)


////////////////////////////////////////////////////////////////////////////////


bool readonly_cache_fill_line_wrapper(mem_req_s* req);


///////////////////////////////////////////////////////////////////////////////////////////////


// constructor
readonly_cache_c::readonly_cache_c(string name, int c_id, uns32 c_size, uns8 c_assoc, 
    uns8 c_line_size, uns8 c_banks, uns8 c_cycles, bool by_pass, Cache_Type c_type, 
    int c_data_size, macsim_c* simBase) : 
  m_core_id(c_id), m_cache_size(c_size), m_cache_assoc(c_assoc), 
  m_cache_line_size(c_line_size), m_cache_banks(c_banks), m_cache_cycles(c_cycles), 
  m_cache_type(c_type)
{
  m_simBase = simBase;

  int num_set = c_size / c_assoc / c_line_size;
  // allocate cache
  m_cache = new cache_c(name, num_set, m_cache_assoc, m_cache_line_size, c_data_size, 
      m_cache_banks, by_pass, m_core_id, m_cache_type, false, simBase);
}


// destructor
readonly_cache_c::~readonly_cache_c()
{
  delete m_cache;
}


// access read-only cache
int readonly_cache_c::load(uop_c *uop)
{
  Addr vaddr = uop->m_vaddr;
  Addr line_addr;
  void *line_data = NULL;

  // access the cache
  int appl_id = m_simBase->m_core_pointers[uop->m_core_id]->get_appl_id(uop->m_thread_id);
  line_data = m_cache->access_cache(vaddr, &line_addr, true, appl_id);


  // only read acesses!!!
  if (uop->m_mem_type == MEM_LD_CM) {
	  POWER_CORE_EVENT(uop->m_core_id, POWER_CONST_CACHE_R);
	  POWER_CORE_EVENT(uop->m_core_id, POWER_CONST_CACHE_R_TAG);
  }
  else {
	  POWER_CORE_EVENT(uop->m_core_id, POWER_TEXTURE_CACHE_R);
	  POWER_CORE_EVENT(uop->m_core_id, POWER_TEXTURE_CACHE_R_TAG);
  }

  // cache miss
  if (line_data == NULL) {
    DEBUG_MEM("core_id:%d tid:%d addr:%s type:%d [readonly cache miss]\n",
       uop->m_core_id, uop->m_thread_id, hexstr64s(line_addr), uop->m_mem_type); 
    Mem_Req_Type req_type = MRT_DFETCH;

    // generate a new request
    int req_status = m_simBase->m_memory->new_mem_req(req_type, line_addr, uop->m_mem_size,
        m_cache_cycles - 1 + *KNOB(KNOB_EXTRA_LD_LATENCY), uop, 
        readonly_cache_fill_line_wrapper, uop->m_unique_num, NULL, m_core_id, 
        uop->m_thread_id, 1);

    if (!req_status) {
      return 0;
    }
    else {
      return -1;
    }
  }
  // cache hit
  else {
    DEBUG_MEM("core_id:%d tid:%d addr:%s type:%d [readonly cache hit]\n",
       uop->m_core_id, uop->m_thread_id, hexstr64s(line_addr), uop->m_mem_type); 
    return m_cache_cycles; 
  }
}


// fills read-only cache with cache block fetched from memory on a cache miss
bool readonly_cache_fill_line_wrapper(mem_req_s* req)
{
  bool result = true;
  macsim_c* m_simBase = req->m_simBase;
  list<mem_req_s*> done_list;
  for (auto I = req->m_merge.begin(), E = req->m_merge.end(); I != E; ++I) {
    if ((*I)->m_done_func && !((*I)->m_done_func((*I)))) {
      result = false;
      continue;
    }
    done_list.push_back((*I));
  }

  // process merged requests
  for (auto I = done_list.begin(), E = done_list.end(); I != E; ++I) {
    req->m_merge.remove((*I));
    m_simBase->m_memory->free_req((*I)->m_core_id, (*I));
  }

  // process the request
  if (result) {
    ASSERT(req->m_uop);
    if (req->m_uop->m_mem_type == MEM_LD_CM) {
      result = m_simBase->m_core_pointers[req->m_core_id]->get_const_cache()->cache_fill_line(req);
    }
    else if (req->m_uop->m_mem_type == MEM_LD_TM) {
      result = m_simBase->m_core_pointers[req->m_core_id]->get_texture_cache()->cache_fill_line(req);
    }
    else
      ASSERT(0);
  }

  return result;
}


// fill a cache line
bool readonly_cache_c::cache_fill_line(mem_req_s *req)
{
  Addr line_addr;
  Addr repl_line_addr;

  // insert cache
  m_cache->insert_cache(req->m_addr, &line_addr, &repl_line_addr, req->m_appl_id, req->m_ptx);

  if (req->m_uop) {
    uop_c* uop = req->m_uop;

    DEBUG("uop:%lld done\n", uop->m_uop_num);
    uop->m_done_cycle = m_simBase->m_core_cycle[uop->m_core_id] + 1;
    uop->m_state = OS_SCHEDULED;

    if (uop->m_mem_type == MEM_LD_CM) {
      POWER_CORE_EVENT(uop->m_core_id, POWER_CONST_CACHE_W);
      POWER_CORE_EVENT(uop->m_core_id, POWER_CONST_CACHE_W_TAG);
    }
    else {
      POWER_CORE_EVENT(uop->m_core_id, POWER_TEXTURE_CACHE_W);
      POWER_CORE_EVENT(uop->m_core_id, POWER_TEXTURE_CACHE_W_TAG);
    }

    // multiple uops case : let parent uop know
    if (uop->m_parent_uop) {
      uop_c* puop = uop->m_parent_uop;
      ++puop->m_num_child_uops_done;
      if (puop->m_num_child_uops_done == puop->m_num_child_uops) {
        if (*m_simBase->m_knobs->KNOB_FETCH_ONLY_LOAD_READY) {
          // set load ready
          core_c* core = m_simBase->m_core_pointers[puop->m_core_id];
          core->get_frontend()->set_load_ready(puop->m_thread_id, puop->m_uop_num);
        }

        puop->m_done_cycle = m_simBase->m_core_cycle[uop->m_core_id] + 1;;
        puop->m_state = OS_SCHEDULED;
      }
    } // uop->m_parent_uop
    else {
      if (*m_simBase->m_knobs->KNOB_FETCH_ONLY_LOAD_READY) {
        // set load ready
        core_c* core = m_simBase->m_core_pointers[uop->m_core_id];
        core->get_frontend()->set_load_ready(uop->m_thread_id, uop->m_uop_num);
      }
    }
  }

  return true;
}


// get cache line address
Addr readonly_cache_c::base_cache_line(Addr addr)
{
  return m_cache->base_cache_line(addr);
}


// get cache line size
uns8 readonly_cache_c::cache_line_size(void)
{
  return m_cache_line_size;
}
