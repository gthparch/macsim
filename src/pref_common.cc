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


/***************************************************************************************
 * File         : pref_common.cc
 * Author       : HPArch
 * Date         : 11/30/2004
 * CVS          : $Id: pref_common.cc,v 1.2 2008/09/12 03:42:01 kacear Exp $:
 * Description  : Common framework for working with prefetchers - less stuff to *mess* with
 ***************************************************************************************/


/*
 * Summary: Hardware Prefetcher Framework
 *
 * Multiple prefetchers can be run together.
 */


#include <cmath>
#include <iostream>
#include <cstring>

#include "global_defs.h"
#include "global_types.h"
#include "debug_macros.h"

#include "utils.h"
#include "assert_macros.h"
#include "uop.h"
#include "cache.h"
#include "statistics.h"
#include "pref_common.h"
#include "pref.h"
#include "core.h"
#include "pref_factory.h"
#include "knob.h"
#include "memory.h"
#include "pref_stride.h"

#include "all_knobs.h"

///////////////////////////////////////////////////////////////////////////////////////////////


#define DEBUG(args...)		_DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_PREF, ## args)
#define DEBUG_MEM(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_MEM, ## args)


///////////////////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////////////////


int  pref_compare_hwp_priority(const void * const a, const void * const b)
{
  // FIXME
  return 0;
  //return (((HWP*)a)->hwp_info->priority - ((HWP*)b)->hwp_info->priority);
}

// FIXME
/*J
int  pref_compare_prefloadhash(const void * const a, const void * const b)
{
  Pref_LoadPCInfo ** dataA = (Pref_LoadPCInfo ** ) a;
  Pref_LoadPCInfo ** dataB = (Pref_LoadPCInfo ** ) b;
    
  return ((*dataB)->count - (*dataA)->count);
}
J*/


///////////////////////////////////////////////////////////////////////////////////////////////


// constructor
hwp_common_c::hwp_common_c()
{
  // do not implement
}


// constructor
hwp_common_c::hwp_common_c(int cid, Unit_Type type, macsim_c* simBase)
{
  core_id = cid;
  m_simBase = simBase;

  // allocate all registered prefetchers
  pref_factory_c::get()->allocate_pref(pref_table, this, type, m_simBase);
  m_shift_bit = LOG2_DCACHE_LINE_SIZE;
}


// destructor
hwp_common_c::~hwp_common_c()
{
  delete[] m_l1req_queue;
  delete[] m_l2req_queue;

  if (*m_simBase->m_knobs->KNOB_PREF_REGION_ON) {
    for (int ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_NUMTRACKING_REGIONS; ++ii) {
      delete[] region_info[ii].status;
    }
    delete[] region_info;
  }
  
  if (*m_simBase->m_knobs->KNOB_PREF_POLBV_ON) {
    delete[] m_polbv_info;
  }
}


// initialize prefetching framework
// initialize all prefetchers enabled
void hwp_common_c::pref_init(bool ptx)
{
  static char pref_trace_filename[] = "mem_trace";
  static char pref_acc_filename[]   = "pref_acc";

  if (!*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON)
    return;

  knob_ptx_sim = ptx;

  // initialize queues
  m_l1req_queue = new pref_mem_req_s[*m_simBase->m_knobs->KNOB_PREF_DL0REQ_QUEUE_SIZE];
  m_l2req_queue = new pref_mem_req_s[*m_simBase->m_knobs->KNOB_PREF_UL1REQ_QUEUE_SIZE];
    
  m_l1req_queue_req_pos  = -1;
  m_l1req_queue_send_pos = 0;
  m_l2req_queue_req_pos  = -1;
  m_l2req_queue_send_pos = 0;
  m_update_acc           = false;


  // Accuracy Studies : Sample at periodic intervals and come up a histogram of usage at
  // different accuracies.
  if (*m_simBase->m_knobs->KNOB_PREF_ACC_STUDY) {
    PREF_ACC_OUT = file_tag_fopen(pref_acc_filename, "w", m_simBase);	
  }

  // initialize each hardware prefetcher 
  for (unsigned int ii = 0; ii < pref_table.size(); ++ii) {
    pref_table[ii]->init_func(core_id);
  }

  // FIXME
  // call init for each prefetcher
  // qsort(pref_table, pref_table.size(), sizeof(HWP), pref_compare_hwp_priority);

  if (*m_simBase->m_knobs->KNOB_PREF_TRACE_ON)
    PREF_TRACE_OUT = file_tag_fopen(pref_trace_filename,"w", m_simBase);

  if (*m_simBase->m_knobs->KNOB_PREF_DEGFB_STATPHASEFILE) {
    PREF_DEGFB_FILE = file_tag_fopen("prefdefbstats.out", "w", m_simBase);
  }

  m_overall_l1useful      = 0;
  m_overall_l1sent        = 0;
  m_overall_l2useful      = 0;
  m_overall_l2sent        = 0;
  m_l2_misses             = 0;
  m_curr_l2_misses        = 0;
  m_num_uselesspref_evict = 0;

  // Initialize hybrid structures : IF we are doing hybrid prefetching, note that 
  // add_l2_req_queue will not send the request out if the prefetcher is not the selected 
  // one for the region
  m_default_prefetcher = *m_simBase->m_knobs->KNOB_PREF_HYBRID_DEFAULT;
  m_last_update_time   = m_simBase->m_simulation_cycle;

  // region
  if (*m_simBase->m_knobs->KNOB_PREF_REGION_ON) {
    region_info = new pref_region_info_s[*m_simBase->m_knobs->KNOB_PREF_NUMTRACKING_REGIONS];
    ASSERT(region_info);
    for (int ii = 0; ii < *KNOB(KNOB_PREF_NUMTRACKING_REGIONS); ++ii) {
      region_info[ii].status = new pref_region_line_status_s[*m_simBase->m_knobs->KNOB_PREF_REGION_SIZE];
      ASSERT(region_info[ii].status);	    
      region_info[ii].trained  = false;
      region_info[ii].last_access = 0;
      region_info[ii].valid    = false;
    }
    m_region_sent        = 0;
    m_region_useful      = 0;
    m_curr_pfpol         = 0;
    m_curr_region_sent   = 0;
    m_curr_region_useful = 0;
  }
    
  // pollution bit vector
  if (*m_simBase->m_knobs->KNOB_PREF_POLBV_ON) {
    m_polbv_info = new char[*m_simBase->m_knobs->KNOB_PREF_POLBV_SIZE];
  }

  m_pfpol         = 0;
  m_num_l2_evicts = 0;
  m_useful        = 0;
  m_phase         = 0;
}


// When a prefetch request has been serviced done function will be called.
void hwp_common_c::pref_done(void)
{
  if (!*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON)
    return;

  if (*m_simBase->m_knobs->KNOB_PREF_ANALYZE_LOAD) {
  }

  for (unsigned int ii = 0; ii < pref_table.size(); ++ii) {
    if (pref_table[ii]->hwp_info->enabled && *m_simBase->m_knobs->KNOB_PREF_ACC_STUDY) {
      fprintf(PREF_ACC_OUT, "Pref: %s\n", pref_table[ii]->name.c_str());
      fprintf(PREF_ACC_OUT, "Buckets       : ");
      for (int kk = 0; kk < PREF_TRACKERS_NUM; ++kk) {
        fprintf(PREF_ACC_OUT, "%d      ", kk);
      }
		    
      for (int jj = 0; jj < 10; ++jj) {
        fprintf(PREF_ACC_OUT, "\nAccuracy  %d   : ", jj);
        for (int kk = 0; kk < PREF_TRACKERS_NUM; ++kk) {
          fprintf(PREF_ACC_OUT, "%llu      ", pref_table[ii]->hwp_info->trackhist[jj][kk]);
        }
      }
      fprintf(PREF_ACC_OUT, "\n");
    }

    if (pref_table[ii]->hwp_info->enabled && pref_table[ii]->done) {
      pref_table[ii]->done_func();
    }
  }
}


