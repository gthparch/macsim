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


