/***************************************************************************************
 * File         : pref_stream.c
 * Author       : Santhosh Srinath ( based on Hyesoon's code )
 * Date         : 1/20/2005
 * CVS          : $Id: pref_stream.cc,v 1.2 2008/09/12 03:42:02 kacear Exp $:
 * Description  : Stream Prefetcher
 ***************************************************************************************/


#include "pref_stream.h"

#include "../global_defs.h"
#include "../global_types.h"
#include "../debug_macros.h"

#include "../utils.h"
#include "../assert_macros.h"
#include "../uop.h"
#include "../memory.h"
#include "../core.h"
#include "../cache.h"
#include "../statistics.h"
#include "../memory.h"
#include "../pref_common.h"

#include "../all_knobs.h"
#include "../knob.h"

/**************************************************************************************/
/* Macros */
#define DEBUG(args...)		_DEBUG((*m_simBase->m_knobs->KNOB_DEBUG_STREAM || *m_simBase->m_knobs->KNOB_DEBUG_MEM_TRACE), ## args)
#define DEBUG_PREFACC(args...)      _DEBUG(DEBUG_PREFACC,    ## args)

/**************************************************************************************/
/* Global Variables */

#if 0 
extern Memory *mem;
extern Dcache_Stage *dc;
#endif

/***************************************************************************************/
/* Local Prototypes */

/**************************************************************************************/
/* stream prefetcher  */
/* prefetch is initiated by dcache miss but the request fills the l1 cache (second level) cache   */
/* each stream has the starting pointer and the ending pointer and those pointers tell whether the l1 miss is within the boundary (buffer) */
/* stream buffer fetches one by one (end point requests the fetch address  */
/* stream buffer just holds the stream boundary not the data itself and data is stored in the second level cache  */
/* At the beginning we will wait until we see 2 miss addresses */
/* using 2 miss addresses we decide the direction of the stream ( upward or downward) and in the begining fill the half of buffer ( so many request !!! ) */
/* Reference : IBM POWER 4 White paper */


// Default Constructor
//pref_stream_c::pref_stream_c() {}

pref_stream_c::pref_stream_c(hwp_common_c *hcc, Unit_Type type, macsim_c* simBase)
: pref_base_c(simBase)
{
  name = "stream";
  hwp_type = Mem_To_UL1;
  hwp_common = hcc;

  // configuration
  switch (type) {
    case UNIT_SMALL:
      knob_enable = *m_simBase->m_knobs->KNOB_PREF_STREAM_ON;
      break;
    case UNIT_MEDIUM:
      knob_enable = *m_simBase->m_knobs->KNOB_PREF_STREAM_ON_MEDIUM_CORE;
      break;
    case UNIT_LARGE:
      knob_enable = *m_simBase->m_knobs->KNOB_PREF_STREAM_ON_LARGE_CORE;
      break;
  }	


  done     = true;
  l1_hit  = true;
  l2_miss = true;
  l2_hit  = true;
  
}


void pref_stream_c::init_func(int cid)
{
  if (!knob_enable) {
    return;
  }

  core_id = cid;
  hwp_info->enabled                  = true;
  pref_stream                             = new Pref_Stream; 
  pref_stream->hwp_info                   = hwp_info;

  pref_stream->stream                     = new Stream_Buffer[*m_simBase->m_knobs->KNOB_STREAM_BUFFER_N];    
  pref_stream->train_filter               = new Addr[*m_simBase->m_knobs->KNOB_TRAIN_FILTER_SIZE];

  pref_stream->train_num                  = *m_simBase->m_knobs->KNOB_STREAM_TRAIN_NUM;
  pref_stream->distance                   = *m_simBase->m_knobs->KNOB_STREAM_LENGTH;
  pref_stream->pref_degree_vals[0] = 4;
  pref_stream->pref_degree_vals[1] = 8;
  pref_stream->pref_degree_vals[2] = 16;
  pref_stream->pref_degree_vals[3] = 32;
  pref_stream->pref_degree_vals[4] = 64;

  pref_stream->num_tosend          = *m_simBase->m_knobs->KNOB_STREAM_PREFETCH_N;
  pref_stream->num_tosend_vals[0] = 1;
  pref_stream->num_tosend_vals[1] = 1;
  pref_stream->num_tosend_vals[2] = 2;
  pref_stream->num_tosend_vals[3] = 4;
  pref_stream->num_tosend_vals[4] = 4;
}


