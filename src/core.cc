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
 * File         : core.cc
 * Author       : Hyesoon Kim
 * Date         : 1/7/2008
 * CVS          : $Id: core.cc 911 2009-11-20 19:08:10Z kacear $:
 * Description  : main function
 *********************************************************************************************/


/**********************************************************************************************
 * 1. run_a_cycle : core_c::run_a_cycle function is the main tick/tock function of the
 *   simulator. Each cycle, by running core_c::run_a_cycle(), we simulate each pipeline 
 *   stages in the system.
 *
 * 2. heartbeat : for every N cycle, print out current/accumulative performance information. 
 *   Heartbeat is called when an application or a thread has been terminated as well.
 *
 * 3. Thread data allocation/deallocation : For GPU simulation, we need to simulate massive
 *   number of threads and each thread requires own data. To efficiently manage memory
 *   space, when a thread has been fetched or terminated, we allocate or deallocate its data
 *
 * 4. Core type: we can simulate various type of cores. In-order, Out-of-order, SMT, GPU (SMs).
 *   By setting appropiriate configurations, we can exploit different models.
 *
 * 5. Application ID : If we want to prioritize based on the application, we need application
 *   ID. To get application ID, we maintain hash structures since multiple applications
 *   can be running together (in case of SMT).
 *********************************************************************************************/


#include <stdio.h>
#include <cmath>
#include <map>
#include <sstream>

#include "assert.h"
#include "core.h"
#include "trace_read.h"
#include "knob.h"
#include "bp.h"
#include "retire.h"
#include "uop.h"
#include "allocate.h"
#include "frontend.h"
#include "schedule.h"
#include "memory.h"
#include "exec.h"
#include "pqueue.h"
#include "rob.h"
#include "cache.h"
#include "statistics.h"
#include "utils.h"
#include "pref_common.h"
#include "map.h"
#include "rob_smc.h"
#include "allocate_smc.h"
#include "pref_common.h"
#include "bug_detector.h"
#include "process_manager.h"
#include "fetch_factory.h"
#include "readonly_cache.h"
#include "sw_managed_cache.h"
#include "network.h"
#include "dram.h"

#include "config.h"

#include "schedule_ooo.h"
#include "schedule_io.h"
#include "schedule_smc.h"
#include "debug_macros.h"

#include "all_knobs.h"

///////////////////////////////////////////////////////////////////////////////////////////////


#define DEBUG(args...)   _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_TRACE_READ, ## args)


///////////////////////////////////////////////////////////////////////////////////////////////


void core_c::init(void)
{
  m_unique_scheduled_thread_num       = 0; 
  m_last_terminated_tid               = 0;
  m_running_thread_num                = 0;
  m_fetching_thread_num               = 0;
  m_running_block_num                 = 0;
  m_fetching_block_id                 = -1; 
  m_num_thread_reach_end              = 0; 
  m_max_inst_fetched                  = 0;
  m_core_cycle_count                  = 0; 
  m_unique_uop_num                    = 0;
  m_heartbeat_printed_inst_count_core = 0;
  m_last_inst_count                   = 0;
  m_appl_id                           = 0;

  m_fetch_ended.clear();
  m_thread_reach_end.clear();
  m_thread_finished.clear();
  m_inst_fetched.clear();
  m_ops_to_be_dispatched.clear();
  m_last_fetch_cycle.clear();
  m_terminated_tid.clear();
  m_heartbeat.clear();
  m_tid_to_appl_map.clear();
  m_thread_trace_info.clear();
  m_bp_recovery_info.clear();
}


