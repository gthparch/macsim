/***************************************************************************************
 * File         : pref_phase.h
 * Author       : Santhosh Srinath
 * Date         : 11/16/2004
 * CVS          : $Id: pref_phase.h,v 1.1 2008/07/30 14:18:16 kacear Exp $:
 * Description  : 
 ***************************************************************************************/
#ifndef __PREF_PHASE_H__

#include "../pref_common.h"
#include "../pref.h"

/***************************************************************************************
 * Phase Based Prefetching:
 * This prefetcher works by predicting the future memory access pattern based on
 * the current access pattern. This is currently modeled more as collection of 
 * accesses rather than as a permutation of the accesses -> Order is not important.

 * Currently this prefetcher collects the L2 miss pattern for the current "phase" 
 * which is based on number of instructions retired. 
 *
 * Largest Prime < 16384 = 16381 
 ***************************************************************************************/
#define PREF_PHASE_REGIONENTRIES     64


// This struct keeps info on the regions being targeted.
typedef struct Phase_Region_Struct {
  Addr    PageNumber;
  bool    RegionMemAccess[PREF_PHASE_REGIONENTRIES];       // This is the access pattern for this region

  Counter last_access;
  bool    valid;
} Phase_Region;


typedef struct PhaseInfoEntry_Struct {
  bool         * MemAccess; // This is the access pattern for the whole of memory
  // for the last interval

  Phase_Region * mapped_regions; // Given the last phase, what is the current access pattern

  Counter        last_access;    // used for lru
  bool           valid;
} PhaseInfoEntry;


class pref_phase_c : public pref_base_c
{
 private:
  PhaseInfoEntry    * phase_table;

  Counter           interval_start;

  uns               curr_phaseid;     // Current phase entry we are prefetching for
    
  bool              * MemAccess;       // Current miss pattern - used to find the next phase
  Phase_Region      * mapped_regions;  // Used to update the phase table
    
  uns               currsent_regid;
  uns               currsent_regid_offset;
  Counter           num_misses;

  pref_phase_c();

 public:
  pref_phase_c(hwp_common_c *, Unit_Type, macsim_c*);

  /*************************************************************/
  /* HWP Interface */
  void init_func(int core_id);
  void done_func() {}
  void l1_miss_func(int, Addr, Addr, uop_c *) {}
  void l1_hit_func(int, Addr, Addr, uop_c *) {}
  void l1_pref_hit_func(int, Addr, Addr, uop_c *) {}
  void l2_miss_func(int tid, Addr lineAddr, Addr loadPC, uop_c * );
  void l2_hit_func(int tid, Addr lineAddr, Addr loadPC, uop_c * );
  void l2_pref_hit_func(int tid, Addr lineAddr, Addr loadPC, uop_c * );
  
  void pref_phase_l2_train(int tid, Addr lineAddr, Addr loadPC, bool pref_hit );


  /*************************************************************/
  /* Misc functions */

  void pref_phase_updateregioninfo(Phase_Region *, Addr lineAddr);

  int pref_phase_computenextphase(void);
};


#define __PREF_PHASE_H__
#endif /*  __PREF_PHASE_H__*/