void pref_stream_c::l1_hit_func(int tid, Addr line_addr, Addr load_PC, uop_c *uop)
{
  l2_miss_func(tid, line_addr, load_PC, uop);
} 


/* line_addr: the first address of the cache block */ 
void pref_stream_c::pref_stream_train(int tid, Addr line_addr, Addr load_PC, bool create)   
{ 
  // search the stream buffer 
  int hit_index = -1;
  int ii;
  int dis, maxdistance;
  Addr line_index = line_addr >> LOG2_DCACHE_LINE_SIZE; 
  /* training filter */

  DEBUG("[DL0MISS%s]:0x%7llx mi:0x%lld core_id:%d\n", "L1", line_addr, line_index, core_id);

  if (!pref_stream_train_stream_filter(line_index)) {

    if (*m_simBase->m_knobs->KNOB_PREF_THROTTLE_ON) {
      pref_stream_throttle();

      if (*m_simBase->m_knobs->KNOB_PREF_STREAM_ACCPERSTREAM) 
        pref_stream_throttle_streams(line_index);
    }

    if (*m_simBase->m_knobs->KNOB_PREF_THROTTLEFB_ON) {
      pref_stream_throttle_fb();
    }

    /* search for stream buffer */
    // so we create on dcache misses also? - confusing... - onur
    hit_index = pref_stream_train_create_stream_buffer(line_index, true, create, tid); 

    if (hit_index == -1) /* we do not have a trained buffer, nor did we create it */
      return;

    pref_stream_addto_train_stream_filter(line_index);

    if (pref_stream->stream[hit_index].trained) {

      pref_stream->stream[hit_index].lru = m_simBase->m_simulation_cycle;  // update lru
      STAT_EVENT(HIT_TRAIN_STREAM);

      /* hit the stream_buffer, request the prefetch */

      for (ii = 0 ; ii < pref_stream->num_tosend; ii++) {

        if ((pref_stream->stream[hit_index].sp == line_index) && (pref_stream->stream[hit_index].buffer_full)) { 
          // when is buffer_full set to false except for buffer creation? - onur
          // stream prefetch is requesting  enough far ahead 
          // stop prefetch and wait until miss address is within the buffer area 
          return;
        }

        if (!hwp_common->pref_addto_l2req_queue(pref_stream->stream[hit_index].ep + pref_stream->stream[hit_index].dir, pref_stream->hwp_info->id, load_PC)){
          //if (!hwp_common->pref_addto_l2req_queue(pref_stream->stream[hit_index].ep + pref_stream->stream[hit_index].dir, pref_stream->hwp_info->id)){
          return;
        }

        pref_stream->stream[hit_index].ep = pref_stream->stream[hit_index].ep + pref_stream->stream[hit_index].dir;
        dis  = pref_stream->stream[hit_index].ep - pref_stream->stream[hit_index].sp ;
        maxdistance = (*m_simBase->m_knobs->KNOB_PREF_STREAM_ACCPERSTREAM ? pref_stream->stream[hit_index].length : pref_stream->distance);
        if (((pref_stream->stream[hit_index].dir == 1) && (dis > maxdistance) ) || 
            ((pref_stream->stream[hit_index].dir == -1)  && (dis < -maxdistance) )) {
          pref_stream->stream[hit_index].buffer_full = true;
          pref_stream->stream[hit_index].sp = pref_stream->stream[hit_index].sp + pref_stream->stream[hit_index].dir;
        }

        if (*m_simBase->m_knobs->KNOB_REMOVE_REDUNDANT_STREAM) 
          pref_stream_remove_redundant_stream(hit_index); 

        DEBUG("[InQ:0x%s]ma:0x%7llx mi:0x%7llx d:%2d ri:0x%7llx, sp:0x%7llx ep:0x%7llx core_id:%d\n",
            "L1", 
            line_addr, line_index, pref_stream->stream[hit_index].dir, 
            pref_stream->stream[hit_index].ep + pref_stream->stream[hit_index].dir,
            pref_stream->stream[hit_index].sp, pref_stream->stream[hit_index].ep, core_id);
        }
      }
      else STAT_EVENT(MISS_TRAIN_STREAM);
    }
  }


