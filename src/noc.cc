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
 * File         : noc.h 
 * Author       : Jaekyu Lee
 * Date         : 3/4/2011
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : Interface to interconnection network
 *********************************************************************************************/


#include "debug_macros.h"
#include "memory.h"
#include "noc.h"

#include "all_knobs.h"

#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_MEM, ## args)


// noc_c constructor
noc_c::noc_c(macsim_c* simBase)
{
  m_simBase = simBase;

  m_pool = new pool_c<noc_entry_s>(1000, "nocpool");
  m_cpu_entry_up = new list<noc_entry_s*>;
  m_cpu_entry_down = new list<noc_entry_s*>;
  if (*m_simBase->m_knobs->KNOB_HETERO_NOC_USE_SAME_QUEUE) {
    m_gpu_entry_up = m_cpu_entry_up;
    m_gpu_entry_down = m_cpu_entry_down;
  }
  else {
    m_gpu_entry_up = new list<noc_entry_s*>;
    m_gpu_entry_down = new list<noc_entry_s*>;
  }

}


// noc_c destructor
noc_c::~noc_c()
{
}


// insert a new request to NoC
bool noc_c::insert(int src, int dst, int msg, mem_req_s* req)
{
  noc_entry_s* new_entry = m_pool->acquire_entry();
 
  if (src > dst) {
    if (req->m_ptx == true)
      m_cpu_entry_up->push_back(new_entry);
    else
      m_gpu_entry_up->push_back(new_entry);
  }
  else {
    if (req->m_ptx == true)
      m_cpu_entry_down->push_back(new_entry);
    else
      m_gpu_entry_down->push_back(new_entry);
  }

  new_entry->m_src = src;
  new_entry->m_dst = dst;
  new_entry->m_msg = msg;
  new_entry->m_req = req;
  new_entry->m_rdy = CYCLE + 10; 

  return true;
}


// tick a cycle
void noc_c::run_a_cycle()
{
  list<noc_entry_s*> done_list;
  memory_c* memory = m_simBase->m_memory;

  for (auto itr = m_cpu_entry_up->begin(); itr != m_cpu_entry_up->end(); ++itr) {
    if ((*itr)->m_rdy <= CYCLE) {
      done_list.push_back((*itr));
    }
    else
      break;
  }

  for (auto itr = done_list.begin(); itr != done_list.end(); ++itr) {
    if (memory->receive((*itr)->m_src, (*itr)->m_dst, (*itr)->m_msg, (*itr)->m_req)) {
      m_cpu_entry_up->remove((*itr));
      m_pool->release_entry((*itr));
    }
  }

  done_list.clear();
  
  if (!*m_simBase->m_knobs->KNOB_HETERO_NOC_USE_SAME_QUEUE) {
    for (auto itr = m_gpu_entry_up->begin(); itr != m_gpu_entry_up->end(); ++itr) {
      if ((*itr)->m_rdy <= CYCLE) {
        done_list.push_back((*itr));
      }
      else
        break;
    }

    for (auto itr = done_list.begin(); itr != done_list.end(); ++itr) {
      if (memory->receive((*itr)->m_src, (*itr)->m_dst, (*itr)->m_msg, (*itr)->m_req)) {
        m_gpu_entry_up->remove((*itr));
        m_pool->release_entry((*itr));
      }
    }

    done_list.clear();
  }


  for (auto itr = m_cpu_entry_down->begin(); itr != m_cpu_entry_down->end(); ++itr) {
    if ((*itr)->m_rdy <= CYCLE) {
      done_list.push_back((*itr));
    }
    else
      break;
  }

  for (auto itr = done_list.begin(); itr != done_list.end(); ++itr) {
    if (memory->receive((*itr)->m_src, (*itr)->m_dst, (*itr)->m_msg, (*itr)->m_req)) {
      m_cpu_entry_down->remove((*itr));
      m_pool->release_entry((*itr));
    }
  }

  done_list.clear();
  
  if (!*m_simBase->m_knobs->KNOB_HETERO_NOC_USE_SAME_QUEUE) {
    for (auto itr = m_gpu_entry_down->begin(); itr != m_gpu_entry_down->end(); ++itr) {
      if ((*itr)->m_rdy <= CYCLE) {
        done_list.push_back((*itr));
      }
      else
        break;
    }

    for (auto itr = done_list.begin(); itr != done_list.end(); ++itr) {
      if (memory->receive((*itr)->m_src, (*itr)->m_dst, (*itr)->m_msg, (*itr)->m_req)) {
        m_gpu_entry_down->remove((*itr));
        m_pool->release_entry((*itr));
      }
    }

    done_list.clear();
  }
}