// core_c constructor
core_c::core_c (int c_id, macsim_c* simBase, Unit_Type type)
{
  // Initialization
  m_core_id         = c_id;
  m_unit_type       = type;
  m_sim_start_time  = time(NULL);
  

  // Reference to simulation base globals
  m_simBase         = simBase;

  // configuration
  CORE_CONFIG();
  

  // memory allocation
  // uop pool
  stringstream sstr;
  string name;
  m_uop_pool = new pool_c<uop_c>(1000, "core_uop_pool");

  // dependence table
  m_map = new map_c(m_simBase); 

  // instruction cache
  m_icache = new cache_c("icache", icache_size, icache_assoc, icache_line_size, 
      sizeof(icache_data_c), icache_banks, icache_bypass, c_id, CACHE_IL1, false, 1, 0, m_simBase);
  m_icache->set_core_id(m_core_id);

  // reorder buffer
  if (m_core_type == "ptx" && *m_simBase->m_knobs->KNOB_GPU_SCHED) {
    m_rob     = NULL;
    m_gpu_rob = new smc_rob_c(m_unit_type, m_core_id, m_simBase);
  }
  else {
    m_rob     = new rob_c(type, m_simBase); 
    m_gpu_rob = NULL;
  }

  // frontend queue
  m_q_frontend = new pqueue_c<int*>(*m_simBase->m_knobs->KNOB_FE_SIZE, 
      (m_knob_fetch_latency + m_knob_alloc_latency), "q_frontend", m_simBase); 

  // allocation queue
  if (m_core_type == "ptx" && *m_simBase->m_knobs->KNOB_GPU_SCHED) {
    m_q_iaq     = NULL;
    m_gpu_q_iaq = new pqueue_c<gpu_allocq_entry_s>* [max_ALLOCQ]; 
  }
  else {
    m_q_iaq     = new pqueue_c<int>* [max_ALLOCQ]; 
    m_gpu_q_iaq = NULL;
  }

  int q_iaq_size[max_ALLOCQ];
  q_iaq_size[gen_ALLOCQ] = giaq_size;
  q_iaq_size[mem_ALLOCQ] = miaq_size;
  q_iaq_size[fp_ALLOCQ]  = fq_size;

  sstr.clear();
  if (m_core_type == "ptx" && *m_simBase->m_knobs->KNOB_GPU_SCHED) {
    for (int i = 0; i < max_ALLOCQ; ++i) {
      sstr << "q_iaq" << i;
      sstr >> name;
      m_gpu_q_iaq[i] = new pqueue_c<gpu_allocq_entry_s>(q_iaq_size[i], 
          (*KNOB(KNOB_ALLOC_TO_EXEC_LATENCY) - *KNOB(KNOB_SCHED_CLOCK)), name.c_str(), m_simBase);
    }
  }
  else {
    for (int i = 0; i < max_ALLOCQ; ++i) {
      sstr << "q_iaq" << i;
      sstr >> name;
      m_q_iaq[i] = new pqueue_c<int>(q_iaq_size[i],
          (*KNOB(KNOB_ALLOC_TO_EXEC_LATENCY) - *KNOB(KNOB_SCHED_CLOCK)), name.c_str(), m_simBase);
    }
  }

  // branch predictor
  m_bp_data = new bp_data_c(c_id, m_simBase); 

  // frontend stage
  m_frontend = fetch_factory_c::get()->allocate_frontend(FRONTEND_INTERFACE_ARGS(), m_simBase);
  
  // allocation stage
  if (m_core_type == "ptx" && *m_simBase->m_knobs->KNOB_GPU_SCHED) {
    m_allocate = NULL;
    m_gpu_allocate = new smc_allocate_c(m_core_id, m_q_frontend, m_gpu_q_iaq, m_uop_pool, 
        m_gpu_rob, m_unit_type, max_ALLOCQ, m_simBase);
  }
  else {
    m_allocate = new allocate_c(m_core_id, m_q_frontend, m_q_iaq, m_uop_pool, m_rob, 
        m_unit_type, max_ALLOCQ, m_simBase);
    m_gpu_allocate = NULL;
  }

  // execution stage
  m_exec = new exec_c (EXEC_INTERFACE_ARGS(), m_simBase);

  // instruction scheduler
  if (m_core_type == "ptx" && *m_simBase->m_knobs->KNOB_GPU_SCHED) {
    m_schedule = new schedule_smc_c (m_core_id, m_gpu_q_iaq, m_gpu_rob, m_exec, m_unit_type, 
        m_frontend, m_simBase);
  }
  else {
    if (m_knob_schedule == "ooo")
      m_schedule = new schedule_ooo_c (m_core_id, m_q_iaq, m_rob, m_exec, m_unit_type, 
          m_frontend, m_simBase);
    else if (m_knob_schedule == "io") 
      m_schedule = new schedule_io_c (m_core_id, m_q_iaq, m_rob, m_exec, m_unit_type, 
          m_frontend, m_simBase);
    else {   
      throw std::string() + "unrecognized schedule class " + m_knob_schedule;
    }   
  }

  // retirement stage
  m_retire = new retire_c (RETIRE_INTERFACE_ARGS(), m_simBase); 

  // hardware prefetcher
  if (*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON && m_knob_enable_pref) 
    m_hw_pref = new hwp_common_c(c_id, type, m_simBase);
  
  // const / texture cache
  if (m_core_type == "ptx" && *m_simBase->m_knobs->KNOB_USE_CONST_AND_TEX_CACHES) {
    m_const_cache = new readonly_cache_c("const_cache", m_core_id, 
        *KNOB(KNOB_CONST_CACHE_SIZE), *KNOB(KNOB_CONST_CACHE_ASSOC), 
        *KNOB(KNOB_CONST_CACHE_LINE_SIZE), *KNOB(KNOB_CONST_CACHE_BANKS), 
        *KNOB(KNOB_CONST_CACHE_CYCLES), 0, CACHE_CONST, sizeof(dcache_data_s), m_simBase);

    m_texture_cache = new readonly_cache_c("texture_cache", m_core_id, 
        *KNOB(KNOB_TEXTURE_CACHE_SIZE), *KNOB(KNOB_TEXTURE_CACHE_ASSOC), 
        *KNOB(KNOB_TEXTURE_CACHE_LINE_SIZE), *KNOB(KNOB_TEXTURE_CACHE_BANKS), 
        *KNOB(KNOB_TEXTURE_CACHE_CYCLES), 0, CACHE_TEXTURE, sizeof(dcache_data_s), m_simBase);
  }
  else {
    m_const_cache = NULL;
    m_texture_cache = NULL;
  }

  // shared memory
  if (m_core_type == "ptx") {
    m_shared_memory = new sw_managed_cache_c("shared_memory", m_core_id, 
        *KNOB(KNOB_SHARED_MEM_SIZE), *KNOB(KNOB_SHARED_MEM_ASSOC), 
        *KNOB(KNOB_SHARED_MEM_LINE_SIZE), *KNOB(KNOB_SHARED_MEM_BANKS), 
        *KNOB(KNOB_SHARED_MEM_CYCLES), 0, CACHE_SW_MANAGED, *KNOB(KNOB_SHARED_MEM_PORTS),
        *KNOB(KNOB_SHARED_MEM_PORTS), sizeof(dcache_data_s), m_simBase);
  }
  else {
    m_shared_memory = NULL;
  }

  // clock cycle
  m_cycle = 0;
}