void pref_stream_c::l2_miss_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  pref_stream_train(tid, lineAddr, loadPC, true);
}


void pref_stream_c::l2_hit_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  pref_stream_train(tid, lineAddr, loadPC, false);
}


int pref_stream_c::pref_stream_train_create_stream_buffer(Addr line_index, bool train, bool create, int tid) 
{
  int ii;
  int dir;
  int lru_index = -1;
  bool found_closeby = false;
  // First check for a trained buffer
  for (ii = 0; ii < *m_simBase->m_knobs->KNOB_STREAM_BUFFER_N; ii++) {
    if ((!*m_simBase->m_knobs->KNOB_PREF_THREAD_INDEX || pref_stream->stream[ii].tid == tid) && 
        pref_stream->stream[ii].valid && 
        pref_stream->stream[ii].trained ) {
      if(((pref_stream->stream[ii].sp <= line_index ) && ((pref_stream->stream[ii].ep + *m_simBase->m_knobs->KNOB_PREF_TRAIN_WINDOW_SLACK) >= (line_index )) && (pref_stream->stream[ii].dir == 1)) || 
          (((pref_stream->stream[ii].sp -*m_simBase->m_knobs->KNOB_PREF_TRAIN_WINDOW_SLACK) >= (line_index ))  && (pref_stream->stream[ii].ep <= line_index) && (pref_stream->stream[ii].dir == -1))) {
        // found a trained buffer 
        return ii;
      }
    }
  }

  if (train || create) {
    for (ii = 0; ii < *m_simBase->m_knobs->KNOB_STREAM_BUFFER_N; ii++) {
      if ((!*m_simBase->m_knobs->KNOB_PREF_THREAD_INDEX || pref_stream->stream[ii].tid == tid) && 
          pref_stream->stream[ii].valid 
          && (!pref_stream->stream[ii].trained)) { 
        if ((pref_stream->stream[ii].sp <= (line_index + *m_simBase->m_knobs->KNOB_STREAM_TRAIN_LENGTH)) && 
            (pref_stream->stream[ii].sp >= (line_index - *m_simBase->m_knobs->KNOB_STREAM_TRAIN_LENGTH))) {

          if (train) { // do these only if we are training
            // decide the train dir 
            if (pref_stream->stream[ii].sp > line_index) dir = -1;
            else dir = 1;
            pref_stream->stream[ii].train_hit++;
            if (pref_stream->stream[ii].train_hit > pref_stream->train_num) {
              pref_stream->stream[ii].trained = true;
              pref_stream->stream[ii].start_vline = pref_stream->stream[ii].sp;
              pref_stream->stream[ii].ep = (dir > 0 ) ? line_index + *m_simBase->m_knobs->KNOB_STREAM_START_DIS : line_index - *m_simBase->m_knobs->KNOB_STREAM_START_DIS  ;  // 04/17/03 BUG !!! ins_model.c 
              pref_stream->stream[ii].dir = dir;
              DEBUG("stream trained stream_index:%3d sp %7llx ep %7llx dir %2d miss_index %llx core_id:%d\n",
                  ii, pref_stream->stream[ii].sp, pref_stream->stream[ii].ep, pref_stream->stream[ii].dir, line_index, core_id);
            }
          }

          // create a new stream buffer 
          // create_stream_buffer(dir, line_index );
          return ii;
        }
      }
    }

    if (!create || found_closeby)
      return -1;
  }	  

  if (create) {
    // search for invalid buffer
    for (ii = 0; ii < *m_simBase->m_knobs->KNOB_STREAM_BUFFER_N; ii++) {
      if (!pref_stream->stream[ii].valid){
        lru_index = ii;
        break;
      }
    }

    // search for oldest buffer 

    if (lru_index == -1) {
      uns len;
      lru_index = 0;
      for ( ii = 0 ; ii < *m_simBase->m_knobs->KNOB_STREAM_BUFFER_N; ii++) {
        if (pref_stream->stream[ii].lru < pref_stream->stream[lru_index].lru ) {
          lru_index = ii;
        }
      }
      STAT_EVENT(REPLACE_OLD_STREAM); 
      if (pref_stream->stream[lru_index].dir==0 || !pref_stream->stream[lru_index].trained) {
        len = 0;
      } 
      else if (pref_stream->stream[lru_index].dir == 1) {
        len = pref_stream->stream[lru_index].ep - pref_stream->stream[lru_index].start_vline + 1;
      } 
      else {
        len = pref_stream->stream[lru_index].start_vline - pref_stream->stream[lru_index].ep + 1;
      }			      
      if (len!=0) {
        STAT_EVENT( STREAM_LENGTH_0 + MIN2(len / 10, 10));
      }
    }

    // create new train buffer

    pref_stream->stream[lru_index].lru         = m_simBase->m_simulation_cycle;
    pref_stream->stream[lru_index].valid       = true;
    pref_stream->stream[lru_index].sp          = line_index;
    pref_stream->stream[lru_index].ep          = line_index;
    pref_stream->stream[lru_index].train_hit   = 1;
    pref_stream->stream[lru_index].trained     = false;
    pref_stream->stream[lru_index].buffer_full = false;

    pref_stream->stream[lru_index].length      = *m_simBase->m_knobs->KNOB_STREAM_LENGTH;
    pref_stream->stream[lru_index].pref_issued = 0;
    pref_stream->stream[lru_index].pref_useful = 0;

    STAT_EVENT(STREAM_TRAIN_CREATE); 
    DEBUG("create new stream : stream_no :%3d, line_index %7llx sp = %7llx core_id:%d\n",
        lru_index, line_index, pref_stream->stream[lru_index].sp, core_id);
    return lru_index;
  }

  return -1;
}					      