// L1 miss handler
void hwp_common_c::pref_l1_miss(int tid, Addr line_addr, Addr load_PC, uop_c *uop)
{
  if (!*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON)
    return;

  if (*m_simBase->m_knobs->KNOB_PREF_DL0_MISS_ON) {
    for (unsigned int ii = 0; ii < pref_table.size(); ++ii) {
      if (pref_table[ii]->hwp_info->enabled && pref_table[ii]->l1_miss) {
        if (*m_simBase->m_knobs->KNOB_PREF_TRAIN_INST_ONCE) {
          if (m_last_inst_num.find(tid) == m_last_inst_num.end() ||
              m_last_inst_num[tid] != uop->m_inst_num) {
            pref_table[ii]->l1_miss_func(tid, line_addr, load_PC, uop);
            m_last_inst_num[tid] = uop->m_inst_num;
          }
        }
        else {
          pref_table[ii]->l1_miss_func(tid, line_addr, load_PC, uop);
        }
      }
    }
  }
}


// L1 hit handler
void hwp_common_c::pref_l1_hit(int tid, Addr line_addr, Addr load_PC, uop_c *uop)
{
  if (!*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON)
    return;

  if (*m_simBase->m_knobs->KNOB_PREF_DL0_HIT_ON) {
    for (unsigned int ii = 0; ii < pref_table.size(); ++ii) {
      if (pref_table[ii]->hwp_info->enabled && pref_table[ii]->l1_hit) {
        if (*m_simBase->m_knobs->KNOB_PREF_TRAIN_INST_ONCE) {
          if (m_last_inst_num.find(tid) == m_last_inst_num.end() ||
              m_last_inst_num[tid] != uop->m_inst_num) {
            pref_table[ii]->l1_hit_func(tid, line_addr, load_PC, uop);
            m_last_inst_num[tid] = uop->m_inst_num;
          }
        }
        else {
          pref_table[ii]->l1_hit_func(tid, line_addr, load_PC, uop);
        }
      }    
    }
  }
}


// L1 prefetch hit handler
void hwp_common_c::pref_l1_pref_hit(int tid, Addr line_addr, Addr load_PC, uns8 prefetcher_id, 
    uop_c *uop)
{
  if (!*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON)
    return;

  if (prefetcher_id == 0)
    return;
	
  m_overall_l1useful++;
    
  if (*m_simBase->m_knobs->KNOB_PREF_DL0_HIT_ON) {
    for (unsigned int ii = 0; ii < pref_table.size(); ++ii) {
      if (pref_table[ii]->hwp_info->enabled && pref_table[ii]->l1_pref_hit) {
        pref_table[ii]->l1_pref_hit_func(tid, line_addr, load_PC, uop);
      }
    }    
  }
}


// L2 miss handler
void hwp_common_c::pref_l2_miss(int tid, Addr line_addr, uop_c* uop)
{
  Addr load_PC = uop ? uop->m_pc : 0;
  if (!*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON)
    return;

  m_curr_l2_misses++;

  if (*m_simBase->m_knobs->KNOB_PREF_TRACE_ON)
    fprintf(PREF_TRACE_OUT, "%s \t %s \t %s \t %s\n", 
        hexstr64s(m_simBase->m_simulation_cycle), hexstr64s(load_PC), hexstr64s(line_addr), "UL1_MISS");

  if (*m_simBase->m_knobs->KNOB_PREF_REGION_ON)
    pref_update_regioninfo(line_addr, false, true, false, 0, 0);

  if (*m_simBase->m_knobs->KNOB_PREF_ACC_STUDY) 
    pref_acc_useupdate(line_addr);

  if (*m_simBase->m_knobs->KNOB_PREF_POLBV_ON) {
    // UPDATE prefpolbv reset entry
    Addr line_index = (line_addr>>m_shift_bit);
    if ((*pref_polbv_access(line_index))==1) {
      m_curr_pfpol++;
      STAT_EVENT(PREF_PFPOL);
    }
    *pref_polbv_access(line_index) = 0;
  }

  for (unsigned int ii = 0; ii < pref_table.size(); ++ii) {
    if (pref_table[ii]->hwp_info->enabled && pref_table[ii]->l2_miss) { 
      if (*m_simBase->m_knobs->KNOB_PREF_TRAIN_INST_ONCE) {
        if (m_last_inst_num.find(tid) == m_last_inst_num.end() ||
            m_last_inst_num[tid] != uop->m_inst_num) {
          pref_table[ii]->l2_miss_func(tid, line_addr, load_PC, uop);
          m_last_inst_num[tid] = uop->m_inst_num;
        }
      }
      else {
        pref_table[ii]->l2_miss_func(tid, line_addr, load_PC, uop);
      }
    }
  }
}


// L2 hit handler
void hwp_common_c::pref_l2_hit(int tid, Addr line_addr, Addr load_PC, uop_c *uop)
{
  if (!*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON)
    return;

  if (*m_simBase->m_knobs->KNOB_PREF_TRACE_ON) 
    fprintf(PREF_TRACE_OUT, "%s \t %s \t %s \t %s\n", 
        hexstr64s(m_simBase->m_simulation_cycle), hexstr64s(0), hexstr64s(line_addr), "UL1_HIT");

  if (*m_simBase->m_knobs->KNOB_PREF_REGION_ON) 
    pref_update_regioninfo(line_addr, true, false, false, 0, 0);

  if (*m_simBase->m_knobs->KNOB_PREF_ACC_STUDY) 
    pref_acc_useupdate(line_addr);

  for (unsigned int ii = 0; ii < pref_table.size(); ++ii) { 
    if (pref_table[ii]->hwp_info->enabled && pref_table[ii]->l2_hit) { 
      if (*m_simBase->m_knobs->KNOB_PREF_TRAIN_INST_ONCE) {
        if (m_last_inst_num.find(tid) == m_last_inst_num.end() ||
            m_last_inst_num[tid] != uop->m_inst_num) {
          pref_table[ii]->l2_hit_func(tid, line_addr, load_PC, uop);
          m_last_inst_num[tid] = uop->m_inst_num;
        }
      }
      else {
        pref_table[ii]->l2_hit_func(tid, line_addr, load_PC, uop);
      }
    }
  }
}


// L2 late prefetch handler
void hwp_common_c::pref_l2_pref_hit_late(int tid, Addr line_addr, Addr load_PC, 
    uns8 prefetcher_id, uop_c *uop)
{
  if (!*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON)
    return;

  if (prefetcher_id == 0)
    return;
    
  m_curr_l2_misses--; // We counted this as a miss earlier...
  pref_table[prefetcher_id]->hwp_info->curr_late++;

  pref_l2_pref_hit(tid, line_addr, load_PC, prefetcher_id, uop);
}


// L2 prefetch hit handler
void hwp_common_c::pref_l2_pref_hit(int tid, Addr line_addr, Addr load_PC, uns8 prefetcher_id, 
    uop_c *uop)
{
  if (prefetcher_id == 0)
    return;

  if (!*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON)
    return;

  if (*m_simBase->m_knobs->KNOB_PREF_TRACE_ON) 
    fprintf(PREF_TRACE_OUT, "%s \t %s \t %s \t %s\n", 
        hexstr64s(m_simBase->m_simulation_cycle), hexstr64s(0), hexstr64s(line_addr), "UL1_PREFHIT");

  if (!*m_simBase->m_knobs->KNOB_PREF_USEREGION_TOCALC_ACC) {
    m_overall_l2useful++;

    pref_table[prefetcher_id]->hwp_info->curr_useful++;
  }

  for (unsigned int ii = 0; ii < pref_table.size(); ++ii) {
    if (pref_table[ii]->hwp_info->enabled && pref_table[ii]->l2_pref_hit) {
      pref_table[ii]->l2_pref_hit_func(tid, line_addr, load_PC, uop);
    }    
  }
}