// core_c destructor
core_c::~core_c()
{
  // deallocate memories
  delete m_q_frontend;
  delete m_frontend;
  delete m_uop_pool;
  if (m_core_type == "ptx" && *m_simBase->m_knobs->KNOB_GPU_SCHED) {
    delete m_gpu_rob;
    delete m_gpu_allocate;
    for (int i = 0; i < max_ALLOCQ; ++i) {
      delete m_gpu_q_iaq[i];
    }
    delete []m_gpu_q_iaq;
  }
  else {
    delete m_rob;
    delete m_allocate;
    for (int i = 0; i < max_ALLOCQ; ++i) {
      delete m_q_iaq[i];
    }
    delete [] m_q_iaq;
  }
  delete m_map;
  delete m_bp_data;
  delete m_exec;
  delete m_schedule;
  delete m_retire;
  delete m_icache;
}


// start core simulation 
void core_c::start(void)
{
  // start frontend
  m_frontend->start();


  // start backend
  if (m_allocate) 
    m_allocate->start();
  else 
    m_gpu_allocate->start();

  m_schedule->start();
  m_retire->start();
}


// stop core simulation 
void core_c::stop(void)
{
  // stop frontend
  m_frontend->stop();

  // stop backend
  if (m_allocate) 
    m_allocate->stop();
  else 
    m_gpu_allocate->stop();

  m_schedule->stop();
  m_retire->stop();
}


