/***************************************************************************************
 * File         : pref_stride.c
 * Author       : Santhosh Srinath
 * Date         : 1/23/2005
 * CVS          : $Id: pref_stridepc.cc,v 1.2 2008/09/12 03:42:02 kacear Exp $:
 * Description  : Stride Prefetcher - Based on load's PC address
 ***************************************************************************************/


#include "pref_stridepc.h"

#include "../global_defs.h"
#include "../global_types.h"
#include "../debug_macros.h"

#include "../utils.h"
#include "../assert_macros.h"
#include "../uop.h"
#include "../statistics.h"
#include "../pref_common.h"
#include "../core.h"
#include "../memory.h"

#include "../all_knobs.h"

/* 
   stride prefetcher : Stride prefetcher based on the original stride work
   - Essentially use the load's PC to index into a table of prefetch entries

*/

/**************************************************************************************/
/* Macros */
#define DEBUG(args...)		_DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_PREF_STRIDEPC, ## args)

/**************************************************************************************/
/* Global variables */


// Default Constructor
//pref_stridepc_c::pref_stridepc_c() {}

pref_stridepc_c::pref_stridepc_c(hwp_common_c *hcc, Unit_Type type, macsim_c* simBase)
: pref_base_c(simBase)
{
  name = "stridepc";
  hwp_type = Mem_To_UL1;
  hwp_common = hcc;
  switch (type) {
  case UNIT_SMALL:
    knob_enable = *m_simBase->m_knobs->KNOB_PREF_STRIDEPC_ON;
    break;
  case UNIT_MEDIUM:
    knob_enable = *m_simBase->m_knobs->KNOB_PREF_STRIDEPC_ON_MEDIUM_CORE;
    break;
  case UNIT_LARGE:
    knob_enable = *m_simBase->m_knobs->KNOB_PREF_STRIDEPC_ON_LARGE_CORE;
    break;
  }	

  l1_miss = true;
  l1_hit = true;
  l2_miss = true;
 
}


void pref_stridepc_c::init_func(int cid)
{
  if (!knob_enable) 
    return;

  core_id = cid;
  hwp_info->enabled   = true;
  // stridepc_hwp             = (Pref_StridePC*)malloc(sizeof(Pref_StridePC));
  this->stride_table = (StridePC_Table_Entry *) calloc(*m_simBase->m_knobs->KNOB_PREF_STRIDEPC_TABLE_N, sizeof(StridePC_Table_Entry));
}

void pref_stridepc_c::l1_hit_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  l2_miss_func(tid, lineAddr, loadPC, uop);
}

void pref_stridepc_c::l1_miss_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  l2_miss_func(tid, lineAddr, loadPC, uop);
}

void pref_stridepc_c::l2_hit_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  pref_stridepc_l2_train(tid, lineAddr, loadPC, uop, true);
}


void pref_stridepc_c::l2_miss_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  pref_stridepc_l2_train(tid, lineAddr, loadPC, uop, false);
}

