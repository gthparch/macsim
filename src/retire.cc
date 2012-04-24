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
 * File         : retire.cc
 * Author       : Hyesoon Kim
 * Date         : 3/17/2008
 * CVS          : $Id: retire.cc 867 2009-11-05 02:28:12Z kacear $:
 * Description  : retirement logic
                  origial author: Jared W.Stark  imported from
 *********************************************************************************************/


///////////////////////////////////////////////////////////////////////////////////////////////
/// \page retire Retirement stage
///
/// This models retire (commit) stage in the processor pipeline. All instructions are retired 
/// in-order. However, basic execution is in the micro-op granularity. Thus, retirement 
/// should carefully handle this cases.
/// \li <c>Instruction termination condition</c> - All uops of an instruction retired in-order
/// \li <c>Thread termination condition</c> - Last uop of a thread
/// \li <c>Process termination condition</c> - # of thread terminated == # of thread created
///
/// \section retire_cpu CPU retirement stage
/// Check the front uop in the rob (in-order retirement).
///
/// \section retire_gpu GPU retirement stage
/// Since there are possibly many ready-to-retire uops from multiple threads. From all
/// ready-to-retire uops from all threads, we sort them based on the ready cycle (age).
///
/// \todo We need to check thread termination condition carefully.
///////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////
/// \page repeat Repeating traces
///
/// \section repeat_1 What is repeating traces?
/// When an application has been terminated, run this application again
///
/// \section repeat_2 Why do we need to repeat traces?
/// In typical multi-programmed workloads (not multi-threaded), whan an application is 
/// terminated earlier than other applications, typically we keep running early-terminated 
/// application until the last application is terminated.
///
/// \section repeat_3 How to enable repeating trace?
/// There are two ways to enable repeating traces.
/// \li Multi-programmed workloads - set <b>*m_simBase->m_knobs->KNOB_REPEAT_TRACE 1</b>
/// \li Single-application - set <b>*m_simBase->m_knobs->KNOB_REPEAT_TRACE 1</b> and 
/// <b>set *m_simBase->m_knobs->KNOB_REPEAT_TRACE_N positive number</b>
///////////////////////////////////////////////////////////////////////////////////////////////


#include "bug_detector.h"
#include "core.h"
#include "frontend.h"
#include "process_manager.h"
#include "retire.h"
#include "rob.h"
#include "rob_smc.h"
#include "uop.h"

#include "config.h"

#include "knob.h"
#include "debug_macros.h"
#include "statistics.h"

#include "all_knobs.h"

#define DEBUG(args...)   _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_RETIRE_STAGE, ## args)


// retire_c constructor
retire_c::retire_c(RETIRE_INTERFACE_PARAMS(), macsim_c* simBase) : RETIRE_INTERFACE_INIT() 
{
  m_simBase = simBase;

  m_retire_running      = false;
  m_total_insts_retired = 0;

  RETIRE_CONFIG();

  if (m_knob_ptx_sim)
    m_knob_width = 1000;
}


// retire_c destructor
retire_c::~retire_c()
{
}