// L2 hit handler
void hwp_common_c::pref_l2_hit (mem_req_s *req)
{
  if (!*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON)
    return;

  if ((req->m_type == MRT_DFETCH) || (req->m_type == MRT_DSTORE)) {
    Addr line_addr = req->m_addr;
    Addr loadPC    = req->m_pc;
		
    pref_l2_hit(req->m_thread_id, line_addr, loadPC, NULL);
  }
}


// Filter l1 request queue with the address
bool hwp_common_c::pref_l1req_queue_filter(Addr line_addr)
{
  if (!*m_simBase->m_knobs->KNOB_PREF_DL0REQ_QUEUE_FILTER_ON)
    return false; 

  for (int ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_DL0REQ_QUEUE_SIZE; ++ii) {
    if (m_l1req_queue[ii].valid && 
        (m_l1req_queue[ii].line_addr >> m_shift_bit) == (line_addr >> m_shift_bit)) {
      m_l1req_queue[ii].valid = false;
      STAT_EVENT(PREF_DL0REQ_QUEUE_HIT_BY_DEMAND);

      return true;
    }
  }
  return false;
}


// Filter l2 request queue with the address
bool hwp_common_c::pref_l2req_queue_filter(Addr line_addr)
{
  if (!*m_simBase->m_knobs->KNOB_PREF_UL1REQ_QUEUE_FILTER_ON)  
    return false; 

  for (int ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_UL1REQ_QUEUE_SIZE; ++ii) {
    if (m_l2req_queue[ii].valid && 
        (m_l2req_queue[ii].line_addr >> m_shift_bit) == (line_addr >> m_shift_bit)) {
      m_l2req_queue[ii].valid = false;
      STAT_EVENT(PREF_UL2REQ_QUEUE_HIT_BY_DEMAND);
     
      return true;
    }
  }
  return false;
}


// send a new prefetch request to l1 request queue
bool hwp_common_c::pref_addto_l1req_queue(Addr line_index, uns8 prefetcher_id) 
{
  pref_mem_req_s new_req;
  if (!line_index) // addr = 0
    return true;

  if (*m_simBase->m_knobs->KNOB_PREF_DL0REQ_ADD_FILTER_ON) {
    for (int ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_DL0REQ_QUEUE_SIZE; ++ii ) {
      if (m_l1req_queue[ii].line_index == line_index) {
        STAT_EVENT(PREF_DL0REQ_QUEUE_MATCHED_REQ);
        return true; // Hit another request
      }
    }
  }
  if (m_l1req_queue[(m_l1req_queue_req_pos + 1) % *m_simBase->m_knobs->KNOB_PREF_DL0REQ_QUEUE_SIZE].valid) {
    STAT_EVENT(PREF_DL0REQ_QUEUE_FULL);
    if (!*m_simBase->m_knobs->KNOB_PREF_DL0REQ_QUEUE_OVERWRITE_ON_FULL) {
      return false; // Q full
    }	
  }

  new_req.line_addr     = line_index << m_shift_bit;
  new_req.line_index    = line_index;
  new_req.valid         = true;
  new_req.prefetcher_id = prefetcher_id;
  m_l1req_queue_req_pos = (m_l1req_queue_req_pos + 1) % *m_simBase->m_knobs->KNOB_PREF_DL0REQ_QUEUE_SIZE;
  m_l1req_queue[m_l1req_queue_req_pos] = new_req;

  return true;
}


// send a new request to l2 request queue
bool hwp_common_c::pref_addto_l2req_queue( Addr line_index, uns8 prefetcher_id)
{
  return pref_addto_l2req_queue_set(line_index, prefetcher_id, false, false, 0, 0);
}


// send a new request to l2 request queue
bool hwp_common_c::pref_addto_l2req_queue(Addr line_index, uns8 prefetcher_id, Addr loadPC)
{
  return pref_addto_l2req_queue_set(line_index, prefetcher_id, false, false, loadPC, 0);
}


// send a new request to l2 request queue
bool hwp_common_c::pref_addto_l2req_queue(Addr line_index, uns8 prefetcher_id, Addr loadPC, 
    int tid)
{
  return pref_addto_l2req_queue_set(line_index, prefetcher_id, false, false, loadPC, tid);
}


// send a new request to l2 request queue
bool hwp_common_c::pref_addto_l2req_queue_set(Addr line_index, uns8 prefetcher_id, bool Begin, 
    bool End, Addr loadPC)
{
  return pref_addto_l2req_queue_set(line_index, prefetcher_id, Begin, End, loadPC, 0);
}