void pref_stridepc_c::pref_stridepc_l2_train(int tid, Addr lineAddr, Addr loadPC, uop_c *uop, bool l2_hit)
{
  int ii;
  int idx = -1;

  Addr lineIndex = lineAddr >> LOG2_DCACHE_LINE_SIZE;
  StridePC_Table_Entry * entry = NULL;

  int stride;
    
  if (loadPC==0) {
    return; // no point hashing on a null address
  }

  for (ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_STRIDEPC_TABLE_N; ii++) {
    if ((!*m_simBase->m_knobs->KNOB_PREF_THREAD_INDEX || this->stride_table[ii].tid == tid) && 
        this->stride_table[ii].load_addr == loadPC && 
        this->stride_table[ii].valid) {
      idx = ii;
      break;
    }
  }


  if (idx == -1) {
    if (l2_hit) { // ONLY TRAIN on hit
      return;
    }
    for (ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_STRIDEPC_TABLE_N; ii++) {
      if (!this->stride_table[ii].valid) {
        idx = ii;
        break;
      }
      if (idx == -1 || (this->stride_table[idx].last_access < this->stride_table[ii].last_access)) {
        idx = ii;
      }
    }
    this->stride_table[idx].trained    = false;
    this->stride_table[idx].valid      = true;
    this->stride_table[idx].stride     = 0; 
    this->stride_table[idx].train_num  = 0;
    this->stride_table[idx].pref_sent  = 0;
    this->stride_table[idx].last_addr  = ( *m_simBase->m_knobs->KNOB_PREF_STRIDEPC_USELOADADDR ? lineAddr : lineIndex);
    //this->stride_table[idx].last_addr  = ( *m_simBase->m_knobs->KNOB_PREF_STRIDEPC_USELOADADDR ? lineAddr : lineIndex);
    this->stride_table[idx].last_addr = uop->m_vaddr;
    this->stride_table[idx].load_addr  = loadPC; 
    this->stride_table[idx].last_access= m_simBase->m_simulation_cycle;
    this->stride_table[idx].tid = tid;
    return; 
  }
    
  entry  = &this->stride_table[idx];
  entry->last_access = m_simBase->m_simulation_cycle;
  stride = ( *m_simBase->m_knobs->KNOB_PREF_STRIDEPC_USELOADADDR ? ( lineAddr - entry->last_addr ) : ( lineIndex - entry->last_addr ) );
    
  //    printf("loadaddr:%llx trained:%d entrystride:%d currstride:%d lineaddr:%llx pfaddr:%llx\n", entry->load_addr, entry->trained, entry->stride, stride, lineIndex, entry->pref_last_index);
  
  if (!entry->trained) {
    // Now let's train
    if (stride == 0)
      return;
    if (entry->stride!=stride) {
      entry->stride = stride;
      entry->train_num = 1;
    } else {
      entry->train_num++;
    }
    if ( entry->train_num == *m_simBase->m_knobs->KNOB_PREF_STRIDEPC_TRAINNUM) {
      entry->trained = true;
      entry->start_index = (*m_simBase->m_knobs->KNOB_PREF_STRIDEPC_USELOADADDR ? lineAddr : lineIndex);
      entry->pref_last_index = entry->start_index + (*m_simBase->m_knobs->KNOB_PREF_STRIDEPC_STARTDIS*entry->stride);
      entry->pref_sent = 0;
    }
  } else {	
    Addr pref_index;
    Addr curr_idx = (*m_simBase->m_knobs->KNOB_PREF_STRIDEPC_USELOADADDR ? lineAddr : lineIndex);
		
    if (entry->pref_sent)
      entry->pref_sent--;
		
    if ((stride % entry->stride==0) &&
        (((stride > 0) && (curr_idx >= entry->start_index) && (curr_idx <= entry->pref_last_index)) ||
         ((stride < 0) && (curr_idx <= entry->start_index) && (curr_idx >= entry->pref_last_index)) )) { 
      // all good. continue sending out prefetches
      for (ii = 0; (ii < *m_simBase->m_knobs->KNOB_PREF_STRIDEPC_DEGREE && entry->pref_sent < *m_simBase->m_knobs->KNOB_PREF_STRIDEPC_DISTANCE); ii++, entry->pref_sent++) {
        pref_index = entry->pref_last_index + entry->stride;
        if (!hwp_common->pref_addto_l2req_queue( (*m_simBase->m_knobs->KNOB_PREF_STRIDEPC_USELOADADDR ? (pref_index>>LOG2_DCACHE_LINE_SIZE) : pref_index), this->hwp_info->id )) 
          break; // q is full
        entry->pref_last_index = pref_index;
      }
    } else {
      // stride has changed...
      // lets retrain
      entry->trained = false;
      entry->train_num = 1;
    }
  }
  entry->last_addr = ( *m_simBase->m_knobs->KNOB_PREF_STRIDEPC_USELOADADDR ? lineAddr : lineIndex);
}
