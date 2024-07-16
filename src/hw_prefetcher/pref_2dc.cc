/***************************************************************************************
 * File         : pref_2dc.c
 * Author       : Santhosh Srinath
 * Date         : 1/19/2006
 * CVS          : $Id: pref_2dc.cc,v 1.2 2008/09/12 03:42:01 kacear Exp $:
 * Description  : 
 ***************************************************************************************/


#include "pref_2dc.h"

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

#include "../all_knobs.h"

#include "../core.h"

/* 
   2dc_prefetcher : 2 delta-correlation prefetcher
  
   O.k... So far 2-delta correlation prefetchers have just gone for
   the basic approach - 2-d table. So we are implementing a cache like
   table which can achieve most of the benefits from a much smaller
   structure.

   Implementation -> Take the deltas and the PC, and the address and
   come up with a hash function that works. Use this to access the 
   cache.
*/

/**************************************************************************************/
/* Macros */
#define DEBUG(args...)		_DEBUG(DEBUG_PREF_2DC, ## args)


/**************************************************************************************/
/* Global variables */
// Pref_2DC * tdc_hwp;


pref_2dc_c::pref_2dc_c(hwp_common_c *hcc, Unit_Type type, macsim_c* simBase)
: pref_base_c(simBase)
{
  name = "2dc";
  hwp_type = Mem_To_UL1;
  hwp_common = hcc;
  switch (type) {
  case UNIT_SMALL:
    knob_enable = *m_simBase->m_knobs->KNOB_PREF_2DC_ON;
    break;
  case UNIT_MEDIUM:
    knob_enable = *m_simBase->m_knobs->KNOB_PREF_2DC_ON_MEDIUM_CORE;
    break;
  case UNIT_LARGE:
    knob_enable = *m_simBase->m_knobs->KNOB_PREF_2DC_ON_LARGE_CORE;
    break;
  }	


  l2_miss = true;
  l2_pref_hit = true;
}


pref_2dc_c::~pref_2dc_c()
{
}

void pref_2dc_c::init_func(int core_id)
{
  if (!knob_enable) 
    return;

  core_id = core_id;

  // CHECKME
  // tdc_hwp              = (Pref_2DC*)malloc(sizeof(Pref_2DC));
  this->hwp_info->enabled = true;

  this->regions     = (Pref_2DC_Region*)calloc(*m_simBase->m_knobs->KNOB_PREF_2DC_NUM_REGIONS, sizeof(Pref_2DC_Region));

  this->last_access = 0;
  this->last_loadPC = 0;

  // CHECKME
  // init_cache(&this->cache, "PREF_2DC_CACHE", *m_simBase->m_knobs->KNOB_PREF_2DC_CACHE_SIZE, *m_simBase->m_knobs->KNOB_PREF_2DC_CACHE_ASSOC, *m_simBase->m_knobs->KNOB_PREF_2DC_CACHE_LINE_SIZE, sizeof(Pref_2DC_Cache_Data), *m_simBase->m_knobs->KNOB_REPL_true_LRU);    
  cache = new cache_c("PREF_2DC_CACHE", *m_simBase->m_knobs->KNOB_PREF_2DC_CACHE_SIZE, *m_simBase->m_knobs->KNOB_PREF_2DC_CACHE_ASSOC, *m_simBase->m_knobs->KNOB_PREF_2DC_CACHE_LINE_SIZE, sizeof(Pref_2DC_Cache_Data), *m_simBase->m_knobs->KNOB_PREF_2DC_BANKS, false, -1, CACHE_DL2, false, 1, 0, m_simBase);    

  this->cache_index_bits = log2_int(*m_simBase->m_knobs->KNOB_PREF_2DC_CACHE_SIZE/4);
  this->hash_func   = PREF_2DC_HASH_FUNC_DEFAULT;
  this->pref_degree = *m_simBase->m_knobs->KNOB_PREF_2DC_DEGREE;
}


void pref_2dc_c::l2_pref_hit_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  pref_2dc_l2_train(tid, lineAddr, loadPC, true);
}


void pref_2dc_c::l2_miss_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  pref_2dc_l2_train(tid, lineAddr, loadPC, false);
}