// send a new request to l2 request queue
bool hwp_common_c::pref_addto_l2req_queue_set(Addr line_index, uns8 prefetcher_id, bool Begin, 
    bool End, Addr loadPC, int tid)
{
  int ii;
  pref_mem_req_s new_req;
  Addr line_addr;
  if (!line_index) // addr = 0
    return true;
    
  line_addr = (line_index) << m_shift_bit;
    
  if (*m_simBase->m_knobs->KNOB_PREF_UPDATE_INTERVAL!=0  && m_num_l2_evicts > *m_simBase->m_knobs->KNOB_PREF_UPDATE_INTERVAL) {
    float pol;   
    
    pref_info_s *hwp_info = pref_table[prefetcher_id]->hwp_info;

    m_num_l2_evicts = 0;
    if (hwp_info->curr_sent) {
      hwp_info->useful = 
        static_cast<Counter>(0.5*hwp_info->useful + (0.5*hwp_info->curr_useful));
      hwp_info->curr_useful = 0;
      m_useful = hwp_info->useful;

      hwp_info->sent = 
        static_cast<Counter>((0.5*hwp_info->sent) + (0.5*hwp_info->curr_sent));
      hwp_info->curr_sent   = 0;

      m_pfpol = static_cast<Counter>((0.5*m_pfpol) + (0.5*m_curr_pfpol));
      m_curr_pfpol = 0;

      hwp_info->late = static_cast<Counter>(0.5*hwp_info->late + (0.5*hwp_info->curr_late));
      hwp_info->curr_late = 0;

      m_l2_misses = 
        static_cast<Counter>((0.5*m_l2_misses) + (0.5*m_curr_l2_misses));
      m_curr_l2_misses = 0;

      m_update_acc = true;
		    
      pol = pref_get_l2pollution();

      if (pol > 0.5) 
        STAT_EVENT(PREF_POL_1);
      else if (pol > 0.40) 
        STAT_EVENT(PREF_POL_2);
      else if (pol > 0.25) 
        STAT_EVENT(PREF_POL_3);
      else if (pol > 0.10) 
        STAT_EVENT(PREF_POL_4);
      else if (pol > 0.05) 
        STAT_EVENT(PREF_POL_5);
      else if (pol > 0.01) 
        STAT_EVENT(PREF_POL_6);
      else if (pol > 0.0075) 
        STAT_EVENT(PREF_POL_7);
      else if (pol > 0.005) 
        STAT_EVENT(PREF_POL_8);
      else if (pol > 0.001) 
        STAT_EVENT(PREF_POL_9);
      else 
        STAT_EVENT(PREF_POL_10);
    }
  }

  if (*m_simBase->m_knobs->KNOB_PREF_POLBV_ON) {
    // UPDATE prefpolbv reset entry
    *pref_polbv_access(line_index) = 0;
  }

  if (*m_simBase->m_knobs->KNOB_PREF_REGION_ON) {
    pref_update_regioninfo(line_addr, false, false, false, 0, prefetcher_id);
  }

  if (*m_simBase->m_knobs->KNOB_PREF_HYBRID_ON) {
    int idx;    
    Addr reg_id = line_addr >> (LOG2_PREF_REGION_SIZE + m_shift_bit);
		
    idx = pref_matchregion(reg_id, false);
    if (idx == -1) {
      return true;
    }

    pref_region_info_s* rinfo = &region_info[idx];

    if (!rinfo->trained) {
      pref_hybrid_makeselection(idx); 
    }

    // Make sure that the correct prefetcher is selected to send req.
    if ((rinfo->trained && rinfo->pref_id != prefetcher_id) || 
        (!rinfo->trained && prefetcher_id != m_default_prefetcher)) {
      return true; // *THINK* your prefetch was sent -- Needed to train correctly
    }
  }

  if (*m_simBase->m_knobs->KNOB_PREF_ACC_STUDY) { 
    pref_info_s * hwp_info = pref_table[prefetcher_id]->hwp_info;
    // In this mode we sample every so many cycles and check which
    // prefetches would be sent
		
    if (m_simBase->m_simulation_cycle - hwp_info->track_lastsample_cycle > 
        *m_simBase->m_knobs->KNOB_PREF_ACC_UPDATE_INTERVAL) {
      if (hwp_info->track_num == PREF_TRACKERS_NUM) {
        int acc_bucket;
        // Calculate accuracy bucket
        float acc = pref_get_accuracy(prefetcher_id);

        acc_bucket = (int)floor(acc*10.0);
        //		printf("acc:%f bucket:%d\t",acc, acc_bucket);
        if (acc == 1.0) {
          acc_bucket = 9;
        }
        // Update counters.
        for (ii = 0; ii < hwp_info->track_num; ++ii) {
          if (hwp_info->trackers_used[ii]) {
            hwp_info->trackhist[acc_bucket][ii]++;
          }
          hwp_info->trackers_used[ii] = false; // setup for the next round
        }
        hwp_info->track_num = 0;
      }

      if (hwp_info->track_num < PREF_TRACKERS_NUM) {
        if (hwp_info->track_num !=0 || Begin) {
          for (ii = 0; ii < hwp_info->track_num; ++ii) {
            if (line_index == hwp_info->trackers[ii]) {
              return true; // already being tracked.
            }
          }
				    
          hwp_info->trackers[hwp_info->track_num] = line_index;
          hwp_info->track_num++;
        }
      }
      if (hwp_info->track_num == PREF_TRACKERS_NUM || ( End && hwp_info->track_num != 0) ) {
        hwp_info->track_lastsample_cycle = m_simBase->m_simulation_cycle;
      }
    }
  }

  if (*m_simBase->m_knobs->KNOB_PREF_UL1REQ_ADD_FILTER_ON) {
    for (ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_UL1REQ_QUEUE_SIZE; ++ii) {
      if (m_l2req_queue[ii].line_index == line_index) {
        STAT_EVENT(PREF_UL2REQ_QUEUE_MATCHED_REQ);
        return true; // Hit another request
      }
    }
  }


  if (m_l2req_queue[(m_l2req_queue_req_pos + 1) % *m_simBase->m_knobs->KNOB_PREF_UL1REQ_QUEUE_SIZE].valid) {
    STAT_EVENT(PREF_UL2REQ_QUEUE_FULL);
    if (!*m_simBase->m_knobs->KNOB_PREF_UL1REQ_QUEUE_OVERWRITE_ON_FULL) {
      return false; // Q full
    }
  }

  // update new request fields
  new_req.line_addr     = line_addr;
  new_req.line_index    = line_index;
  new_req.valid         = true;
  new_req.prefetcher_id = prefetcher_id;
  new_req.loadPC        = loadPC;
  new_req.core_id       = core_id;
  new_req.thread_id     = tid;


  m_l2req_queue_req_pos = (m_l2req_queue_req_pos + 1) % *m_simBase->m_knobs->KNOB_PREF_UL1REQ_QUEUE_SIZE;
  m_l2req_queue[m_l2req_queue_req_pos] = new_req;

  return true;
}


// update prefetch queues
// called by main.cc
void hwp_common_c::pref_update_queues(void)
{
  if (!*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON)
    return;

  // first check the l1 req queue to see if they can be satisfied by the l1.
  // otherwise send them to the l2 by putting them in the l2req queue    

  // dcache access 
  //  - 1. check to make sure ports are available
  //  - 2. if missing in the cache,  insert into the l2 req queue.
    
  // l2 access
  //  - 1. create a new request and call mem_req or new_mem_req

  for (int ii = 0 ; ii < *m_simBase->m_knobs->KNOB_PREF_DL0SCHEDULE_NUM ; ++ii) {
    int q_index = m_l1req_queue_send_pos; 
    uns bank;
    dcache_data_s* dc_hit;
    Addr dummy_line_addr;
    bool inc_send_pos = true;

    if (m_l1req_queue[q_index].valid ) {
      // port access
      bank = m_simBase->m_memory->bank_id(m_l1req_queue[q_index].core_id, 
          m_l1req_queue[q_index].line_addr);
		    
      if (!m_simBase->m_memory->get_read_port(m_l1req_queue[q_index].core_id, bank)) {
        // Port is not available.
        continue;
      }
      DEBUG_MEM("prefetch L1[%d] port bank:%d acquired\n", m_l1req_queue[q_index].core_id, bank);

      // Now, access the cache
      // FIXME (jaekyu, 4-26-2011) use right application id
      dc_hit = (dcache_data_s*)m_simBase->m_memory->access_cache(m_l1req_queue[q_index].core_id, \
          m_l1req_queue[q_index].line_addr, &dummy_line_addr, false, 0);
		    
      if (dc_hit) {
        // nothing for now
      } 
      else {
        // put req. into the l2req_queue	    
        if (!pref_addto_l2req_queue(m_l1req_queue[q_index].line_index, 
              m_l1req_queue[q_index].prefetcher_id)) {
          m_overall_l1sent++;
          inc_send_pos = false;
        }
      }
    }
    // Done with the l1
    if (inc_send_pos) {
      m_l1req_queue_send_pos = (m_l1req_queue_send_pos+1) % *m_simBase->m_knobs->KNOB_PREF_DL0REQ_QUEUE_SIZE;
    }
  }
		

  // Now work with the l2
  for (int ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_UL1SCHEDULE_NUM; ++ii) {
    int q_index = m_l2req_queue_send_pos; 
    bool inc_send_pos = true;
    if (m_l2req_queue[q_index].valid) {
      bool send_req = true;
      // now access the l2
      if (send_req) {
        pref_req_info_s info;
        int pref_core_id     = m_l2req_queue[q_index].core_id;
        info.m_prefetcher_id = m_l2req_queue[q_index].prefetcher_id;
        info.m_loadPC        = m_l2req_queue[q_index].loadPC;
        info.m_core_id       = m_l2req_queue[q_index].core_id;

        // bool (*done)(struct mem_req_s *) = NULL; // currently not used
        bool result;

        // FIXME
        // 64 -> L2_LINE_SIZE

        // there is some weird case that address is negative 64-bit number
        // this logic prevents wrong prefetch
        if (static_cast<int64_t>(m_l2req_queue[q_index].line_addr) < 0) {
          result = true;
        }
        else {
          result  = m_simBase->m_memory->new_mem_req(MRT_DPRF, m_l2req_queue[q_index].line_addr, 64, 1, \
              NULL, NULL, 0, &info, pref_core_id, m_l2req_queue[q_index].thread_id, \
              knob_ptx_sim); 
        }

        if (result) {
          DEBUG_MEM("core_id:%d thread_id:%d addr:%s type:%s [prefetch generated]\n",
             pref_core_id, m_l2req_queue[q_index].thread_id, \
             hexstr64s(m_l2req_queue[q_index].line_addr), mem_req_c::mem_req_type_name[MRT_DPRF]);

          DEBUG("Sent req %llx to l2 Qpos:%d\n", 
              m_l2req_queue[q_index].line_index, m_l2req_queue_send_pos);
          STAT_EVENT(PREF_UL2REQ_QUEUE_SENTREQ);
          m_l2req_queue[q_index].valid = false;
        }
        else { 
          STAT_EVENT(PREF_UL2REQ_SEND_QUEUE_STALL); 
          inc_send_pos = false;
          break; // buffer is full. wait!!
        }
      } 
      else { 
        // Do not send the request
        m_l2req_queue[q_index].valid = false;
      }
    }

    if (inc_send_pos) { 
      m_l2req_queue_send_pos = (m_l2req_queue_send_pos+1) % *m_simBase->m_knobs->KNOB_PREF_UL1REQ_QUEUE_SIZE;
    }
  }    
}