void pref_stream_c::pref_stream_throttle(void)
{
  int dyn_shift = 0;
  float acc = hwp_common->pref_get_accuracy(pref_stream->hwp_info->id);
  float regacc = hwp_common->pref_get_regionbased_acc();
  float accratio = acc/regacc;

  if (acc != 1.0) {
    if (acc > *m_simBase->m_knobs->KNOB_PREF_ACC_THRESH_1) {
      dyn_shift += 2;
    } 
    else if (acc > *m_simBase->m_knobs->KNOB_PREF_ACC_THRESH_2) {
      dyn_shift += 1;
    } 
    else if (acc > *m_simBase->m_knobs->KNOB_PREF_ACC_THRESH_3) {
      dyn_shift = 0;
    } 
    else if (acc > *m_simBase->m_knobs->KNOB_PREF_ACC_THRESH_4) {
      dyn_shift = dyn_shift - 1;
    } 
    else {
      dyn_shift = dyn_shift - 2;
    }
  }
  if (*m_simBase->m_knobs->KNOB_PREF_ACCRATIOTHROTTLE) {
    if (accratio < *m_simBase->m_knobs->KNOB_PREF_ACCRATIO_1 ) {
      dyn_shift = dyn_shift - 1;
    }
  }
  if (acc==1.0) {
    pref_stream->distance  = 64;
    //pref_stream->train_num = PREF_STREAM_TRAIN_NUM_3;
    STAT_EVENT(PREF_DISTANCE_4);
  } 
  else {	
    if (dyn_shift >= 2 ) {
      pref_stream->distance = 128;
      //	pref_stream->train_num = PREF_STREAM_TRAIN_NUM_3;
      STAT_EVENT(PREF_DISTANCE_5);
    } 
    else if (dyn_shift == 1 ) {
      pref_stream->distance  = 64;
      //pref_stream->train_num = PREF_STREAM_TRAIN_NUM_3;
      STAT_EVENT(PREF_DISTANCE_4);
    } 
    else if (dyn_shift == 0 ) {
      pref_stream->distance  = 32;
      //pref_stream->train_num = PREF_STREAM_TRAIN_NUM_2;
      STAT_EVENT(PREF_DISTANCE_3);
    } 
    else if (dyn_shift == -1 ) {
      pref_stream->distance  = 16;
      //pref_stream->train_num = PREF_STREAM_TRAIN_NUM_1;
      STAT_EVENT(PREF_DISTANCE_2);
    } 
    else if (dyn_shift <= -2 ) {
      pref_stream->distance  = 5;
      //pref_stream->train_num = PREF_STREAM_TRAIN_NUM_0;
      STAT_EVENT(PREF_DISTANCE_1);
    } 
  }
}