// main execution routine
// In every cycle, run all pipeline stages
void core_c::run_a_cycle(bool pll_lock)
{
  if (pll_lock) {
    ++m_cycle;
    return ;
  }

  start();

  // to simulate kernel invocation from host code
  if (*KNOB(KNOB_ENABLE_CONDITIONAL_EXECUTION)) {
    if (m_core_type == "ptx" && m_simBase->m_gpu_paused) {
      m_frontend->stop();
    }
  }


  // run each pipeline stages in backwards
  
  // execution stage
  m_exec->run_a_cycle();

  // retire stage
  m_retire->run_a_cycle();

  // prefetcher
  if (*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON && m_knob_enable_pref) 
    m_hw_pref->pref_update_queues();

  // scheduler
  m_schedule->run_a_cycle();

  // allocate stage
  if (m_allocate) 
    m_allocate->run_a_cycle();
  else 
    m_gpu_allocate->run_a_cycle();

  // frontend stage
  m_frontend->run_a_cycle();

  ++m_cycle;
}


// age entries in various queues 
void core_c::advance_queues(void)
{
  // advance frontend queue
  m_q_frontend->advance();

  // advance allocation queue
  if (m_core_type == "ptx" && *m_simBase->m_knobs->KNOB_GPU_SCHED) {
    for (int i = 0; i < max_ALLOCQ; ++i) {
      m_gpu_q_iaq[i]->advance();
    }
  }
  else {
    for (int i = 0; i < max_ALLOCQ; ++i) {
      m_q_iaq[i]->advance();
    }
  }
}


// last heartbeat call when a thread is terminated 
void core_c::final_heartbeat(int thread_id) 
{
  // schedule a new thread
  m_simBase->m_process_manager->sim_thread_schedule(false); 

  // check done
  m_heartbeat[thread_id]->m_check_done = true; 
  
  // thread final heart beat
  thread_heartbeat(thread_id, true);
}


// In every HEARTBEAT_INTERVAL cycles, print performance information
void core_c::check_heartbeat(bool final)
{
  core_heartbeat(final); 

  for (int ii = m_last_terminated_tid; ii < m_unique_scheduled_thread_num; ++ii) { 
    if (m_heartbeat.find(ii) == m_heartbeat.end() || m_heartbeat[ii]->m_check_done) 
      continue; 

    // print thread heartbeat
    thread_heartbeat(ii, final); 
  }
}


// thread heartbeat
void core_c::thread_heartbeat(int tid, bool final) 
{
  if (!*m_simBase->m_knobs->KNOB_PRINT_HEARTBEAT)
    return ;

  /* End Bookkeeping */
  m_inst_count = m_retire->get_instrs_retired(tid);
  Counter inst_diff = ((m_inst_count > m_heartbeat[tid]->m_printed_inst_count)? 
                        m_inst_count - m_heartbeat[tid]->m_printed_inst_count : 0);

  /* print heartbeat message if necessary */
  if ((*KNOB(KNOB_HEARTBEAT_INTERVAL) && inst_diff >= *KNOB(KNOB_HEARTBEAT_INTERVAL)) || final) {
    time_t cur_time = time(NULL);
    double int_ipc = (double)(m_inst_count - m_heartbeat[tid]->m_last_inst_count) / 
                             (m_core_cycle_count - m_heartbeat[tid]->m_last_cycle_count);
    double cum_ipc = (double)m_inst_count / m_core_cycle_count;

    double int_khz = (double)(m_inst_count - m_heartbeat[tid]->m_last_inst_count) / 
                             (cur_time - m_heartbeat[tid]->m_last_time) / 1000;

    double cum_khz = (double)m_inst_count / (cur_time - m_sim_start_time) / 1000;
    if (final) {
      fprintf(m_simBase->g_mystdout, "**Core %d Thread %d Finished:   insts:%-10llu  "
          "cycles:%-10llu (%llu)  seconds:%-5llu -- %.2f IPC (%.2f IPC) --  N/A  KHz (%.2f KHz) \n",
          m_core_id, tid, m_inst_count, m_core_cycle_count, m_simBase->m_simulation_cycle,
          (Counter)(cur_time - m_sim_start_time), cum_ipc, cum_ipc, cum_khz);
    } 
    else {
      fprintf(m_simBase->g_mystdout, "** Heartbeat for core[%d]:  Thread:%-6d insts:%-10llu  "
          "cycles:%-10llu (%llu) seconds:%-5llu -- %-3.2f IPC (%-3.2f IPC) -- %-3.2f KIPS (%-3.2f KIPS)\n",
          m_core_id, tid, m_inst_count / 100 * 100, m_core_cycle_count, m_simBase->m_simulation_cycle,
          (Counter)(cur_time - m_sim_start_time), int_ipc, cum_ipc, int_khz, cum_khz);
      fflush(m_simBase->g_mystdout);
    }

    // update thread heartbeat
    m_heartbeat[tid]->m_last_time           = cur_time;
    m_heartbeat[tid]->m_last_cycle_count    = m_core_cycle_count;
    m_heartbeat[tid]->m_last_inst_count     = m_inst_count;
    m_heartbeat[tid]->m_printed_inst_count += *m_simBase->m_knobs->KNOB_HEARTBEAT_INTERVAL;
  }
}