// run_a_cycle : try to commit (retire) instructions every cycle
// Check front ROB (reorder buffer) entry and see whehther it is completed
// If there are multiple completed uops, commit until pipeline width
void retire_c::run_a_cycle()
{
  // check whether retire stage is running
  if (!m_retire_running) {
    return;
  }

  m_cur_core_cycle = m_simBase->m_core_cycle[m_core_id];
  core_c *core = m_simBase->m_core_pointers[m_core_id];
    
  vector<uop_c*>* uop_list = NULL;
  unsigned int uop_list_index = 0;
  if (m_knob_ptx_sim && *m_simBase->m_knobs->KNOB_GPU_SCHED) {
    // GPU : many retireable uops from multiple threads. Get entire retireable uops
    uop_list = m_gpu_rob->get_n_uops_in_ready_order(m_knob_width, m_cur_core_cycle);
  }


  // retire instructions : all micro-ops within an inst. need to be retired for an inst.
  for (int count = 0; count < m_knob_width; ++count) {
    uop_c* cur_uop;
    rob_c* rob;

    // we need to handle retirement for x86 and ptx separately
    
    // retirement logic for GPU
    if (m_knob_ptx_sim && *m_simBase->m_knobs->KNOB_GPU_SCHED) {
      // GPU : many retireable uops from multiple threads. Get entire retireable uops
      if (uop_list_index == uop_list->size()) {
        uop_list->clear();
        break;
      }

      cur_uop = uop_list->at(uop_list_index++);

      rob = m_gpu_rob->get_thread_rob(cur_uop->m_thread_id);
      rob->pop();
    }
    // retirement logic for x86 simulation
    else {
      rob = m_rob;

      // rob is empty;
      if (rob->entries() == 0) {
        break;
      }

      cur_uop = rob->front();

      // uop cannot be retired
      if (!cur_uop->m_done_cycle || cur_uop->m_done_cycle > m_cur_core_cycle) {
        break;
      }

      rob->pop();
      POWER_CORE_EVENT(m_core_id, POWER_REORDER_BUF_R);
      POWER_CORE_EVENT(m_core_id, POWER_INST_COMMIT_SEL_LOGIC_R);
    }


    // all uops belong to previous instruction have been retired : inst_count++
    // nagesh - why are we marking the instruction as retired when the uop
    // marked BOM is retired? shouldn't we do that when the EOM is retired?
    // nagesh - ISTR that I tried changing and something failed - not 100% 
    // sure though : (jaekyu) I think this is the matter of the design. we can update
    // everything from the first uop of an instruction.
    if (cur_uop->m_isitBOM) {
      if (cur_uop->m_uop_type >= UOP_FCF && cur_uop->m_uop_type <= UOP_FCMOV) {
        STAT_EVENT(FP_OPS_TOT);
        STAT_CORE_EVENT(cur_uop->m_core_id, FP_OPS);
      }

      ++m_insts_retired[cur_uop->m_thread_id];
      ++m_total_insts_retired;
      ++m_period_inst_count;

      STAT_CORE_EVENT(cur_uop->m_core_id, INST_COUNT);
      POWER_CORE_EVENT(cur_uop->m_core_id, POWER_PIPELINE);
      STAT_EVENT(INST_COUNT_TOT);
    }


    // GPU : barrier
    if (m_knob_ptx_sim && cur_uop->m_bar_type == BAR_FETCH) {
      frontend_c *frontend = core->get_frontend();
      frontend->synch_thread(cur_uop->m_block_id, cur_uop->m_thread_id);
    }


    // Terminate thread : current uop is last uop of a thread, so we can retire a thread now
    thread_s* thread_trace_info = core->get_trace_info(cur_uop->m_thread_id);
    process_s *process = thread_trace_info->m_process;
    if (cur_uop->m_last_uop || m_insts_retired[cur_uop->m_thread_id] >= *m_simBase->m_knobs->KNOB_MAX_INSTS) {
      core->m_thread_reach_end[cur_uop->m_thread_id] = true;
      ++core->m_num_thread_reach_end;
      if (!core->m_thread_finished[cur_uop->m_thread_id]) {
        DEBUG("core_id:%d thread_id:%d terminated\n", m_core_id, cur_uop->m_thread_id);

        // terminate thread
        m_simBase->m_process_manager->terminate_thread(m_core_id, thread_trace_info, \
            cur_uop->m_thread_id, cur_uop->m_block_id);

        // disable current thread's fetch engine
        if (!core->m_fetch_ended[cur_uop->m_thread_id]) {
          core->m_fetch_ended[cur_uop->m_thread_id] = true;
          core->m_fetching_thread_num--;
        }

        // all threads in an application have been retired. Thus, we can retire an appliacation
        if (process->m_no_of_threads_terminated == process->m_no_of_threads_created) {
          if (process->m_current_vector_index == process->m_applications.size() 
            || (*m_simBase->m_ProcessorStats)[INST_COUNT_TOT].getCount() >= *KNOB(KNOB_MAX_INSTS1)) {
            update_stats(process);
            m_simBase->m_process_manager->terminate_process(process);
            if (m_simBase->m_process_count_without_repeat == 0) {
              m_simBase->m_repeat_done = true;
            }
            repeat_traces(process);
          }
          else {
            m_simBase->m_process_manager->terminate_process(process);
          }

          // schedule new threads
          m_simBase->m_process_manager->sim_thread_schedule();
        }

        // schedule new threads
        m_simBase->m_process_manager->sim_thread_schedule();
      }
    } // terminate_thread


    // update number of retired uops
    ++m_uops_retired[cur_uop->m_thread_id];

    DEBUG("core_id:%d thread_id:%d retired_insts:%lld uop->inst_num:%lld uop_num:%lld " 
        "done_cycle:%lld\n",
        m_core_id, cur_uop->m_thread_id, m_insts_retired[cur_uop->m_thread_id], 
        cur_uop->m_inst_num, cur_uop->m_uop_num, cur_uop->m_done_cycle);


    // free uop
    for (int ii = 0; ii < cur_uop->m_num_child_uops; ++ii) {
			if (*m_simBase->m_knobs->KNOB_BUG_DETECTOR_ENABLE)
	      m_simBase->m_bug_detector->deallocate(cur_uop->m_child_uops[ii]);
      m_uop_pool->release_entry(cur_uop->m_child_uops[ii]->free());
    }

    if (*m_simBase->m_knobs->KNOB_BUG_DETECTOR_ENABLE)
			m_simBase->m_bug_detector->deallocate(cur_uop);

    delete [] cur_uop->m_child_uops;
    m_uop_pool->release_entry(cur_uop->free());
  
    // release physical registers
    if (cur_uop->m_req_lb) {
      rob->dealloc_lb();
    }
    if (cur_uop->m_req_sb) {
      rob->dealloc_sb();
    }
    if (cur_uop->m_req_int_reg) {
      rob->dealloc_int_reg();
    }
    if (cur_uop->m_req_fp_reg) {
      rob->dealloc_fp_reg();
    }
  }

  if (m_core_id == 0) {
    m_simBase->m_core0_inst_count = m_insts_retired[0];
  }
}