// Event handler: when a prefetch misses in the LLC and sent to the dram
void hwp_common_c::pref_l2sent(uns8 prefetcher_id)
{
  if (!*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON)
    return;
  if (prefetcher_id==0)
    return;

  if (!*m_simBase->m_knobs->KNOB_PREF_USEREGION_TOCALC_ACC) {
    // prefetch missed in the l2 and went out on the bus
    m_overall_l2sent++;
    pref_table[prefetcher_id]->hwp_info->curr_sent++;

    if (pref_table[prefetcher_id]->hwp_info->curr_sent == 
        *m_simBase->m_knobs->KNOB_PREF_DHAL_SENTTHRESH && *m_simBase->m_knobs->KNOB_PREF_DHAL) {
      if (pref_table[prefetcher_id]->hwp_info->curr_useful > *m_simBase->m_knobs->KNOB_PREF_DHAL_USETHRESH_MAX) {
        // INC
        if (pref_table[prefetcher_id]->hwp_info->dyn_degree<*m_simBase->m_knobs->KNOB_PREF_DHAL_MAXDEG)
          pref_table[prefetcher_id]->hwp_info->dyn_degree++;
      } 
      else if ( pref_table[prefetcher_id]->hwp_info->curr_useful < 
          *m_simBase->m_knobs->KNOB_PREF_DHAL_USETHRESH_MIN2) {
        if (pref_table[prefetcher_id]->hwp_info->curr_useful < *m_simBase->m_knobs->KNOB_PREF_DHAL_USETHRESH_MIN1) {
          // FAST DEC
          if (pref_table[prefetcher_id]->hwp_info->dyn_degree>8)
            pref_table[prefetcher_id]->hwp_info->dyn_degree = 
              pref_table[prefetcher_id]->hwp_info->dyn_degree/2;
          else 
            pref_table[prefetcher_id]->hwp_info->dyn_degree = 4;
        } 
        else {
          // DEC
          if (pref_table[prefetcher_id]->hwp_info->dyn_degree>4)
            pref_table[prefetcher_id]->hwp_info->dyn_degree--;
        } 
      }
      // reset the counts.
      pref_table[prefetcher_id]->hwp_info->curr_sent = 0;
      pref_table[prefetcher_id]->hwp_info->curr_useful = 0;
    }
  }
}


// Unused prefetch line eviction
void hwp_common_c::pref_evictline_notused(Addr addr)
{
  if (!*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON)
    return;

  m_num_uselesspref_evict++;
  m_num_l2_evicts--;  // do not count this as an evict.

  STAT_EVENT(PREF_UNUSED_EVICT);
}


// Increment L2 prefetch eviction count
void hwp_common_c::pref_l2evict(Addr addr)
{
  if (!*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON)
    return;

  m_num_l2_evicts++;
}


// Increment L2 prefetch eviction count
void hwp_common_c::pref_l2evictOnPF(Addr addr)
{
  if (!*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON)
    return;

  if (*m_simBase->m_knobs->KNOB_PREF_POLBV_ON) {
    Addr line_index = (addr>>m_shift_bit);
    *pref_polbv_access(line_index) = 1;
  }
    
  if (*m_simBase->m_knobs->KNOB_PREF_REGION_ON)
    pref_update_regioninfo(addr, false, false, true, m_simBase->m_simulation_cycle, 0);
}


// Get the prefetch accuracy 
float hwp_common_c::pref_get_replaccuracy(uns8 prefetcher_id)
{
  float acc;
  // For replacement wait a long time before switching.
  if (*m_simBase->m_knobs->KNOB_PREF_UPDATE_INTERVAL != 0) {
    acc = (pref_table[prefetcher_id]->hwp_info->sent > 1000) ? 
      ((float)pref_table[prefetcher_id]->hwp_info->useful / 
       (float)pref_table[prefetcher_id]->hwp_info->sent) : 1.0;  
  } 
  else {
    acc = (pref_table[prefetcher_id]->hwp_info->curr_sent > 1000) ? 
      ((float)pref_table[prefetcher_id]->hwp_info->curr_useful / 
       (float)pref_table[prefetcher_id]->hwp_info->curr_sent) : 1.0;  
  }
  return acc;
}


