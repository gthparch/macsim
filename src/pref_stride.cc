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
 * File         : pref_stride.cc
 * Author       : HPArch
 * Date         : 1/23/2005
 * CVS          : $Id: pref_stride.cc,v 1.2 2008/09/12 03:42:01 kacear Exp $:
 * Description  : Stride Prefetcher - Based on RPT Prefetcher ( ICS'04 )
 *********************************************************************************************/


/* 
   stride prefetcher : Stride prefetcher based on Abraham's ICS'04 paper
   - "Effective Stream-Based and Execution-Based Data Prefetching"

   Divides memory in regions and then does multi-stride prefetching

*/


#include "global_defs.h"
#include "global_types.h"
#include "debug_macros.h"

#include "utils.h"
#include "assert.h"
#include "uop.h"
#include "core.h"
#include "memory.h"

#include "statistics.h"
#include "pref_stride.h"
#include "pref_common.h"

#include "all_knobs.h"

///////////////////////////////////////////////////////////////////////////////////////////////


#define DEBUG_MEM(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_MEM_TRACE, ## args)
#define DEBUG(args...)		_DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_PREF_STRIDE, ## args)


///////////////////////////////////////////////////////////////////////////////////////////////


// Default Constructor
pref_stride_c::pref_stride_c(macsim_c* simBase) : pref_base_c(simBase)
{
}


// constructor
pref_stride_c::pref_stride_c(hwp_common_c *hcc, Unit_Type type, macsim_c* simBase)
  : pref_base_c(simBase)
{
  name = "stride";
  hwp_type = Mem_To_UL1;
  hwp_common = hcc;
  switch (type) {
    case UNIT_SMALL:
      knob_enable = *m_simBase->m_knobs->KNOB_PREF_STRIDE_ON;
      break;
    case UNIT_MEDIUM:
      knob_enable = *m_simBase->m_knobs->KNOB_PREF_STRIDE_ON_MEDIUM_CORE;
      break;
    case UNIT_LARGE:
      knob_enable = *m_simBase->m_knobs->KNOB_PREF_STRIDE_ON_LARGE_CORE;
      break;
  }	


  // configuration
  l1_miss = true;
  l1_hit  = true;
  l2_miss = true;

}


// destructor
pref_stride_c::~pref_stride_c()
{
  if (!knob_enable)
    return ;

  delete region_table;
  delete index_table;
}


// initialization
void pref_stride_c::init_func(int core_id)
{
  if (!knob_enable) 
    return;

  core_id = core_id;

  hwp_info->enabled   = true;    
  region_table = new stride_region_table_entry_s[*m_simBase->m_knobs->KNOB_PREF_STRIDE_TABLE_N];
  index_table  = new stride_index_table_entry_s[*m_simBase->m_knobs->KNOB_PREF_STRIDE_TABLE_N];
}


// L1 hit training function
void pref_stride_c::l1_hit_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  l2_miss_func(tid, lineAddr, loadPC, uop);
}


// L1 miss training function
void pref_stride_c::l1_miss_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  l2_miss_func(tid, lineAddr, loadPC, uop);
}


// L2 hit training function
void pref_stride_c::l2_hit_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  train(tid, lineAddr, loadPC, true);
}


// L2 miss training function
void pref_stride_c::l2_miss_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  train(tid, lineAddr, loadPC, false);
}


