/**********************************************************************************************
 * File         : pref.cc 
 * Author       : Jaekyu Lee
 * Date         : 2/14/2011
 * SVN          : $Id: dram.cc 912 2009-11-20 19:09:21Z kacear $
 * Description  : Prefetcher base class
 *********************************************************************************************/


#include "pref.h"

#include "all_knobs.h"

static int prefetcher_id = 0;

// hardware prefetcher base class constructor
pref_base_c::pref_base_c(macsim_c* simBase)
  : init(true), done(false), l1_miss(false), l1_hit(false), l1_pref_hit(false),
  l2_miss(false), l2_hit(false), l2_pref_hit(false)
{
  m_simBase = simBase;

  // hwp_info holds all hardware prefetcher related information
  hwp_info = new pref_info_s;

  hwp_info->id                = prefetcher_id++;
  hwp_info->useful            = 0;
  hwp_info->sent              = 0;
  hwp_info->late              = 0;
  hwp_info->curr_useful       = 0;
  hwp_info->curr_sent         = 0;
  hwp_info->curr_late         = 0;
  hwp_info->priority          = 0;
  hwp_info->enabled           = false;
  hwp_info->hybrid_lastuseful = 0;
  hwp_info->hybrid_lastsent   = 0;
  hwp_info->prefhit_count     = 0;
  hwp_info->dyn_degree        = 2;


  // Accuracy Studies
  // Sample at periodic intervals and come up a histogram of usage at
  // different accuracies
  // Initialize variable and file
  if (*simBase->m_knobs->KNOB_PREF_ACC_STUDY) {
    hwp_info->track_num = 0;
    hwp_info->track_lastsample_cycle = m_simBase->m_simulation_cycle;

    // Initialize the accuracy buckets
    for (int kk = 0; kk < 10; ++kk) { 
      for (int jj = 0; jj < PREF_TRACKERS_NUM; ++jj) {
        hwp_info->trackhist[kk][jj] = 0;
      }
    }
  }
}