// core heartbeat
void core_c::core_heartbeat(bool final) 
{
  // get total retired instruction counts
  m_inst_count = m_retire->get_total_insts_retired();

  // instruction difference from last heartbeat print
  Counter inst_diff = ((m_inst_count > m_heartbeat_printed_inst_count_core)? 
                      m_inst_count - m_heartbeat_printed_inst_count_core : 0); 

  /* print heartbeat message if necessary */
  if ((*KNOB(KNOB_HEARTBEAT_INTERVAL) && inst_diff >= *KNOB(KNOB_HEARTBEAT_INTERVAL)) || final) { 
    time_t cur_time = time(NULL);
    double int_ipc = (double)(m_inst_count - m_heartbeat_last_inst_count_core) / 
                             (m_core_cycle_count - m_heartbeat_last_cycle_count_core); 
    double cum_ipc = (double)m_inst_count / m_core_cycle_count;
    double int_khz = (double)(m_inst_count - m_heartbeat_last_inst_count_core) / 
                             (cur_time - m_heartbeat_last_time_core) / 1000; 
    double cum_khz = (double)m_inst_count / (cur_time - m_sim_start_time) / 1000;

    if (final) {
      fprintf(m_simBase->g_mystdout, "**Core %d Core_Total  Finished:   insts:%-10llu  "
          "cycles:%-10llu (%llu) seconds:%-5llu -- %-3.2f IPC (%-3.2f IPC) --  N/A  KHz (%-3.2f KHz)\n",
          m_core_id, m_inst_count, m_core_cycle_count, m_simBase->m_simulation_cycle,
          (Counter)(cur_time - m_sim_start_time), cum_ipc, cum_ipc, cum_khz);
    } 
    else {
      fprintf(m_simBase->g_mystdout, "** Heartbeat for core[%d]:        insts:%-10llu  "
          "cycles:%-10llu (%llu) seconds:%-5llu -- %-3.2f IPC (%-3.2f IPC) -- %-3.2f KIPS (%-3.2f KIPS)\n",
          m_core_id, m_inst_count / 100 * 100, m_core_cycle_count, m_simBase->m_simulation_cycle,
          (Counter)(cur_time - m_sim_start_time), int_ipc, cum_ipc, int_khz, cum_khz);
      fflush(m_simBase->g_mystdout);
    }

    // update heartbeat
    m_heartbeat_last_time_core           = cur_time;
    m_heartbeat_last_cycle_count_core    = m_core_cycle_count;
    m_heartbeat_last_inst_count_core     = m_inst_count;
    m_heartbeat_printed_inst_count_core += *m_simBase->m_knobs->KNOB_HEARTBEAT_INTERVAL;
  }
}