// When a new thread has been scheduled, reset data
void retire_c::allocate_retire_data(int tid)
{
  m_insts_retired[tid] = 0;
  m_uops_retired[tid]  = 0;
}


void retire_c::start()
{
  m_retire_running = true;
}


void retire_c::stop()
{
  m_retire_running = false;
}


bool retire_c::is_running()
{
  return m_retire_running;
}



#if 0
// return number of retired instructions per thread
inline Counter retire_c::get_instrs_retired(int thread_id) 
{ 
  return m_insts_retired[thread_id]; 
}
#endif


// return number of retired uops per thread
Counter retire_c::get_uops_retired(int thread_id) 
{ 
  return m_uops_retired[thread_id]; 
}


// return total number of retired instructions
Counter retire_c::get_total_insts_retired() 
{ 
  return m_total_insts_retired; 
}


// whan an application is completed, update corresponding stats
void retire_c::update_stats(process_s* process)
{
  core_c* core = m_simBase->m_core_pointers[m_core_id];

  // repeating traces in case of running multiple applications
  // TOCHECK I will get back to this later
  if (*KNOB(KNOB_REPEAT_TRACE) && process->m_repeat < *KNOB(KNOB_REPEAT_TRACE_N) &&
      core->get_core_type() == "ptx") {
    if ((process->m_repeat+1) == *m_simBase->m_knobs->KNOB_REPEAT_TRACE_N) {
      --m_simBase->m_process_count_without_repeat;
      STAT_EVENT_N(CYC_COUNT_PTX, CYCLE);
      report("application " << process->m_process_id << " terminated " 
          << "(" << process->m_applications[process->m_current_vector_index-1] 
          << "," << process->m_repeat << ") at " << CYCLE);
    }
  } 
  else {
    if (process->m_repeat == 0) {
      if (core->get_core_type() == "ptx") {
        STAT_EVENT_N(CYC_COUNT_PTX, CYCLE);
      }
      else {
        STAT_EVENT_N(CYC_COUNT_X86, CYCLE);
      }
      --m_simBase->m_process_count_without_repeat;
      report("----- application " << process->m_process_id << " terminated (" 
          << process->m_applications[process->m_current_vector_index-1] 
          << "," << process->m_repeat << ") at " << CYCLE);
    }
  }
}
          

// repeat (terminated) trace, if necessary
void retire_c::repeat_traces(process_s* process)
{
  if ((*KNOB(KNOB_REPEAT_TRACE) || (*KNOB(KNOB_REPEAT_TRACE) && *KNOB(KNOB_REPEAT_TRACE_N) > 0)) && 
      m_simBase->m_process_count_without_repeat > 0) {
    // create duplicate process once previous one is terminated
    m_simBase->m_process_manager->create_process(process->m_kernel_config_name, process->m_repeat+1, 
        process->m_orig_pid);
    STAT_EVENT(NUM_REPEAT);
  }
}
