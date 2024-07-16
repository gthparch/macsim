/***************************************************************************************
 * File         : pref_phase.c
 * Author       : Santhosh Srinath
 * Date         : 
 * CVS          : $Id: pref_phase.cc,v 1.2 2008/09/12 03:42:02 kacear Exp $:
 * Description  : 
 ***************************************************************************************/


#include "pref_phase.h"

#include "../global_defs.h"
#include "../global_types.h"
#include "../debug_macros.h"

#include "../utils.h"
#include "../assert_macros.h"
#include "../uop.h"

#include "../cache.h"
#include "../statistics.h"
#include "../memory.h"
#include "../core.h"

#include "../all_knobs.h"

/**************************************************************************************/
/* Macros */
#define DEBUG(args...)		_DEBUG(DEBUG_PREF_PHASE, ## args)

#define PAGENUM(x)              (x>>*m_simBase->m_knobs->KNOB_PREF_PHASE_LOG2REGIONSIZE)

/**************************************************************************************/
/* Global variables */
// Pref_PHASE * phase_hwp;
// FIXME

FILE * PREF_PHASE_OUT;


//pref_phase_c::pref_phase_c() {}

pref_phase_c::pref_phase_c(hwp_common_c *hcc, Unit_Type type, macsim_c* simBase)
: pref_base_c(simBase)
{
  name = "phase";
  hwp_type = Mem_To_UL1;
  hwp_common = hcc;
  switch (type) {
  case UNIT_SMALL:
    knob_enable = *m_simBase->m_knobs->KNOB_PREF_PHASE_ON;
    break;
  case UNIT_MEDIUM:
    knob_enable = *m_simBase->m_knobs->KNOB_PREF_PHASE_ON_MEDIUM_CORE;
    break;
  case UNIT_LARGE:
    knob_enable = *m_simBase->m_knobs->KNOB_PREF_PHASE_ON_LARGE_CORE;
    break;
  }	
  
  l2_miss = true;
  l2_hit = true;
  l2_pref_hit = true;

}

void pref_phase_c::init_func(int cid)
{
  int ii;
  static char pref_phase_filename[] = "pref_phase";

  core_id = cid;

  if (!knob_enable) 
    return;

  // phase_hwp              = (Pref_PHASE*)malloc(sizeof(Pref_PHASE));
  this->hwp_info->enabled = true;
    
  this->phase_table = (PhaseInfoEntry *)calloc(*m_simBase->m_knobs->KNOB_PREF_PHASE_TABLE_SIZE,sizeof(PhaseInfoEntry));
    
  for (ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_PHASE_TABLE_SIZE; ii++) {
    this->phase_table[ii].MemAccess = (bool *)calloc(*m_simBase->m_knobs->KNOB_PREF_PHASE_INFOSIZE, sizeof(bool)); 
    this->phase_table[ii].mapped_regions = (Phase_Region *)calloc(*m_simBase->m_knobs->KNOB_PREF_PHASE_TRACKEDREGIONS, sizeof(Phase_Region));
  }
  this->MemAccess      = (bool *) calloc(*m_simBase->m_knobs->KNOB_PREF_PHASE_INFOSIZE, sizeof(bool));
  this->mapped_regions = (Phase_Region *) calloc(*m_simBase->m_knobs->KNOB_PREF_PHASE_TRACKEDREGIONS, sizeof(Phase_Region));
  this->interval_start = 0;
  this->curr_phaseid   = 0;
  this->num_misses     = 0;
    
  if (*m_simBase->m_knobs->KNOB_PREF_PHASE_STUDY) {
    PREF_PHASE_OUT = file_tag_fopen(pref_phase_filename, "w", m_simBase);
  }
}

// CHECKME
void pref_phase_c::l2_hit_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  // Do nothing on a l2 hit
  //    pref_phase_l2_train(lineAddr, loadPC, true);
  if (0) {
    printf("%lld %lld", lineAddr, loadPC);
  }
}