////////////////////////////////////////////////////////////////////////
// Rest Used when throttling for each stream separately - NON FUNCTIONAL
bool pref_stream_c::pref_stream_train_stream_filter(Addr line_index) 
{
  int ii;
  for ( ii = 0 ; ii < *m_simBase->m_knobs->KNOB_TRAIN_FILTER_SIZE; ii++) {
    if (pref_stream->train_filter[ii] == line_index) {
      return true;
    }
  }
  return false;
}

inline void pref_stream_c::pref_stream_addto_train_stream_filter(Addr line_index)
{
  pref_stream->train_filter[(pref_stream->train_filter_no++)%*m_simBase->m_knobs->KNOB_TRAIN_FILTER_SIZE] = line_index;    
}


void pref_stream_c::pref_stream_remove_redundant_stream(int hit_index)
{
  int ii;

  for (ii = 0; ii < *m_simBase->m_knobs->KNOB_STREAM_BUFFER_N; ii++) {
    if ((ii == hit_index) || (!pref_stream->stream[ii].valid)) continue; 
    if (((pref_stream->stream[ii].ep < pref_stream->stream[hit_index].ep ) && 
          (pref_stream->stream[ii].ep > pref_stream->stream[hit_index].sp )  ) || 
        ((pref_stream->stream[ii].sp < pref_stream->stream[hit_index].ep ) && 
         (pref_stream->stream[ii].sp > pref_stream->stream[hit_index].sp ) )) {
      pref_stream->stream[ii].valid = false;
      STAT_EVENT(REMOVE_REDUNDANT_STREAM_STAT);
      DEBUG("stream[%d] sp:0x%llx ep:0x%llx is removed by stream[%d] sp:0x%llx ep:0x%llx core_id:%d\n",
          ii, pref_stream->stream[ii].sp, pref_stream->stream[ii].ep, hit_index, 
          pref_stream->stream[hit_index].sp, pref_stream->stream[hit_index].ep, core_id);
    }
  }
}


float pref_stream_c::pref_stream_acc_getacc(int index, float pref_acc)
{
  float acc = pref_stream->stream[index].pref_issued>40?((float) pref_stream->stream[index].pref_useful) / ((float)pref_stream->stream[index].pref_issued) : pref_acc;
  return acc;
}


void pref_stream_c::pref_stream_acc_l2_useful(Addr line_index)
{
  if (!knob_enable) {
    return;
  }

  for (int ii = 0; ii < *m_simBase->m_knobs->KNOB_STREAM_BUFFER_N; ii++) 
  {
    if (pref_stream->stream[ii].valid && pref_stream->stream[ii].trained ) 
    {
      if(((pref_stream->stream[ii].start_vline <= line_index ) && (pref_stream->stream[ii].ep >= line_index) && (pref_stream->stream[ii].dir == 1)) || 
          ((pref_stream->stream[ii].start_vline >= line_index ) && (pref_stream->stream[ii].ep <= line_index) && (pref_stream->stream[ii].dir == -1))) {
        // found a trained buffer
        pref_stream->stream[ii].pref_useful += 1;
      }
    }
  }    
  //    pref_stream->pref_useful += 1;
}


void pref_stream_c::pref_stream_acc_l2_issued(Addr line_index)
{   
  for (int ii = 0; ii < *m_simBase->m_knobs->KNOB_STREAM_BUFFER_N; ++ii) 
  {
    if (pref_stream->stream[ii].valid && pref_stream->stream[ii].trained ) 
    {
      if(((pref_stream->stream[ii].start_vline <= line_index ) && 
            (pref_stream->stream[ii].ep >= line_index) && 
            (pref_stream->stream[ii].dir == 1)) || 
          ((pref_stream->stream[ii].start_vline >= line_index ) && 
           (pref_stream->stream[ii].ep <= line_index) && 
           (pref_stream->stream[ii].dir == -1))) 
      {
        // found a trained buffer
        pref_stream->stream[ii].pref_issued += 1;
      }
    }
  }
  //    pref_stream->pref_issued += 1;
}