// This function says whether you want to increase/decrease the degree (feedback-driven).
// Use only with UPDATE.
HWP_DynAggr hwp_common_c::pref_get_degfb(uns8 prefetcher_id)
{
  HWP_DynAggr ret = AGGR_STAY;
  if (m_update_acc) {
    m_update_acc = false;
    float acc    = pref_get_accuracy(prefetcher_id);
    float timely = pref_get_timeliness(prefetcher_id);
    float pol    = pref_get_l2pollution();

    STAT_EVENT(PREF_UPDATE_COUNT);
    if (acc > 0.9) 
      STAT_EVENT(PREF_ACC_1);
    else if (acc > 0.8) 
      STAT_EVENT(PREF_ACC_2);
    else if (acc > 0.7) 
      STAT_EVENT(PREF_ACC_3);
    else if (acc > 0.6) 
      STAT_EVENT(PREF_ACC_4);
    else if (acc > 0.5) 
      STAT_EVENT(PREF_ACC_5);
    else if (acc > 0.4) 
      STAT_EVENT(PREF_ACC_6);
    else if (acc > 0.3) 
      STAT_EVENT(PREF_ACC_7);
    else if (acc > 0.2) 
      STAT_EVENT(PREF_ACC_8);
    else if (acc > 0.1) 
      STAT_EVENT(PREF_ACC_9);
    else 
      STAT_EVENT(PREF_ACC_10);

    if (timely > 0.9) 
      STAT_EVENT(PREF_TIMELY_1);
    else if (timely > 0.8) 
      STAT_EVENT(PREF_TIMELY_2);
    else if (timely > 0.7) 
      STAT_EVENT(PREF_TIMELY_3);
    else if (timely > 0.6) 
      STAT_EVENT(PREF_TIMELY_4);
    else if (timely > 0.5) 
      STAT_EVENT(PREF_TIMELY_5);
    else if (timely > 0.4) 
      STAT_EVENT(PREF_TIMELY_6);
    else if (timely > 0.3) 
      STAT_EVENT(PREF_TIMELY_7);
    else if (timely > 0.2) 
      STAT_EVENT(PREF_TIMELY_8);
    else if (timely > 0.1) 
      STAT_EVENT(PREF_TIMELY_9);
    else 
      STAT_EVENT(PREF_TIMELY_10);
    
		
    if (*m_simBase->m_knobs->KNOB_PREF_DEGFB_USEONLYLATE) {
      if (timely > *m_simBase->m_knobs->KNOB_PREF_TIMELY_THRESH) { // NOT TIMELY 
        ret = AGGR_INC;
        STAT_EVENT(PREF_ACC1_HT_LP);
        if (pref_table[prefetcher_id]->hwp_info->dyn_degree<4)
          pref_table[prefetcher_id]->hwp_info->dyn_degree++;
			
      } 
      else if (timely < *m_simBase->m_knobs->KNOB_PREF_TIMELY_THRESH_2) { // TOO TIMELY... go down
        ret = AGGR_DEC;
        if (pref_table[prefetcher_id]->hwp_info->dyn_degree>0)
          pref_table[prefetcher_id]->hwp_info->dyn_degree--;
      }
    } 
    else if (*m_simBase->m_knobs->KNOB_PREF_DEGFB_USEONLYPOL) {
      if (pol > *m_simBase->m_knobs->KNOB_PREF_POL_THRESH_1) {
        ret = AGGR_DEC;
        if (pref_table[prefetcher_id]->hwp_info->dyn_degree>0)
          pref_table[prefetcher_id]->hwp_info->dyn_degree--;
      } 
      else if (pol < *m_simBase->m_knobs->KNOB_PREF_POL_THRESH_2) {
        ret = AGGR_INC;
        STAT_EVENT(PREF_ACC1_HT_LP);
        if (pref_table[prefetcher_id]->hwp_info->dyn_degree<4)
          pref_table[prefetcher_id]->hwp_info->dyn_degree++;
      }
    } 
    else if ( acc > *m_simBase->m_knobs->KNOB_PREF_ACC_THRESH_1) {
      if (*m_simBase->m_knobs->KNOB_PREF_DEGFB_USEONLYACC) {
        ret = AGGR_INC;
        STAT_EVENT(PREF_ACC1_HT_LP);
        if (pref_table[prefetcher_id]->hwp_info->dyn_degree<4)
          pref_table[prefetcher_id]->hwp_info->dyn_degree++;
      } 
      else if (timely < *m_simBase->m_knobs->KNOB_PREF_TIMELY_THRESH) {
        if (pol > *m_simBase->m_knobs->KNOB_PREF_POLPF_THRESH) { // TIMELY WITH HIGH POL
          STAT_EVENT(PREF_ACC1_HT_HP);
          ret = AGGR_DEC;
          if (pref_table[prefetcher_id]->hwp_info->dyn_degree>0)
            pref_table[prefetcher_id]->hwp_info->dyn_degree--;
        } 
        else { // TIMELY WITH LOW POL
          STAT_EVENT(PREF_ACC1_HT_LP);
          ret = AGGR_STAY;
        }
      } 
      else { 
        if (pol > *m_simBase->m_knobs->KNOB_PREF_POLPF_THRESH) { // NOT TIMELY WITH HIGH POL
          STAT_EVENT(PREF_ACC1_LT_HP);
          /*
            ret = AGGR_DEC;  // MAYBE STAY???
            if (pref_table[prefetcher_id]->hwp_info->dyn_degree>0)
            pref_table[prefetcher_id]->hwp_info->dyn_degree--;
          */ 
          //		    ret = AGGR_STAY;
          ret = AGGR_INC;
          if (pref_table[prefetcher_id]->hwp_info->dyn_degree<4)
            pref_table[prefetcher_id]->hwp_info->dyn_degree++;
        } 
        else { // NOT TIMELY WITH LOW POL
          STAT_EVENT(PREF_ACC1_LT_LP);
          ret = AGGR_INC;
          if (pref_table[prefetcher_id]->hwp_info->dyn_degree<4)
            pref_table[prefetcher_id]->hwp_info->dyn_degree++;
        }
      }
    } 
    else if (acc > *m_simBase->m_knobs->KNOB_PREF_ACC_THRESH_2) {
      if (*m_simBase->m_knobs->KNOB_PREF_DEGFB_USEONLYACC) {
        ret = AGGR_STAY;
        STAT_EVENT(PREF_ACC2_HT_LP);
      } 
      else if (timely < *m_simBase->m_knobs->KNOB_PREF_TIMELY_THRESH) {
        if (pol > *m_simBase->m_knobs->KNOB_PREF_POLPF_THRESH) { // TIMELY WITH HIGH POL
          STAT_EVENT(PREF_ACC2_HT_HP);
          ret = AGGR_DEC;
          if (pref_table[prefetcher_id]->hwp_info->dyn_degree>0)
            pref_table[prefetcher_id]->hwp_info->dyn_degree--;
        } else { // TIMELY WITH LOW POL
          STAT_EVENT(PREF_ACC2_HT_LP);
          ret = AGGR_STAY;
        }
      } 
      else { 
        if (pol > *m_simBase->m_knobs->KNOB_PREF_POLPF_THRESH) { // NOT TIMELY WITH HIGH POL
          STAT_EVENT(PREF_ACC2_LT_HP);
          ret = AGGR_DEC;
          if (pref_table[prefetcher_id]->hwp_info->dyn_degree>0)
            pref_table[prefetcher_id]->hwp_info->dyn_degree--;
          //		    ret = AGGR_STAY;
        } else { // NOT TIMELY WITH LOW POL
          STAT_EVENT(PREF_ACC2_LT_LP);
          ret = AGGR_INC;
          if (pref_table[prefetcher_id]->hwp_info->dyn_degree<4)
            pref_table[prefetcher_id]->hwp_info->dyn_degree++;
        }
      }
    } 
    else if (acc > *m_simBase->m_knobs->KNOB_PREF_ACC_THRESH_3) {
      if (*m_simBase->m_knobs->KNOB_PREF_DEGFB_USEONLYACC) {
        STAT_EVENT(PREF_ACC3_HT_LP);
        ret = AGGR_DEC;
        if (pref_table[prefetcher_id]->hwp_info->dyn_degree>0)
          pref_table[prefetcher_id]->hwp_info->dyn_degree--;		
      } 
      else if (timely < *m_simBase->m_knobs->KNOB_PREF_TIMELY_THRESH) {
        if (pol > *m_simBase->m_knobs->KNOB_PREF_POLPF_THRESH) { // TIMELY WITH HIGH POL
          STAT_EVENT(PREF_ACC3_HT_HP);
          ret = AGGR_DEC;
          if (pref_table[prefetcher_id]->hwp_info->dyn_degree>0)
            pref_table[prefetcher_id]->hwp_info->dyn_degree--;
        } 
        else { // TIMELY WITH LOW POL
          STAT_EVENT(PREF_ACC3_HT_LP);
          ret = AGGR_DEC;
          if (pref_table[prefetcher_id]->hwp_info->dyn_degree>0)
            pref_table[prefetcher_id]->hwp_info->dyn_degree--;
				    
          //ret = AGGR_STAY; // MAYBE DEC for B/W
        }
      } 
      else { 
        if (pol > *m_simBase->m_knobs->KNOB_PREF_POLPF_THRESH) { // NOT TIMELY WITH HIGH POL
          STAT_EVENT(PREF_ACC3_LT_HP);
          ret = AGGR_DEC;
          if (pref_table[prefetcher_id]->hwp_info->dyn_degree>0)
            pref_table[prefetcher_id]->hwp_info->dyn_degree--;
        } 
        else { // NOT TIMELY WITH LOW POL
          STAT_EVENT(PREF_ACC3_LT_LP);
          ret = AGGR_STAY;
        }
      }
    } 
    else {
      if (*m_simBase->m_knobs->KNOB_PREF_DEGFB_USEONLYACC) {
        STAT_EVENT(PREF_ACC4_HT_LP);
        ret = AGGR_DEC;
        if (pref_table[prefetcher_id]->hwp_info->dyn_degree>0)
          pref_table[prefetcher_id]->hwp_info->dyn_degree--;		
      } 
      else if (timely < *m_simBase->m_knobs->KNOB_PREF_TIMELY_THRESH) {
        if (pol > *m_simBase->m_knobs->KNOB_PREF_POLPF_THRESH) { // TIMELY WITH HIGH POL
          STAT_EVENT(PREF_ACC4_HT_HP);
          ret = AGGR_DEC;
          if (pref_table[prefetcher_id]->hwp_info->dyn_degree>0)
            pref_table[prefetcher_id]->hwp_info->dyn_degree--;
        } 
        else { // TIMELY WITH LOW POL
          STAT_EVENT(PREF_ACC4_HT_LP);
          //		    ret = AGGR_STAY; // MAYBE DEC FOR BW
          ret = AGGR_DEC;
          if (pref_table[prefetcher_id]->hwp_info->dyn_degree>0)
            pref_table[prefetcher_id]->hwp_info->dyn_degree--;
        }
      } 
      else { 
        if (pol > *m_simBase->m_knobs->KNOB_PREF_POLPF_THRESH) { // NOT TIMELY WITH HIGH POL
          STAT_EVENT(PREF_ACC4_LT_HP);
          ret = AGGR_DEC;
          if (pref_table[prefetcher_id]->hwp_info->dyn_degree>0)
            pref_table[prefetcher_id]->hwp_info->dyn_degree--;
        } 
        else { // NOT TIMELY WITH LOW POL
          STAT_EVENT(PREF_ACC4_LT_LP);
          ret = AGGR_STAY;
        }
      }
    }


    if (pref_table[prefetcher_id]->hwp_info->dyn_degree == 4 ) 
      STAT_EVENT(PREF_DISTANCE_5);
    else if (pref_table[prefetcher_id]->hwp_info->dyn_degree == 3 ) 
      STAT_EVENT(PREF_DISTANCE_4);
    else if (pref_table[prefetcher_id]->hwp_info->dyn_degree == 2 ) 
      STAT_EVENT(PREF_DISTANCE_3);
    else if (pref_table[prefetcher_id]->hwp_info->dyn_degree == 1 ) 
      STAT_EVENT(PREF_DISTANCE_2);
    else if (pref_table[prefetcher_id]->hwp_info->dyn_degree == 0 ) 
      STAT_EVENT(PREF_DISTANCE_1);
    
    m_phase++;

    if (*m_simBase->m_knobs->KNOB_PREF_DEGFB_STATPHASEFILE) {
      fprintf(PREF_DEGFB_FILE, "%d %d\n", 
          pref_table[prefetcher_id]->hwp_info->dyn_degree, m_phase);
    }
  }
    
  return ret;
}


