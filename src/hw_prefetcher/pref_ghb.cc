/***************************************************************************************
 * File         : pref_ghb.c
 * Author       : Santhosh Srinath
 * Date         : 11/16/2004
 * CVS          : $Id: pref_ghb.cc,v 1.2 2008/09/12 03:42:01 kacear Exp $:
 * Description  : 
 ***************************************************************************************/


#include "pref_ghb.h"

#include "../global_defs.h"
#include "../global_types.h"
#include "../debug_macros.h"

#include "../utils.h"
#include "../assert_macros.h"
#include "../uop.h"

#include "../cache.h"
#include "../statistics.h"
#include "../memory.h"
#include "../pref_common.h"
#include "../core.h"

#include "../all_knobs.h"

/* 
   ghb_prefetcher : Global History Buffer prefetcher
   Based on the C/DC prefetcher described in the AC/DC paper
   
   Divides memory into "regions" - static partition of the address space 
   The index table is indexed by the region id and gives a pointer to the 
   last access in that region in the GHB 
*/

/**************************************************************************************/
/* Macros */
#define DEBUG(args...)		_DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_PREF_GHB, ## args)

#define CIRC_DEC(val, num)		(((val) == 0) ? (num) - 1 : (val) - 1)


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
//pref_ghb_c::pref_ghb_c()
//{
//}


pref_ghb_c::~pref_ghb_c()
{
}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
pref_ghb_c::pref_ghb_c(hwp_common_c *hcc, Unit_Type type, macsim_c* simBase)
: pref_base_c(simBase)
{
  name = "ghb";
  hwp_type = Mem_To_UL1;
  hwp_common = hcc;

  switch (type) {
    case UNIT_SMALL:
      knob_enable = *m_simBase->m_knobs->KNOB_PREF_GHB_ON;
      break;
    case UNIT_MEDIUM:
      knob_enable = *m_simBase->m_knobs->KNOB_PREF_GHB_ON_MEDIUM_CORE;
      break;
    case UNIT_LARGE:
      knob_enable = *m_simBase->m_knobs->KNOB_PREF_GHB_ON_LARGE_CORE;
      break;
  }	

  shift_bit = LOG2_DCACHE_LINE_SIZE; 
  l1_miss = true;
  l1_hit = true;
  l2_miss = true;
  l2_pref_hit = true;

}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void pref_ghb_c::init_func(int core_id)
{
  if (!knob_enable) 
    return;

  core_id = core_id;

  this->hwp_info->enabled = true;
  this->index_table = (GHB_Index_Table_Entry *) malloc(sizeof(GHB_Index_Table_Entry)**m_simBase->m_knobs->KNOB_PREF_GHB_INDEX_N);
  this->ghb_buffer  = (GHB_Entry *) malloc(sizeof(GHB_Entry)**m_simBase->m_knobs->KNOB_PREF_GHB_BUFFER_N);
    
  this->ghb_head    = -1;
  this->ghb_tail    = -1;
  this->deltab_size = *m_simBase->m_knobs->KNOB_PREF_GHB_MAX_DEGREE+2;

  this->delta_buffer = (int *) calloc(this->deltab_size, sizeof(int));
  this->pref_degree = *m_simBase->m_knobs->KNOB_PREF_GHB_DEGREE;

  for (int ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_GHB_INDEX_N; ++ii) 
  {
    this->index_table[ii].valid = false;
    this->index_table[ii].last_access = 0;
  }

  for (int ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_GHB_BUFFER_N; ++ii) 
  {
    this->ghb_buffer[ii].ghb_ptr = -1;
    this->ghb_buffer[ii].ghb_reverse_ptr = -1;
    this->ghb_buffer[ii].idx_reverse_ptr = -1;
  }


  this->pref_degree_vals[0] = 2;
  this->pref_degree_vals[1] = 4;
  this->pref_degree_vals[2] = 8;
  this->pref_degree_vals[3] = 12;
  this->pref_degree_vals[4] = 16;
}


void pref_ghb_c::l1_hit_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  l2_miss_func(tid, lineAddr, loadPC, uop);
}