void pref_stream_c::pref_stream_throttle_streams(Addr line_index)
{
  for (int ii = 0; ii < *m_simBase->m_knobs->KNOB_STREAM_BUFFER_N; ++ii) 
  {
    if (pref_stream->stream[ii].valid && pref_stream->stream[ii].trained ) 
    {
      if(((pref_stream->stream[ii].ep - *m_simBase->m_knobs->KNOB_PREF_ACC_DISTANCE_10 <= line_index ) && 
            (pref_stream->stream[ii].ep >= line_index) && (pref_stream->stream[ii].dir == 1)) || 
          ((pref_stream->stream[ii].ep + *m_simBase->m_knobs->KNOB_PREF_ACC_DISTANCE_10 >= line_index ) && (
            pref_stream->stream[ii].ep <= line_index) && (pref_stream->stream[ii].dir == -1))) 
      {
        // found a trained buffer
        pref_stream_throttle_stream(ii);
        return;
      }
    }
  }
}

/* 
   throttle_stream_pf -> reset the stream length and train length for this stream buffer
   */
void pref_stream_c::pref_stream_throttle_stream(int index)
{
  if (0) { // to prevent compilation error, jaekyu (11-3-2009)
    printf("%d", index);
  }
  /*
     float stream_acc;
     float pref_acc;
     uns thresh_num = 0;
     uns   old_length = pref_stream->stream[index].length;

     pref_acc = (pref_stream->pref_issued > 100) ?((float)pref_stream->pref_useful)/((float)pref_stream->pref_issued):1;

     if (PREF_ACC_USE_CACHE) {
     stream_acc = pref_stream_acc_getacc(index, pref_acc);
     } else if (PREF_ACC_USE_REGION) {
     stream_acc = Pref_getaccuracy(pref_stream->stream[index].ep, index);
     } else { 
     stream_acc = Pref_getaccuracy_stream(pref_stream->stream[index].ep, index, pref_stream->stream[index].start_vline, pref_stream->stream[index].ep);
     }

  // First set train distance based on pref_acc 
  if (PREF_ACC_DYN_TRAIN_ON) {
  if (pref_acc >= PREF_TRAIN_THRESH_3) {
  pref_stream->train_num = PREF_ACC_TRAIN_NUM_3;
  }
  else if (pref_acc >= PREF_TRAIN_THRESH_2) {
  pref_stream->train_num = PREF_ACC_TRAIN_NUM_2;
  }
  else if (pref_acc >= PREF_TRAIN_THRESH_1) {
  pref_stream->train_num = PREF_ACC_TRAIN_NUM_1;
  }
  else {
  pref_stream->train_num = PREF_ACC_TRAIN_NUM_0;
  }
  }
  if (!PREF_ACC_DYN_DIST_ON) {
  return;
  }
  if (PREF_ACC_USE_OVERALLALSO && stream_acc < PREF_ACC_THRESH_7 ) {
  if (pref_acc >= PREF_TRAIN_THRESH_3) {
  stream_acc = stream_acc + PREF_ACC_TRAIN_ACCOFFSET_3;	  
  }
  else if (pref_acc >= PREF_TRAIN_THRESH_2) {
  stream_acc = stream_acc + PREF_ACC_TRAIN_ACCOFFSET_2;
  }
  else if (pref_acc >= PREF_TRAIN_THRESH_1) {
  stream_acc = stream_acc + PREF_ACC_TRAIN_ACCOFFSET_1;
  }
  else {
  stream_acc = stream_acc + PREF_ACC_TRAIN_ACCOFFSET_0;
  }
  }
  if (PREF_ACC_USE_ONLYGLOBAL) {
  stream_acc = pref_acc;
  }
  // Now set the stream length for this buffer based on stream_acc
  if (stream_acc >= PREF_ACC_THRESH_9) {
  if (PREF_ACC_INCDEC_LENGTH && pref_stream->stream[index].length < PREF_ACC_DISTANCE_10) {
  pref_stream->stream[index].length += 6;
  } else {
  pref_stream->stream[index].length = PREF_ACC_DISTANCE_10;
  }
  thresh_num = 9;
  } else if (stream_acc >= PREF_ACC_THRESH_8) {
  if (PREF_ACC_INCDEC_LENGTH && pref_stream->stream[index].length < PREF_ACC_DISTANCE_9) {
  pref_stream->stream[index].length += 5;
  } else {
  pref_stream->stream[index].length = PREF_ACC_DISTANCE_9;
  }
  thresh_num = 8;
  } else if (stream_acc >= PREF_ACC_THRESH_7) { 
  if (PREF_ACC_INCDEC_LENGTH && pref_stream->stream[index].length < PREF_ACC_DISTANCE_8) {
  pref_stream->stream[index].length += 4;
  } else {
  pref_stream->stream[index].length = PREF_ACC_DISTANCE_8;
}
thresh_num = 7;
} else if (stream_acc >= PREF_ACC_THRESH_6) { 
  if (PREF_ACC_INCDEC_LENGTH && pref_stream->stream[index].length < PREF_ACC_DISTANCE_7) {
    pref_stream->stream[index].length += 3;
  } else {
    pref_stream->stream[index].length = PREF_ACC_DISTANCE_7;
  }
  thresh_num = 6;
} else if (stream_acc >= PREF_ACC_THRESH_5) { 
  if (PREF_ACC_INCDEC_LENGTH && pref_stream->stream[index].length < PREF_ACC_DISTANCE_6) {
    pref_stream->stream[index].length += 2;
  } else {
    pref_stream->stream[index].length = PREF_ACC_DISTANCE_6;
  }
  thresh_num = 5;
} else if (stream_acc >= PREF_ACC_THRESH_4) { 
  if (PREF_ACC_INCDEC_LENGTH && pref_stream->stream[index].length < PREF_ACC_DISTANCE_5) {
    pref_stream->stream[index].length += 1;
  } else {
    pref_stream->stream[index].length = PREF_ACC_DISTANCE_5;
  }
  thresh_num = 4;
} else if (stream_acc >= PREF_ACC_THRESH_3) { 
  if (PREF_ACC_INCDEC_LENGTH && pref_stream->stream[index].length > PREF_ACC_DISTANCE_4) {
    pref_stream->stream[index].length -= 1;
  } else {
    pref_stream->stream[index].length = PREF_ACC_DISTANCE_4;
  }
  thresh_num = 3;
} else if (stream_acc >= PREF_ACC_THRESH_2) { 
  if (PREF_ACC_INCDEC_LENGTH && pref_stream->stream[index].length > PREF_ACC_DISTANCE_3) {
    pref_stream->stream[index].length -= 2;
  } else {
    pref_stream->stream[index].length = PREF_ACC_DISTANCE_3;
  }
  thresh_num = 2;
} else if (stream_acc >= PREF_ACC_THRESH_1) { 
  if (PREF_ACC_INCDEC_LENGTH && pref_stream->stream[index].length > PREF_ACC_DISTANCE_2) {
    pref_stream->stream[index].length -= 3;
  } else {
    pref_stream->stream[index].length = PREF_ACC_DISTANCE_2;
  }
  thresh_num = 1;
} else { 
  if (PREF_ACC_INCDEC_LENGTH && pref_stream->stream[index].length > PREF_ACC_DISTANCE_1) {
    pref_stream->stream[index].length -= 4;
  } else {
    pref_stream->stream[index].length = PREF_ACC_DISTANCE_1;
  }
  thresh_num = 0;
}
if (pref_stream->stream[index].dir == 1) {
  Addr newsp = pref_stream->stream[index].ep - pref_stream->stream[index].length;
  if (newsp > pref_stream->stream[index].start_vline) {
    pref_stream->stream[index].sp = newsp;
  } else {
    pref_stream->stream[index].sp = pref_stream->stream[index].start_vline;
  }
} else if  (pref_stream->stream[index].dir == -1) {
  Addr newsp = pref_stream->stream[index].ep + pref_stream->stream[index].length;
  if (newsp < pref_stream->stream[index].start_vline) {
    pref_stream->stream[index].sp = pref_stream->stream[index].ep + pref_stream->stream[index].length;
  } else {
    pref_stream->stream[index].sp = pref_stream->stream[index].start_vline;
  }
} 

STAT_EVENT(PREF_ACC_NUM_1 + thresh_num);

if (pref_stream->stream[index].length > old_length) {
  STAT_EVENT(PREF_ACC_INC_LENGTH);
} else if (pref_stream->stream[index].length < old_length) {
  STAT_EVENT(PREF_ACC_DEC_LENGTH);
}
*/
}