// get prefetch accuracy
float hwp_common_c::pref_get_accuracy(uns8 prefetcher_id)
{
  float acc;
  if (*m_simBase->m_knobs->KNOB_PREF_UPDATE_INTERVAL!=0) {
    acc = (pref_table[prefetcher_id]->hwp_info->sent > 20) ? 
      ((float)pref_table[prefetcher_id]->hwp_info->useful / 
       (float)pref_table[prefetcher_id]->hwp_info->sent) : 1.0;  
  } 
  else {
    acc = (pref_table[prefetcher_id]->hwp_info->curr_sent > 100) ? 
      ((float)pref_table[prefetcher_id]->hwp_info->curr_useful / 
       (float)pref_table[prefetcher_id]->hwp_info->curr_sent) : 1.0;  
  }
  return acc;
}


// Get overall prefetch accuracy
float hwp_common_c::pref_get_overallaccuracy(HWP_Type type)
{
  float acc;
  switch (type) {
    case Mem_To_UL1:
      acc = (m_overall_l2sent > 20) ? 
        ((float)m_overall_l2useful / (float)m_overall_l2sent) : 1.0;
      break;
    case UL1_To_DL0:
      acc = (m_overall_l2sent > 20) ? 
        ((float)m_overall_l2useful / (float)m_overall_l2sent) : 1.0;
      break;	
  }
  return acc;
}


// get prefetch timeliness
float hwp_common_c::pref_get_timeliness(uns8 prefetcher_id)
{
  float timely = 0.0;
  if (*m_simBase->m_knobs->KNOB_PREF_UPDATE_INTERVAL != 0) {
    timely = (pref_table[prefetcher_id]->hwp_info->useful > 100) ? 
      ((float)pref_table[prefetcher_id]->hwp_info->late / 
       (float)pref_table[prefetcher_id]->hwp_info->useful) : 1.0;  
  } 
  else {
    timely = (pref_table[prefetcher_id]->hwp_info->curr_useful > 100) ? 
      ((float)pref_table[prefetcher_id]->hwp_info->curr_late / 
       (float)pref_table[prefetcher_id]->hwp_info->curr_useful) : 1.0;  
  }
  return timely;
}


// get l2 pollution rate
float hwp_common_c::pref_get_l2pollution(void)
{
  float pol;
  if (*m_simBase->m_knobs->KNOB_PREF_UPDATE_INTERVAL != 0) {
    pol = (((m_l2_misses) > 100) ? ((float)m_pfpol / (float)(m_l2_misses)) : 0.0);
  } 
  else {
    pol = (((m_curr_l2_misses) > 1000) ? 
        ((float)m_curr_pfpol / (float)(m_curr_l2_misses)) : 0.0);
  }
    
  return pol;
}


// get region based accuracy
float hwp_common_c::pref_get_regionbased_acc(void)
{
  float acc;
  acc = (((m_region_sent) > 1000) ? ((float)m_region_useful / (float)(m_region_sent)) : 1.0);
  return acc;
}


// update prefetch region information
void  hwp_common_c::pref_update_regioninfo(Addr line_addr, bool l2_hit, bool l2_miss, 
    bool evict_onPF, Counter cycle_evict, uns8 prefetcher_id)
{
  uns  region_linenum;
  Addr reg_id;
  int  idx;

  region_linenum = (line_addr >> m_shift_bit) & (N_BIT_MASK(LOG2_PREF_REGION_SIZE));
  reg_id = line_addr >> (LOG2_PREF_REGION_SIZE + m_shift_bit);
  idx = pref_matchregion(reg_id, evict_onPF);

  if (idx == -1)
    return;

  pref_region_info_s* rinfo = &region_info[idx];
  pref_region_line_status_s* status = &rinfo->status[region_linenum];

  rinfo->last_access = m_simBase->m_simulation_cycle;

  if (l2_hit || l2_miss) {
    if (status->evict_onPF) { // POLLUTION EXISTS
      STAT_EVENT(PREF_PFPOL);
      //	    ASSERT(l2_miss);
      status->evict_onPF = false;
      m_curr_pfpol++;
    }
  }    

  status->l2_hit   = status->l2_hit || l2_hit;
  status->l2_miss  = status->l2_miss || l2_miss;
  status->l2_evict = status->l2_evict || (cycle_evict!=0);

  if (cycle_evict) { 
    status->cycle_evict = cycle_evict;
  }
  status->evict_onPF = status->evict_onPF || evict_onPF;

  ASSERT (prefetcher_id < 16);
    
  if (prefetcher_id != 0) {
    if (status->l2_hit || status->l2_miss) {
      status->l2_hit    = false;
      status->l2_miss   = false;
      status->pref_sent = 0;
    }
    status->l2_evict   = false;
    status->evict_onPF = false;
  }
}