void pref_ghb_c::l1_miss_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  l2_miss_func(tid, lineAddr, loadPC, uop);
}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void pref_ghb_c::l2_pref_hit_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  pref_ghb_l2_train(tid, lineAddr, loadPC, true, NULL);
}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void pref_ghb_c::l2_miss_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  pref_ghb_l2_train(tid, lineAddr, loadPC, false, uop);    
}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void pref_ghb_c::pref_ghb_l2_train(int tid, Addr lineAddr, Addr loadPC, bool is_l2_hit, uop_c *uop)
{
//  if (g_simulation_cycle == 314)
//    cout << "found\n";

  // 1. adds address to ghb
  // 2. sends upto "degree" prefetches to the prefQ
  int ii;
  int czone_idx = -1;
  int old_ptr   = -1;

  int ghb_idx   = -1;
  int delta1    = 0;
  int delta2    = 0;

  int num_pref_sent = 0;
  int deltab_head = -1;
  int curr_deltab_size = 0;

  Addr lineIndex = lineAddr >> shift_bit;
  Addr currLineIndex = lineIndex;
  Addr index_tag = CZONE_TAG(lineAddr);
    
  DEBUG("GHB : Miss Addr %d czone: %d\n", (int) lineAddr, (int) index_tag);
  for (ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_GHB_INDEX_N; ++ii) 
  {
    if ((!*m_simBase->m_knobs->KNOB_PREF_THREAD_INDEX || tid == this->index_table[ii].tid) &&
        index_tag == this->index_table[ii].czone_tag && 
        this->index_table[ii].valid) 
    {
      // got a hit in the index table
      czone_idx = ii;
      old_ptr   = this->index_table[ii].ghb_ptr;
      break;
    }
  }

  if (czone_idx == -1) 
  {
    if (is_l2_hit)  // ONLY TRAIN on l2_hit
      return;
		
    // Not present in index table. 
    // Make new czone
    // First look if any entry is unused
    for (ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_GHB_INDEX_N; ii++) {
      if (!this->index_table[ii].valid) {
        czone_idx = ii;
        break;
      }
      if (czone_idx == -1 || (this->index_table[czone_idx].last_access < this->index_table[ii].last_access)) {
        czone_idx = ii;
      }
    }
    DEBUG("Created new entry in index: %d idx:%d \n", (int) index_tag ,  czone_idx);
  }

  if (old_ptr != -1 && this->ghb_buffer[old_ptr].miss_index == lineIndex) {
    return;
  }

  if (*m_simBase->m_knobs->KNOB_PREF_THROTTLE_ON) 
    pref_ghb_throttle();
  
  if (*m_simBase->m_knobs->KNOB_PREF_THROTTLEFB_ON) 
    pref_ghb_throttle_fb();

  pref_ghb_create_newentry(czone_idx, lineAddr, index_tag, old_ptr, tid);

  for (ii = 0; ii < this->deltab_size; ii++)
    this->delta_buffer[ii] = 0;

  // Now ghb_tail points to the new entry. Work backwards to find a 2 delta match...
  ghb_idx = this->ghb_buffer[this->ghb_tail].ghb_ptr;
  DEBUG("l2hit:%d lineidx:%llx loadPC:%llx\n", is_l2_hit, lineIndex, loadPC);
  while (ghb_idx != -1 && num_pref_sent<this->pref_degree) {
    int delta = currLineIndex - this->ghb_buffer[ghb_idx].miss_index;
    if (delta > 100 || delta < -100) 
      break;

    // insert into delta buffer
    deltab_head = (deltab_head + 1) % this->deltab_size;
    this->delta_buffer[deltab_head] = delta;
    curr_deltab_size++;
    if (delta1==0) {
      delta1 = delta;
    } else if (delta2==0) {
      delta2 = delta;
    } else {
      DEBUG("delta1:%d, delta2:%d\n", delta1, delta2);
      // Catch strides quickly
      if (delta1 == delta2) {
        for (; num_pref_sent<this->pref_degree; num_pref_sent++) {		    
          lineIndex += delta1;
          hwp_common->pref_addto_l2req_queue_set(lineIndex, this->hwp_info->id, (num_pref_sent==0), (num_pref_sent==this->pref_degree), loadPC);
        }
      } else {
        if (delta1 == this->delta_buffer[(deltab_head-1)%this->deltab_size]  && delta2 == this->delta_buffer[deltab_head]) {
          // found a match
          // lets go for a walk 
          int deltab_idx = (deltab_head - 2) % this->deltab_size;
          int deltab_start_idx = deltab_idx;
          for (; num_pref_sent<this->pref_degree; num_pref_sent++) {
            lineIndex += this->delta_buffer[deltab_idx];
            hwp_common->pref_addto_l2req_queue_set(lineIndex, this->hwp_info->id, (num_pref_sent==0), (num_pref_sent==this->pref_degree), loadPC);
            DEBUG("Sent %llx\n", lineIndex);
            deltab_idx = CIRC_DEC(deltab_idx, this->deltab_size);
            if (deltab_idx > curr_deltab_size) {
              deltab_idx = deltab_start_idx;
            }
          }
          break;
        }
      }
    }
    currLineIndex =  this->ghb_buffer[ghb_idx].miss_index;
    ghb_idx = this->ghb_buffer[ghb_idx].ghb_ptr;
  }
  if (num_pref_sent) {
    DEBUG("Num sent %d\n", num_pref_sent);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void pref_ghb_c::pref_ghb_create_newentry (int idx, Addr line_addr, Addr czone_tag, int old_ptr, int tid)
{
  int rev_ptr;
  int rev_idx_ptr;

  this->index_table[idx].valid     = true;
  this->index_table[idx].czone_tag = czone_tag;
  this->index_table[idx].last_access = m_simBase->m_simulation_cycle;
  this->index_table[idx].tid = tid;

  // Now make entry in ghb
  this->ghb_tail = (this->ghb_tail + 1) % *m_simBase->m_knobs->KNOB_PREF_GHB_BUFFER_N;
  if (this->ghb_tail == old_ptr) { // takes care of some bad corner cases
    old_ptr = -1;
  }
  if (this->ghb_head == -1) {
    this->ghb_head = 0;
  } else if (this->ghb_tail == this->ghb_head) {
    // wrap-around
    this->ghb_head = (this->ghb_head + 1) % *m_simBase->m_knobs->KNOB_PREF_GHB_BUFFER_N;
  }

  rev_ptr = this->ghb_buffer[this->ghb_tail].ghb_reverse_ptr;
  rev_idx_ptr = this->ghb_buffer[this->ghb_tail].idx_reverse_ptr;
  if (rev_ptr!=-1) {
    this->ghb_buffer[rev_ptr].ghb_ptr = -1;
  }
    
  if (rev_idx_ptr != -1 && this->index_table[rev_idx_ptr].ghb_ptr == this->ghb_tail && rev_idx_ptr != idx) {
    this->index_table[rev_idx_ptr].ghb_ptr = -1;
    this->index_table[rev_idx_ptr].valid   = false;
  }
	
  this->ghb_buffer[this->ghb_tail].miss_index = line_addr >> shift_bit;
  this->ghb_buffer[this->ghb_tail].ghb_ptr    = old_ptr ;
  this->ghb_buffer[this->ghb_tail].ghb_reverse_ptr = -1;
  this->ghb_buffer[this->ghb_tail].idx_reverse_ptr = idx;
  if (old_ptr != -1)
    this->ghb_buffer[old_ptr].ghb_reverse_ptr           = this->ghb_tail;

  this->index_table[idx].ghb_ptr                 = this->ghb_tail;
}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void pref_ghb_c::pref_ghb_throttle(void)
{
  int dyn_shift = 0;

  float acc = hwp_common->pref_get_accuracy(this->hwp_info->id);
  //float acc = pref_get_overallaccuracy(Mem_To_UL1);
  float regacc = hwp_common->pref_get_regionbased_acc();
  float accratio = acc/regacc;
  //    float cov = pref_get_coverage(this->hwp_info->id);

  if (acc != 1.0) {
    if (acc > *m_simBase->m_knobs->KNOB_PREF_ACC_THRESH_1) {
      dyn_shift += 2;
    } else if (acc > *m_simBase->m_knobs->KNOB_PREF_ACC_THRESH_2) {
      dyn_shift += 1;
    } else if (acc > *m_simBase->m_knobs->KNOB_PREF_ACC_THRESH_3) {
      dyn_shift = 0;
    } else if (acc > *m_simBase->m_knobs->KNOB_PREF_ACC_THRESH_4) {
      dyn_shift = dyn_shift - 1;
    } else {
      dyn_shift = dyn_shift - 2;
    }
  }
  if (*m_simBase->m_knobs->KNOB_PREF_ACCRATIOTHROTTLE) {
    if (accratio < *m_simBase->m_knobs->KNOB_PREF_ACCRATIO_1 ) {
      dyn_shift = dyn_shift - 1;
    }
  }
  /*	    
  // Adjust for high coverage
  if (cov > 0.70) {
  if (dyn_shift < 0) {
  dyn_shift = 0;
  }
  }
  */
  // COLLECT STATS
  if (acc > 0.9) {
    STAT_EVENT(PREF_ACC_1);
  } else if (acc > 0.8) {
    STAT_EVENT(PREF_ACC_2);
  } else if (acc > 0.7) {
    STAT_EVENT(PREF_ACC_3);
  } else if (acc > 0.6) {
    STAT_EVENT(PREF_ACC_4);
  } else if (acc > 0.5) {
    STAT_EVENT(PREF_ACC_5);
  } else if (acc > 0.4) {
    STAT_EVENT(PREF_ACC_6);
  } else if (acc > 0.3) {
    STAT_EVENT(PREF_ACC_7);
  } else if (acc > 0.2) {
    STAT_EVENT(PREF_ACC_8);
  } else if (acc > 0.1) {
    STAT_EVENT(PREF_ACC_9);
  } else {
    STAT_EVENT(PREF_ACC_10);
  }

  if (acc==1.0){
    this->pref_degree = 64;
  } else {
    if (dyn_shift >= 2 ) {
      this->pref_degree = 64;
      STAT_EVENT(PREF_DISTANCE_5);
    } else if (dyn_shift == 1 ) {
      this->pref_degree = 32;
      STAT_EVENT(PREF_DISTANCE_4);
    } else if (dyn_shift == 0 ) {
      this->pref_degree = 16;
      STAT_EVENT(PREF_DISTANCE_3);
    } else if (dyn_shift == -1 ) {
      this->pref_degree = 8;
      STAT_EVENT(PREF_DISTANCE_2);
    } else if (dyn_shift <= -2 ) {
      this->pref_degree = 2;
      STAT_EVENT(PREF_DISTANCE_1);
    } 
  }
}


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
void pref_ghb_c::pref_ghb_throttle_fb(void)
{
  hwp_common->pref_get_degfb(this->hwp_info->id);
  ASSERT(this->hwp_info->dyn_degree<=4 );
  this->pref_degree = this->pref_degree_vals[this->hwp_info->dyn_degree];
}