void pref_stream_c::done_func(void) 
{
  int ii;
  uns len;

  for (ii = 0; ii < *m_simBase->m_knobs->KNOB_STREAM_BUFFER_N; ++ii) 
  {
    if (pref_stream->stream[ii].dir == 0 || !pref_stream->stream[ii].valid) 
    {
      len = 0;
    } 
    else if (pref_stream->stream[ii].dir == 1) 
    {
      len = pref_stream->stream[ii].ep - pref_stream->stream[ii].start_vline + 1;
    } 
    else 
    {
      len = pref_stream->stream[ii].start_vline - pref_stream->stream[ii].ep + 1;
    }			      

    if (len != 0)
      STAT_EVENT( STREAM_LENGTH_0 + MIN2(len / 10, 10));
  }
}

/*
   void pref_stream_runahead(Counter op_num)
   {
// Try to run ahead each stream having an accuracy > STREAM_RA_ACC by STREAM_RA_NUM 
// on a l2 miss.
int ii, jj;
float stream_acc;
bool  queue_full = false;
float pref_acc = (pref_stream->pref_issued > 100) ?((float)pref_stream->pref_useful)/((float)pref_stream->pref_issued):1;

if (pref_stream->curr_op_num != op_num) {
pref_stream->ra_num_sent = 0;
pref_stream->ra_stream_index = 0;
pref_stream->curr_op_num = op_num;
STAT_EVENT(STREAM_ENTER_RA);
}

for ( jj = pref_stream->ra_num_sent;  jj < STREAM_RA_NUM; jj++) {
for ( ii = pref_stream->ra_stream_index ; ii < STREAM_BUFFER_N ; ii++) {
stream_acc = pref_acc_stream_getacc(ii, pref_acc );
if (stream_acc > STREAM_RA_ACC && pref_stream->stream[ii].trained && pref_stream->stream[ii].valid) {	  
// Send out request
Pref_Mem_Req new_req;

new_req.line_index         = pref_stream->stream[ii].ep + pref_stream->stream[ii].dir;
new_req.line_addr          = (new_req.line_index) << LOG2_DCACHE_LINE_SIZE;
//	  new_req.hit_stream_buffer  = ii;
new_req.valid              = true;

if (pref_stream->pref_req_queue[stream_pref_req_no%PREF_REQ_Q_SIZE].valid) {
queue_full = true;
break;
}

pref_stream->pref_req_queue[stream_pref_req_no++%PREF_REQ_Q_SIZE] = new_req;

pref_stream->stream[ii].ep = pref_stream->stream[ii].ep + pref_stream->stream[ii].dir;
STAT_EVENT(STREAM_BUFFER_REQ); 
}
}
if (queue_full) {
break;
}      
}
pref_stream->ra_stream_index = ii;
pref_stream->ra_num_sent = jj;
}

*/


void pref_stream_c::pref_stream_throttle_fb(void)
{
  // on pref_dhal, we update the dyn_degree based on sent pref
  if (*m_simBase->m_knobs->KNOB_PREF_DHAL) 
  { 
    pref_stream->distance = pref_stream->hwp_info->dyn_degree;
  } 
  else 
  {
    hwp_common->pref_get_degfb(pref_stream->hwp_info->id);
    ASSERT(pref_stream->hwp_info->dyn_degree<=4 );
    //		ASSERT(pref_stream->hwp_info->dyn_degree>=0 && pref_stream->hwp_info->dyn_degree<=4 );
    pref_stream->distance = pref_stream->pref_degree_vals[pref_stream->hwp_info->dyn_degree];
    pref_stream->num_tosend = pref_stream->num_tosend_vals[pref_stream->hwp_info->dyn_degree];
  }
}