void pref_phase_c::l2_pref_hit_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  pref_phase_l2_train(tid, lineAddr, loadPC, true);
}

void pref_phase_c::l2_miss_func(int tid, Addr lineAddr, Addr loadPC, uop_c *uop)
{
  pref_phase_l2_train(tid, lineAddr, loadPC, false);    
}

void pref_phase_c::pref_phase_l2_train(int tid, Addr lineAddr, Addr loadPC, bool pref_hit)
{
  if (0) { // to prevent compilation error, jaekyu (11-3-2009)
    printf("%lld %lld %d", lineAddr, loadPC, pref_hit);
  }
  // CHECKME / Jaekyu / If we want to use phase prefetcher, then each core should contain one phare prefetcher
#if NEED_TO_DEBUG
  int next_phaseid;

  bool qFull = false;
  Addr lineIndex = lineAddr >> LOG2_DCACHE_LINE_SIZE;
  Addr hashIndex = lineIndex % *m_simBase->m_knobs->KNOB_PREF_PHASE_PRIME_HASH;
    
  // Update access pattern
  this->MemAccess[hashIndex] = true; 
   
  pref_phase_updateregioninfo(this->mapped_regions, lineIndex);

  this->num_misses++;
  if (m_inst_count - this->interval_start > *m_simBase->m_knobs->KNOB_PREF_PHASE_INTERVAL) {
    this->interval_start = m_inst_count;
    if (this->num_misses > *m_simBase->m_knobs->KNOB_PREF_PHASE_MIN_MISSES) {
      if (*m_simBase->m_knobs->KNOB_PREF_PHASE_STUDY) {
        int ii;
        for (ii = 0; ii<*m_simBase->m_knobs->KNOB_PREF_PHASE_INFOSIZE; ii++) {
          fprintf(PREF_PHASE_OUT, (this->MemAccess[ii] ? "1" : "0"));
        }
        fprintf(PREF_PHASE_OUT, "\n");
      }

      this->num_misses = 0;
      next_phaseid = pref_phase_computenextphase();
      STAT_EVENT(PREF_PHASE_NEWPHASE_DET);
      {
        // Set the memAccess pattern for the next phase correctly
        bool * tmp = this->MemAccess;
        this->MemAccess = this->phase_table[next_phaseid].MemAccess;
        this->phase_table[next_phaseid].MemAccess = tmp;
			
        memset(this->MemAccess, 0, sizeof(bool)**m_simBase->m_knobs->KNOB_PREF_PHASE_INFOSIZE);
      }
      {
        // Set the mapped regions for the current region correctly
        Phase_Region * tmp = this->mapped_regions;
        this->mapped_regions = this->phase_table[this->curr_phaseid].mapped_regions;
        this->phase_table[this->curr_phaseid].mapped_regions = tmp;
			
        memset(this->mapped_regions, 0, sizeof(Phase_Region)**m_simBase->m_knobs->KNOB_PREF_PHASE_TRACKEDREGIONS);
      }
		    
      if (!this->phase_table[next_phaseid].m_valid) {
        STAT_EVENT(PREF_PHASE_NEWPHASE_NOTVALID);
				
        this->phase_table[next_phaseid].m_valid = true;
        memset(this->phase_table[next_phaseid].mapped_regions, 0, sizeof(Phase_Region)**m_simBase->m_knobs->KNOB_PREF_PHASE_TRACKEDREGIONS);
      }
		    
      this->curr_phaseid = next_phaseid;
      this->phase_table[next_phaseid].last_access = g_simulation_cycle;
      this->currsent_regid = 0;
      this->currsent_regid_offset = 0;
    }
  }
  // Send out prefetches
  while (this->currsent_regid < PREF_PHASE_REGIONENTRIES) {
    Addr lineIndex, startIndex;
    Phase_Region * region = &this->phase_table[this->curr_phaseid].mapped_regions[this->currsent_regid];
    if (region->m_valid) {
      startIndex = region->PageNumber<<( *m_simBase->m_knobs->KNOB_PREF_PHASE_LOG2REGIONSIZE - LOG2_DCACHE_LINE_SIZE);
		    
      for (; this->currsent_regid_offset < PREF_PHASE_REGIONENTRIES; this->currsent_regid_offset++) { 
        if ( region->RegionMemAccess[this->currsent_regid_offset]) {
          lineIndex = startIndex + this->currsent_regid_offset;
          if(!hw_prefetcher->pref_addto_l2req_queue(lineIndex, this->hwp_info->m_id)) {
            qFull = true;
            break;
          }
          STAT_EVENT(PREF_PHASE_SENTPREF);
        }
      }
      if (qFull) 
        break;
    }
    this->currsent_regid++;
    this->currsent_regid_offset = 0;
  }
	
#endif
}