// check forward progress of the simulation 
void core_c::check_forward_progress()
{
  // get total retired instruction count
  Counter inst_count = m_retire->get_total_insts_retired();

  // update whenever new instruction is retired
  if (inst_count > m_last_inst_count) { 
    m_last_forward_progress = m_core_cycle_count;
    m_last_inst_count       = inst_count;
  }

  // if no instruction has been retired more than KNOB_FORWARD_PROGRESS_LIMIT cycle,
  // raise forward progress exception
  if (m_core_cycle_count - m_last_forward_progress > *KNOB(KNOB_FORWARD_PROGRESS_LIMIT)) {
    STAT_EVENT(PROGRESS_ERROR);

    // print all mshr entries
    m_simBase->m_memory->print_mshr();

    // print all dram requests
    for (int ii = 0; ii < *KNOB(KNOB_DRAM_NUM_MC); ++ii) {
      DRAM_CTRL[ii]->print_req();
    }

    // print all remaining uop states
    if (*KNOB(KNOB_BUG_DETECTOR_ENABLE)) {
      m_simBase->m_bug_detector->print(m_core_id, m_last_terminated_tid);
      m_simBase->m_bug_detector->print_fence_info();
    }

    m_simBase->m_network->print();

    ASSERTM(m_core_cycle_count - m_last_forward_progress <= *KNOB(KNOB_FORWARD_PROGRESS_LIMIT),
        "core_id:%d core_cycle_count:%llu (%llu) last_forward_progress:%llu last_tid:%d "
        "last_inst_count:%llu\n", m_core_id, m_core_cycle_count, m_simBase->m_simulation_cycle,
        m_last_forward_progress, m_last_terminated_tid, m_last_inst_count);
  }
}


///////////////////////////////////////////////////////////////////////////////////////////////

// In GPU simulation: each thread (or warp) requires its own data structures. We allocate and
// deallocate those data on demand. Thus, we need to allocate thread-specific data when a 
// new thread has been launched and deallocate when it is terminated.

// Mostly, these data are maintained using hash structure. 'Allocation' adds new entry in the
// hash table and 'Deallocation' deletes corresponding entry from the hash table.


// allocate thread specific data
void core_c::allocate_thread_data(int tid)
{
  // add thread data
  m_fetch_ended[tid]          = false;
  m_thread_finished[tid]      = false;
  m_thread_reach_end[tid]     = false;
  m_inst_fetched[tid]         = 0;
  m_last_fetch_cycle[tid]     = 0;
  m_ops_to_be_dispatched[tid] = 0;

  // allocate heartbeat
  heartbeat_s* heartbeat = m_simBase->m_heartbeat_pool->acquire_entry();
  heartbeat->m_check_done              = false;
  heartbeat->m_last_time               = 0;
  heartbeat->m_last_cycle_count        = 0;
  heartbeat->m_last_inst_count         = 0;
  heartbeat->m_printed_inst_count      = 0;
  m_heartbeat[tid]                    = heartbeat; 

  // allocate bp recovery
  bp_recovery_info_c* bp_recovery_info = m_simBase->m_bp_recovery_info_pool->acquire_entry();
  bp_recovery_info->m_recovery_cycle   = MAX_CTR;
  bp_recovery_info->m_redirect_cycle   = MAX_CTR;
  m_bp_recovery_info[tid]              = bp_recovery_info;
  m_bp_data->m_bp_recovery_cycle[tid]  = 0;
  m_bp_data->m_bp_redirect_cycle[tid]  = 0; 
  m_bp_data->m_bp_cause_op[tid]        = 0;

  // allocate retire data
  m_retire->allocate_retire_data(tid);

  // allocate scheduler queue and rob for GPU simulation
  if (m_core_type == "ptx" && *m_simBase->m_knobs->KNOB_GPU_SCHED) 
    m_gpu_rob->reserve_rob(tid);
}