// find matching region
int  hwp_common_c::pref_matchregion(Addr reg_id, bool evict_onPF)
{
  int ii;
  int idx = -1;
  for (ii = 0; ii< *m_simBase->m_knobs->KNOB_PREF_NUMTRACKING_REGIONS; ++ii) {
    // found matching entry
    if (region_info[ii].valid && region_info[ii].region_id == reg_id) {
      return ii;
    }

    // find entry to replace
    if ((idx == -1) || (!region_info[ii].valid) || 
        (region_info[idx].valid && 
         region_info[idx].last_access > region_info[ii].last_access)) {
      idx = ii;
    }
  }

  pref_region_info_s* rinfo = &region_info[idx];

  // NO Match
  // Reset entry
  if (evict_onPF || !rinfo->valid) {  // Create only on prefetcher evictions...
    if (rinfo->valid)
      STAT_EVENT(PREF_REGION_EVICT);
    rinfo->valid = true;
    rinfo->region_id = reg_id;
    rinfo->trained = false;
    memset(rinfo->status, 0, *m_simBase->m_knobs->KNOB_PREF_REGION_SIZE * sizeof(pref_region_line_status_s));

    return idx;
  }
  return -1;
}


// Try to pick a pref. for the current scenario
void  hwp_common_c::pref_hybrid_makeselection(int reg_idx)
{
  float best_acc = 0.0;
  float best_cov = 0.0;
  int   best_pref_id = -1;

  bool  update_ctrs  = false;

  // NOTE : This code assumes that at the most there are 16 prefetchers specified
  float pref_acc[16];
  float pref_cov[16];
    
  uns   pref_sent[16];
  uns   pref_useful[16];

  ASSERT(pref_table.size() < 16);
  uns   mem_accesses;
  if ((m_simBase->m_simulation_cycle-m_last_update_time) > *m_simBase->m_knobs->KNOB_PREF_HYBRID_DEFAULT_TIMEPERIOD) {
    // TIME To update the default.
    if ((m_simBase->m_simulation_cycle-m_last_update_time) > 
        *m_simBase->m_knobs->KNOB_PREF_HYBRID_UPDATE_MULTIPLE**m_simBase->m_knobs->KNOB_PREF_HYBRID_DEFAULT_TIMEPERIOD) {
      update_ctrs = true;
    }

    m_last_update_time = m_simBase->m_simulation_cycle;
    // Default is picked based on best accuracy 
    for (unsigned int ii = 0; ii < pref_table.size() ; ++ii) {	
      pref_info_s *hwp_info = pref_table[ii]->hwp_info;
      if (hwp_info->enabled) {
        float acc;
        Counter useful;
        Counter sent;
				
        useful = hwp_info->useful - hwp_info->hybrid_lastuseful;
        sent   = hwp_info->sent   - hwp_info->hybrid_lastsent;
				
        if (update_ctrs) {
          hwp_info->hybrid_lastuseful = hwp_info->useful;
          hwp_info->hybrid_lastsent   = hwp_info->sent;
        }

        acc = (sent > 20) ? ((float) useful/ (float) sent) : 0.0;  
        if (acc > best_acc) {
          best_acc = acc;
          best_pref_id = ii;
        }
      }
    }
    if (best_pref_id == -1) {
      best_pref_id = *m_simBase->m_knobs->KNOB_PREF_HYBRID_DEFAULT;
    }
    m_default_prefetcher = best_pref_id;
  }
	    
  best_acc     = 0.0;
  best_cov     = 0.0;
  best_pref_id = -1;
  mem_accesses = 0;

  for (unsigned int jj = 0; jj < pref_table.size(); ++jj) {
    pref_sent[jj] = 0;
    pref_useful[jj] = 0;
  }


  for (int ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_REGION_SIZE; ++ii) {
    bool useful = false;
    if (0) { // to prevent compilation error, jaekyu (11-3-2009)
      printf("%d", useful);
    }

    if ((region_info[reg_idx].status[ii].l2_hit || 
          region_info[reg_idx].status[ii].l2_miss)) {
      useful = true;
      mem_accesses++;
    }


    for (unsigned int jj = 0; jj < pref_table.size(); ++jj) {
#if NEED_TO_DEBUG
      if (g_pref_table[jj].hwp_info->enabled && 
          TESTBIT(region_info[reg_idx].status[ii].pref_sent, jj)) {
        pref_sent[jj]++;
        if (m_useful) 
          pref_useful[jj]++;
      }
#endif
    }
  }


  for (unsigned int jj = 0 ; jj < pref_table.size(); ++jj) {
    pref_acc[jj] = 0.0;
    pref_cov[jj] = 0.0;

    if (pref_sent[jj] > *m_simBase->m_knobs->KNOB_PREF_HYBRID_MIN_SENT) {
      pref_acc[jj] = ((float)pref_useful[jj] )/ ((float)pref_sent[jj]);
    }
    if (mem_accesses > *m_simBase->m_knobs->KNOB_PREF_HYBRID_MIN_MEMUSED) {
      pref_cov[jj] = ((float)pref_useful[jj] )/ ((float)mem_accesses);
    }
    // FOR NOW just knob these two
    if (*m_simBase->m_knobs->KNOB_PREF_HYBRID_SORT_ON_ACC) {
      if (pref_acc[jj] > best_acc) {
        best_pref_id = jj;
        best_acc = pref_acc[jj];
      }
    }
    if (*m_simBase->m_knobs->KNOB_PREF_HYBRID_SORT_ON_COV) {
      if (pref_cov[jj] > best_cov) {
        best_pref_id = jj;
        best_cov = pref_cov[jj];
      }
    }
  }


  if (best_pref_id!=-1) {
    region_info[reg_idx].trained = true;
    region_info[reg_idx].pref_id = best_pref_id;
    STAT_EVENT(PREF_HYBRID_SEL_0+best_pref_id);
  }
}


// update accuracy (used) 
void hwp_common_c::pref_acc_useupdate(Addr line_addr)
{
  Addr line_index = (line_addr>>m_shift_bit);
  for (unsigned int jj = 0 ; jj < pref_table.size(); ++jj) {
    pref_info_s *hwp_info = pref_table[jj]->hwp_info;
    if (hwp_info->enabled) {
      for (int ii = 0; ii < hwp_info->track_num; ++ii) {
        if (line_index == hwp_info->trackers[ii])
          hwp_info->trackers_used[ii] = true;
      }
    }
  }
}


// Access prefetch pollution bit vector
char* hwp_common_c::pref_polbv_access(Addr lineIndex)
{
  uns index = ((lineIndex >> *m_simBase->m_knobs->KNOB_LOG2_PREF_POLBV_SIZE) ^ lineIndex) & 
    *m_simBase->m_knobs->KNOB_LOG2_PREF_POLBV_SIZE;
  
  return &m_polbv_info[index];
}
    

// Train hardware prefetchers
void hwp_common_c::train(int level, int tid, Addr line_addr, Addr load_PC, uop_c* uop, \
    bool hit)
{
  if (!*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON)
    return;

  if (level == MEM_L2) {
    if (hit) {
      pref_l1_hit(tid, line_addr, load_PC, uop);
    }
    else {
      pref_l1_miss(tid, line_addr, load_PC, uop);
    }
  }
  else if (level == MEM_L3) {
    if (hit) {
      pref_l2_hit(tid, line_addr, load_PC, uop);
    }
    else {
      pref_l2_miss(tid, line_addr, uop);
    }
  }
}