void pref_phase_c::pref_phase_updateregioninfo(Phase_Region *mapped_regions, Addr lineAddr)
{
  int ii, id;
  Addr pagenum = PAGENUM(lineAddr);
  int  region_offset = (lineAddr >> LOG2_DCACHE_LINE_SIZE) & N_BIT_MASK(*m_simBase->m_knobs->KNOB_PREF_PHASE_LOG2REGIONSIZE - LOG2_DCACHE_LINE_SIZE);
  id = -1;
    
  for (ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_PHASE_TRACKEDREGIONS; ii++) {
    if (mapped_regions[ii].valid && mapped_regions[ii].PageNumber == pagenum) {
      id = ii; 
      break;
    }
    if (!mapped_regions[ii].valid || id==-1) {
      id = ii;
    } else if (mapped_regions[id].valid && mapped_regions[ii].last_access < mapped_regions[id].last_access) {
      id = ii;
    }
  }
  if (!mapped_regions[id].valid || mapped_regions[id].PageNumber!=pagenum ) {
    memset(mapped_regions[id].RegionMemAccess, 0, sizeof(bool)*PREF_PHASE_REGIONENTRIES);
  }
  if (mapped_regions[id].PageNumber != pagenum) {
    STAT_EVENT(PREF_PHASE_OVERWRITE_PAGE);
  }
  mapped_regions[id].PageNumber  = pagenum;
  mapped_regions[id].last_access = m_simBase->m_simulation_cycle;
  mapped_regions[id].valid       = true;
  mapped_regions[id].RegionMemAccess[region_offset] = true;
}

int pref_phase_c::pref_phase_computenextphase(void)
{
  int ii, jj;
  int id = -1;
  for (ii = 0; ii < *m_simBase->m_knobs->KNOB_PREF_PHASE_TABLE_SIZE; ii++) {
    if (this->phase_table[ii].valid) {
      int diffnum = 0;
      int missnum = 0;
      float missper = 0.0;
      for (jj = 0; jj < *m_simBase->m_knobs->KNOB_PREF_PHASE_INFOSIZE; jj++) {
        if (this->MemAccess[jj] != this->phase_table[ii].MemAccess[jj]) {
          diffnum++;
        }
        if (this->MemAccess[jj] == 1) {
          missnum++;
        }
      }
      missper = (1.0*diffnum) / (1.0*missnum);
      if (diffnum < *m_simBase->m_knobs->KNOB_PREF_PHASE_MAXDIFF_THRESH && missper < *m_simBase->m_knobs->KNOB_PREF_PHASE_MISSPER) {
        // Found a match
        return ii;
      }
    }
    if (id == -1 || !this->phase_table[ii].valid) {
      id = ii;
    } else if (this->phase_table[id].valid && this->phase_table[ii].last_access < this->phase_table[id].last_access) {
      id = ii;
    }
  }
  // Taken another entry... 
  // So set it to invalid
  this->phase_table[id].valid = false;
  return id;
}