// train stride tables
void pref_stride_c::train(int tid, Addr lineAddr, Addr loadPC, bool l2_hit)
{
  DEBUG_MEM("core_id:%d addr:%s trained\n", core_id, hexstr64s(lineAddr));

  int ii;
  int region_idx = -1;

  Addr lineIndex = lineAddr >> LOG2_DCACHE_LINE_SIZE;
  Addr index_tag = STRIDE_REGION(lineAddr);
  stride_index_table_entry_s * entry = NULL;

  int stride;
    
  // search an entry with index_tag
  for (ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_STRIDE_TABLE_N; ++ii) {
    if ((!*m_simBase->m_knobs->KNOB_PREF_THREAD_INDEX || tid == region_table[ii].tid) &&
        index_tag == region_table[ii].tag && 
        region_table[ii].valid) {
      // got a hit in the region table
      region_idx = ii;
      break;
    }
  }

  // entry not found, create a new entry
  if (region_idx == -1) {
    if (l2_hit) { // DONT CREATE ON HIT
      return;
    }

    // Not present in region table. 
    // Make new region. First look if any entry is unused
    for (ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_STRIDE_TABLE_N; ++ii) {
      if (!region_table[ii].valid) {
        region_idx = ii;
        break;
      }
      if (region_idx == -1 || 
          (region_table[region_idx].last_access < region_table[ii].last_access)) {
        region_idx = ii;
      }
    }
    create_newentry(region_idx, lineAddr, index_tag);

    return; 
  }

  
  entry  = &index_table[region_idx];
  stride = lineIndex - index_table[region_idx].last_index;
  index_table[region_idx].last_index = lineIndex;
  region_table[region_idx].last_access = CYCLE;

  // not trained yet
  if (!entry->trained) {
    if (!entry->train_count_mode) { 
      // init state
      if (entry->stride[entry->curr_state] == 0) { 
        entry->stride[entry->curr_state] = stride;
        entry->s_cnt[entry->curr_state] = 1;
      } 
      // stride match
      else if (entry->stride[entry->curr_state] == stride ) {
        entry->stride[entry->curr_state] = stride;
        entry->s_cnt[entry->curr_state]++;
      } 
      // stride does not match
      else {
        // single-stride
        if (*m_simBase->m_knobs->KNOB_PREF_STRIDE_SINGLE_STRIDE_MODE) {
          entry->stride[entry->curr_state] = stride;
          entry->s_cnt[entry->curr_state] = 1; // CORRECT --- 	    
        } 
        // in mult-stride mode
        // new stride -> Maybe it is a transition:
        else {
          entry->strans[entry->curr_state] = stride;
          if (entry->num_states == 1) {
            entry->num_states = 2;		    
          }
          entry->curr_state = (1 - entry->curr_state); // change 0 to 1 or 1 to 0
          if (entry->curr_state == 0) {
            entry->train_count_mode = true; // move into a checking mode
            entry->count = 0;
            entry->recnt = 0;
          }		
        }
      }	    
    } 
    else {
      // in train_count_mode
      if (stride == entry->stride[entry->curr_state] && 
          entry->count < entry->s_cnt[entry->curr_state]) {
        entry->recnt++;
        entry->count++;
      } 
      else if (stride == entry->strans[entry->curr_state] && 
          entry->count == entry->s_cnt[entry->curr_state]) {
        entry->recnt++;
        entry->count = 0;
        entry->curr_state = (1 - entry->curr_state);
      } 
      else {
        // does not match... lets reset.
        create_newentry(region_idx, lineAddr, index_tag);
      }
    }

    if ((entry->s_cnt[entry->curr_state] >= *KNOB(KNOB_PREF_STRIDE_SINGLE_THRESH))) {
      // single stride stream
      entry->trained = true;
      entry->num_states = 1;
      entry->curr_state = 0;
      entry->stride[0] = entry->stride[entry->curr_state];
      entry->pref_last_index = entry->last_index + 
                               (entry->stride[0] * *KNOB(KNOB_PREF_STRIDE_STARTDISTANCE));
    }

    if (entry->recnt >= *KNOB(KNOB_PREF_STRIDE_MULTI_THRESH)) {
      Addr pref_index;
      entry->trained = true;
      entry->pref_count = entry->count;
      entry->pref_curr_state = entry->curr_state;
      entry->pref_last_index = entry->last_index;
      for (ii = 0; (ii < *m_simBase->m_knobs->KNOB_PREF_STRIDE_STARTDISTANCE); ++ii) { 
        if (entry->pref_count==entry->s_cnt[entry->pref_curr_state]){
          pref_index = entry->pref_last_index + entry->strans[entry->pref_curr_state];
          entry->pref_count = 0;		    
          entry->pref_curr_state = (1 - entry->pref_curr_state);
        }
        else {
          pref_index = entry->pref_last_index + entry->stride[entry->pref_curr_state];
          entry->pref_count++;
        }
        entry->pref_last_index = pref_index;
      }
    }
  } // !trained 
  // entry has been already trained
  else {
    Addr pref_index;
    if (entry->pref_sent)
      entry->pref_sent--;
    // single stride case
    if (entry->num_states == 1 && stride == entry->stride[0]) {
      for (ii = 0; 
          (ii < *KNOB(KNOB_PREF_STRIDE_DEGREE) && 
           entry->pref_sent < *KNOB(KNOB_PREF_STRIDE_DISTANCE)); 
          ++ii, entry->pref_sent++) {
        pref_index = entry->pref_last_index + entry->stride[0];
        if (!hwp_common->pref_addto_l2req_queue(pref_index, hwp_info->id))
          break; // queue is full
        entry->pref_last_index = pref_index;
      }
    } 
    // multi-stride
    else if ((stride == entry->stride[entry->curr_state] && 
          entry->count <entry->s_cnt[entry->curr_state]) ||
        (stride == entry->strans[entry->curr_state] && 
         entry->count==entry->s_cnt[entry->curr_state]) ) {
      // first update verification info.
      if (entry->count==entry->s_cnt[entry->curr_state]){
        entry->count = 0;
        entry->curr_state = (1 - entry->curr_state);
      }
      else {
        entry->count++;
      }
      // now send out prefetches
      for (ii = 0; 
          (ii < *KNOB(KNOB_PREF_STRIDE_DEGREE) && 
           entry->pref_sent < *KNOB(KNOB_PREF_STRIDE_DISTANCE)); 
          ++ii, entry->pref_sent++) { 
        if (entry->pref_count == entry->s_cnt[entry->pref_curr_state]){
          pref_index = entry->pref_last_index + entry->strans[entry->pref_curr_state];
          if (!hwp_common->pref_addto_l2req_queue(pref_index, hwp_info->id))
            break; // q is full
          entry->pref_count = 0;		    
          entry->pref_curr_state = (1 - entry->pref_curr_state);
        }
        else {
          pref_index = entry->pref_last_index + entry->stride[entry->pref_curr_state];
          if (!hwp_common->pref_addto_l2req_queue(pref_index, hwp_info->id))
            break; // q is full
          entry->pref_count++;
          DEBUG_MEM("core_id:%d pref_addto_l2req_queue index:%s\n",
              core_id, hexstr64s(pref_index));

        }
        entry->pref_last_index = pref_index;
      }
    } 
    // not trained
    else {
      entry->trained          = false;
      entry->train_count_mode = false;
      entry->num_states       = 1;
      entry->curr_state       = 0;
      entry->stride[0]        = 0;
      entry->stride[1]        = 0;
      entry->s_cnt[0]         = 0;
      entry->s_cnt[1]         = 0;
      entry->strans[0]        = 0;
      entry->strans[1]        = 0;
      entry->count            = 0;
      entry->recnt            = 0;
      entry->pref_sent        = 0;
    }
  }
}


// create a new stride
void pref_stride_c::create_newentry (int idx, Addr line_addr, Addr region_tag)
{
  region_table[idx].tag             = region_tag;
  region_table[idx].valid           = true;
  region_table[idx].last_access     = CYCLE;    
  index_table[idx].trained          = false;
  index_table[idx].num_states       = 1;
  index_table[idx].curr_state       = 0; // 0 or 1
  index_table[idx].last_index       = line_addr >> LOG2_DCACHE_LINE_SIZE;
  index_table[idx].stride[0]        = 0;
  index_table[idx].s_cnt[0]         = 0;
  index_table[idx].stride[1]        = 0;
  index_table[idx].s_cnt[1]         = 0;
  index_table[idx].strans[0]        = 0;
  index_table[idx].strans[1]        = 0;
  index_table[idx].recnt            = 0;
  index_table[idx].count            = 0;
  index_table[idx].train_count_mode = false;
  index_table[idx].pref_sent        = 0;
}

