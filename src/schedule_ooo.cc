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


/*******************************************************************************
 * File         : schedule_ooo.cc
 * Author       : Hyesoon Kim
 * Date         : 1/1/2008 
 * SVN          : $Id: main.cc,v 1.26 2008-09-21 00:02:54 kacear Exp $:
 * Description  : out-of-order scheduler
 ******************************************************************************/


/*
 * Summary: Out-of-Order scheduler
 */


#include "schedule_ooo.h"
#include "pqueue.h"
#include "exec.h"
#include "assert.h"
#include "core.h"
#include "memory.h"
#include "rob_smc.h"

#include "debug_macros.h"
#include "knob.h"
#include "statistics.h"

#include "all_knobs.h"

#define DEBUG(args...)   _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_SCHEDULE_STAGE, ## args) 


// schedule_ooo_c constructor
schedule_ooo_c::schedule_ooo_c(int core_id, pqueue_c<int>** alloc_q, rob_c* rob, 
    exec_c* exec, Unit_Type unit_type, frontend_c* frontend, macsim_c* simBase)
: schedule_c(exec, core_id, unit_type, frontend, alloc_q, simBase)
{
  m_rob = rob;
  m_simBase = simBase;
}


// schedule_ooo_c destructor
schedule_ooo_c::~schedule_ooo_c(void)
{
}


// main execution routine
// In every cycle, schedule uops from rob
void schedule_ooo_c::run_a_cycle(void)
{
  // Check if the schedule isn't running
  if (!is_running())
    return;

  m_cur_core_cycle =  m_simBase->m_core_cycle[m_core_id];

  // clear execution ports
  m_exec->clear_ports();

  int count = 0;
  if (m_num_in_sched) { 
    for (int i = m_first_schlist_ptr; i != m_last_schlist_ptr; i = (i + 1) % MAX_SCHED_SIZE) {
      if (m_schedule_list[i] != -1) {
        SCHED_FAIL_TYPE sched_fail_reason;

        // schedule un uop
        if (uop_schedule(m_schedule_list[i], &sched_fail_reason)) {
          STAT_CORE_EVENT(m_core_id, SCHED_FAILED_REASON_SUCCESS);
          if (i == m_first_schlist_ptr) {
            m_first_schlist_ptr = (m_first_schlist_ptr + 1) % MAX_SCHED_SIZE;
          }

          m_schedule_list[i] = -1;
          ++count;

          // schedule enough uops, break it
          if (m_knob_sched_to_width && count >= m_knob_width) 
            break;
        }
        else {
          // schedule has been failed for current uop
          // try to find next available one
          STAT_CORE_EVENT(m_core_id, 
              SCHED_FAILED_REASON_SUCCESS + MIN2(sched_fail_reason, 2));
        }
      } 
      else if (i == m_first_schlist_ptr) {
        m_first_schlist_ptr = (m_first_schlist_ptr + 1) % MAX_SCHED_SIZE;
      }
    }

    // no uop has been scheduled
    if (m_count == 0) 
      STAT_CORE_EVENT(m_core_id, NUM_NO_SCHED_CYCLE);
  }
  else {
    // no uop in scheduler
    STAT_CORE_EVENT(m_core_id, NUM_SCHED_IDLE_CYCLE);
  }


  // advance uops from alloc queue to schedule queue
  for (int i = 0; i < max_ALLOCQ; ++i) {
    this->advance(i);
  }
}