// When a thread is terminated, deallocate all data used by this thread
void core_c::deallocate_thread_data(int tid)
{
  if (tid != 0) {
    // deallocate heartbeat info
    heartbeat_s* heartbeat = m_heartbeat[tid];
    m_heartbeat.erase(tid);
    m_simBase->m_heartbeat_pool->release_entry(heartbeat);

    // deallocate bp recovery info
    bp_recovery_info_c* bp_recovery_info = m_bp_recovery_info[tid];
    m_bp_recovery_info.erase(tid);
    m_simBase->m_bp_recovery_info_pool->release_entry(bp_recovery_info);
  }

  // deallocate dependence map
  m_map->delete_map(tid);

  // mark current thread as terminated
  m_terminated_tid[tid] = true;

  // update last terminated thread id : all threads before last_terminted_tid are terminated.
  // Since threads are terminated not in ascending order, in order to get correct
  // last_terminated_tid, we need to hold all previous terminated thread ids.
  int t_id = m_last_terminated_tid;
  while (m_terminated_tid.find(t_id) != m_terminated_tid.end()) {
    m_terminated_tid.erase(t_id);
    m_last_terminated_tid = ++t_id;
  }

  if (m_core_type == "ptx" && *m_simBase->m_knobs->KNOB_GPU_SCHED) 
    m_gpu_rob->free_rob(tid);

  // check forward progress
  if (m_unique_scheduled_thread_num >= m_last_terminated_tid + 1000) {
    DEBUG("core_id=%d thread_id:%d last_terminated_tid:%d\n",
        m_core_id, tid, m_last_terminated_tid);
    ASSERTM(0, "core_id=%d thread_id:%d last_terminated_tid:%d\n",
        m_core_id, tid, m_last_terminated_tid);
  }
}


///////////////////////////////////////////////////////////////////////////////////////////////

// hardware prefetchers


// train hardware prefetchers based on the event
void core_c::train_hw_pref(int level, int tid, Addr addr, Addr pc, uop_c* uop, bool hit)
{
  if (*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON && m_knob_enable_pref) {
    m_hw_pref->train(level, tid, addr, pc, uop, hit);
  }
}


// hardware prefetcher initialization
void core_c::pref_init(void) 
{
  if (*m_simBase->m_knobs->KNOB_PREF_FRAMEWORK_ON && m_knob_enable_pref) {
    m_hw_pref->pref_init(m_core_type == "ptx" ? true : false);
  }
}


///////////////////////////////////////////////////////////////////////////////////////////////

// When we need to differentiate uop/memory request/... with application id, we need to get
// the application id from the core.


// add a new application to the core
void core_c::add_application(int tid, process_s *process)
{
  m_tid_to_appl_map[tid] = process;
  m_appl_id              = process->m_orig_pid;
}


// delete the application from the core
void core_c::delete_application(int appl_id)
{
  // since many threads may be mapped to same application id
  // we need to delete all entries from the map
  for (auto I = m_tid_to_appl_map.begin(), E  = m_tid_to_appl_map.end(); I != E;) {
    auto current = I++;
    if ((*current).second->m_orig_pid == appl_id) {
      m_tid_to_appl_map.erase(current);
    }
  }

  m_appl_id = 0;
}


// get the application id from the thread id
int core_c::get_appl_id(int tid)
{
  if (m_tid_to_appl_map.find(tid) == m_tid_to_appl_map.end()) 
    return 0;
  else 
    return m_tid_to_appl_map[tid]->m_orig_pid;
}


// get the current application id
int core_c::get_appl_id()
{
  return m_appl_id;
}


///////////////////////////////////////////////////////////////////////////////////////////////


// get the thread trace information
thread_s* core_c::get_trace_info(int tid)
{
  if (m_thread_trace_info.find(tid) == m_thread_trace_info.end())
    return NULL;

  return m_thread_trace_info[tid];
}


// create a new thread trace information
void core_c::create_trace_info(int tid, thread_s* thread)
{
  m_thread_trace_info[tid] = thread;

  allocate_thread_data(tid);
  ++m_unique_scheduled_thread_num;
  ++m_running_thread_num;
  ++m_fetching_thread_num;

  // to prevent from unnecessary forward progress error for a newely launched cores 
  m_last_forward_progress = m_core_cycle_count; 
}


///////////////////////////////////////////////////////////////////////////////////////////////