void pref_2dc_c::pref_2dc_l2_train(int tid, Addr lineAddr, Addr loadPC, bool l2_hit)
{
  int delta;
  Addr hash;
  Addr lineIndex = lineAddr >> LOG2_DCACHE_LINE_SIZE;
  Addr dummy_lineaddr;
  Pref_2DC_Region * region = &this->regions[(lineIndex >> *m_simBase->m_knobs->KNOB_PREF_2DC_ZONE_SHIFT) % *m_simBase->m_knobs->KNOB_PREF_2DC_REGION_HASH];
    
  if (this->last_access != 0) {	
    delta = lineIndex - this->last_access;
    if (delta == 0) { // no point updating if we have the same address twice
      return;
    }
    // update state of the cache for deltaA, deltaB
    // no point inserting same deltas in. so if deltaA == deltaB and deltaB == delta then dont insert.
    if (region->deltaA != 0 && region->deltaB != 0 && (!(region->deltaA == region->deltaB && region->deltaB == delta))) {
      hash = pref_2dc_hash(this->last_access, this->last_loadPC, region->deltaA, region->deltaB);
      // CHECKME
      // FIXME (jaekyu, 4-26-2011) use appropriate application id
      Pref_2DC_Cache_Data * data = (Pref_2DC_Cache_Data *)cache->access_cache(hash, &dummy_lineaddr, true, 0);
      if (!data) {
        Addr repl_addr;
        if (!l2_hit) { //insert only on miss
          data = (Pref_2DC_Cache_Data *)cache->insert_cache(hash, &dummy_lineaddr, &repl_addr, 0, false);
        } else {
          return;
        }
      }
      data->delta = delta;
    }
    region->deltaC = region->deltaB;
    region->deltaB = region->deltaA;
    region->deltaA = delta;
  }
  this->last_access = lineIndex;
  this->last_loadPC = loadPC;
    
  if (region->deltaA == 0 || region->deltaB == 0) {
    return;
  } // No useful deltas yet
    
  {
    // Send out prefetches
    Pref_2DC_Cache_Data * data;
    uns num_pref_sent = 0;
    int delta1 = region->deltaB;
    int delta2 = region->deltaA;
	
    if (region->deltaA == region->deltaB && region->deltaB == region->deltaC) {
      // Now just assume that this is a strided access and send out the next few.
      for (; num_pref_sent<this->pref_degree; num_pref_sent++) {
        lineIndex += region->deltaA;
        hwp_common->pref_addto_l2req_queue_set(lineIndex, this->hwp_info->id, (num_pref_sent==0), (num_pref_sent=this->pref_degree), loadPC);
      }
    }
    while (num_pref_sent < this->pref_degree) {
      hash = pref_2dc_hash(lineIndex, loadPC, delta1, delta2);
      // FIXME (jaekyu, 4-26-2011) use appropriate application id
      data = (Pref_2DC_Cache_Data *)cache->access_cache(hash, &dummy_lineaddr, true, 0);
      if (!data) { // no hit for this hash
        return;
      }
      lineIndex += data->delta;

      delta1 = delta2;
      delta2 = data->delta;
	    
      hwp_common->pref_addto_l2req_queue_set(lineIndex, this->hwp_info->id, (num_pref_sent==0), (num_pref_sent=this->pref_degree), loadPC);
      num_pref_sent++;
    }
  }
}


void pref_2dc_c::pref_2dc_throttle(void)
{
  int dyn_shift = 0;

  float acc = hwp_common->pref_get_accuracy(this->hwp_info->id);

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


Addr pref_2dc_c::pref_2dc_hash(Addr lineIndex, Addr loadPC, int deltaA, int deltaB)
{
  Addr res = 0;
  uns  cache_indexbitsA;
  uns  cache_indexbitsB;
  uns  tagbits;

  if (0) { // to prevent compilation error, jaekyu (11-3-2009)
    printf("%lld", loadPC);
  }

  switch(this->hash_func) {
  case PREF_2DC_HASH_FUNC_DEFAULT:
    // In this function, we just use the lower bits from each delta 
    // to form the hash.
    cache_indexbitsA = this->cache_index_bits >> 1;
    cache_indexbitsB = this->cache_index_bits - cache_indexbitsA;

    tagbits = (((deltaA >> cache_indexbitsA) ^ (deltaB >> cache_indexbitsB) ^
                ( lineIndex >> *m_simBase->m_knobs->KNOB_PREF_2DC_ZONE_SHIFT) ) & N_BIT_MASK(*m_simBase->m_knobs->KNOB_PREF_2DC_TAG_SIZE));
		    
    res = (((deltaA & N_BIT_MASK(cache_indexbitsA)) | ((deltaB & N_BIT_MASK(cache_indexbitsB))<<cache_indexbitsA)) 
           | (tagbits << this->cache_index_bits));
    break;
  default:
    ASSERT(0);
    break;
  }
  return res;
}