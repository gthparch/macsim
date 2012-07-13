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
 * File         : schedule_io.cc
 * Author       : Hyesoon Kim
 * Date         : 1/1/2008 
 * SVN          : $Id: main.cc,v 1.26 2008-09-21 00:02:54 kacear Exp $:
 * Description  : scheduler for gpu (small many cores)
 *********************************************************************************************/


#include "allocate_smc.h"
#include "schedule_smc.h"
#include "pqueue.h"
#include "exec.h"
#include "core.h"
#include "memory.h"
#include "rob_smc.h"

#include "knob.h"
#include "debug_macros.h"
#include "statistics.h"

#include "all_knobs.h"

#define DEBUG(args...)   _DEBUG(*KNOB(KNOB_DEBUG_SCHEDULE_STAGE), ## args) 


///////////////////////////////////////////////////////////////////////////////////////////////
/// \page page1 GPU instruction scheduler
/// How this scheduler is implemented: \n
/// \section asdf
/// The instruction window will hold instructions from multiple threads. the 
/// scheduler can switch from one thread to another in any order (depending 
/// on the policy), but instructions within a thread must be scheduled inorder.
/// for these reasons, the scheduler is implemented as a list of scheduler 
/// queues. each thread is assigned a queue when the thread (block) is assigned 
/// to the core. when the thread terminates, the queue is freed.
///////////////////////////////////////////////////////////////////////////////////////////////


// schedule_smc_c constructor
schedule_smc_c::schedule_smc_c(int core_id, pqueue_c<gpu_allocq_entry_s>** gpu_allocq,
    smc_rob_c* gpu_rob, exec_c* exec, Unit_Type unit_type, frontend_c* frontend, 
    macsim_c* simBase) :  
    schedule_c(exec, core_id, unit_type, frontend, NULL, simBase),
    m_gpu_rob(gpu_rob), m_gpu_allocq(gpu_allocq)
{

  m_simBase = simBase;

  // configuration
  switch (m_unit_type) {
    case UNIT_SMALL:
      knob_num_threads      = *KNOB(KNOB_MAX_THREADS_PER_CORE);
      break;
    case UNIT_MEDIUM:
      knob_num_threads      = *KNOB(KNOB_MAX_THREADS_PER_MEDIUM_CORE);
      break;
    case UNIT_LARGE:
      knob_num_threads      = *KNOB(KNOB_MAX_THREADS_PER_LARGE_CORE);
      break;
  }

  m_schedule_modulo = (*KNOB(KNOB_GPU_SCHEDULE_RATIO - 1)); 
  m_schlist_size    = MAX_GPU_SCHED_SIZE * knob_num_threads;
  m_schlist_entry   = new int[m_schlist_size];
  m_schlist_tid     = new int[m_schlist_size];
  m_first_schlist   = 0;
  m_last_schlist    = 0;
}


// schedule_smc_c destructor
schedule_smc_c::~schedule_smc_c(void) 
{
  delete[] m_schlist_entry;
  delete[] m_schlist_tid;
}


// move uops from alloc queue to schedule queue
void schedule_smc_c::advance(int q_index) {
  fill_n(m_count, static_cast<size_t>(max_ALLOCQ), 0);

  while (m_gpu_allocq[q_index]->ready()) {
    // this prevents scheduler overwritten
    if ((m_last_schlist + 1) % m_schlist_size == m_first_schlist)
      break ;


    gpu_allocq_entry_s  allocq_entry = m_gpu_allocq[q_index]->peek(0); 

    int tid        = allocq_entry.m_thread_id; 
    rob_c *m_rob   = m_gpu_rob->get_thread_rob(tid);
    uop_c *cur_uop = (uop_c *) (*m_rob)[allocq_entry.m_rob_entry];
    
    POWER_CORE_EVENT(m_core_id, POWER_INST_QUEUE_R);
    POWER_CORE_EVENT(m_core_id, POWER_UOP_QUEUE_R);

    switch (cur_uop->m_uop_type){
      case UOP_FMEM:
      case UOP_FCF:
      case UOP_FCVT:
      case UOP_FADD:    
      case UOP_FMUL:    
      case UOP_FDIV:    
      case UOP_FCMP:    
      case UOP_FBIT:    
      case UOP_FCMOV:
        STAT_CORE_EVENT(m_core_id, POWER_FP_RENAME_R);
        break;
      default:
        break;
    } 

    ALLOCQ_Type allocq = (*m_rob)[allocq_entry.m_rob_entry]->m_allocq_num;
    if ((m_count[allocq] >= m_sched_rate[allocq]) ||
        (m_num_per_sched[allocq] >= m_sched_size[allocq])) {
      break;
    }


    // dequeue the element from the alloc queue
    m_gpu_allocq[q_index]->dequeue();


    // if the entry has been flushed
    if (cur_uop->m_bogus || (cur_uop->m_done_cycle) ) {
      cur_uop->m_done_cycle = (m_simBase->m_core_cycle[m_core_id]);
      continue;
    }


    // update the element m_count for corresponding scheduled queue
    m_count[allocq]         = m_count[allocq]+1;
    cur_uop->m_in_iaq       = false;
    cur_uop->m_in_scheduler = true;

    //    int queue = get_reserved_sched_queue(allocq_entry.m_thread_id);
    int entry = allocq_entry.m_rob_entry;

    m_schlist_entry[m_last_schlist] = entry;
    m_schlist_tid[m_last_schlist++] = tid;
    m_last_schlist %= m_schlist_size;
    ++m_num_in_sched;
    ++m_num_per_sched[allocq];
    assert(m_last_schlist != m_first_schlist);
    
    DEBUG("core_id:%d thread_id:%d uop_num:%lld inserted into scheduler\n",
        m_core_id, cur_uop->m_thread_id, cur_uop->m_uop_num);
   	
    POWER_CORE_EVENT(m_core_id, POWER_RESERVATION_STATION_W);
  }
}


// check source registers are ready
bool schedule_smc_c::check_srcs_smc(int thread_id, int entry)
{
  bool ready = true;
  uop_c *cur_uop = NULL;
  rob_c *thread_m_rob = m_gpu_rob->get_thread_rob(thread_id);
  cur_uop = (*thread_m_rob)[entry];
  

  // check if all sources are already ready
  if (cur_uop->m_srcs_rdy) {
    return true;  
  }

  for (int i = 0; i < cur_uop->m_num_srcs; ++i) {
    if (cur_uop->m_map_src_info[i].m_uop == NULL) {
      continue;
    }


    // Extract the source uop info
    uop_c* src_uop = cur_uop->m_map_src_info[i].m_uop;
    uns src_uop_num = cur_uop->m_map_src_info[i].m_uop_num;
    

    // Check if source uop is valid
    if (!src_uop || 
        !src_uop->m_valid || 
        (src_uop->m_uop_num != src_uop_num) ||
        (src_uop->m_thread_id != cur_uop->m_thread_id))  {
      continue;
    }

    DEBUG("core_cycle_m_count:%lld core_id:%d thread_id:%d uop_num:%lld "
        "src_uop_num:%u src_uop->uop_num:%lld src_uop->done_cycle:%lld "
        "src_uop->uop_num:%s  src_uop_num:%s \n", m_cur_core_cycle, m_core_id,
        cur_uop->m_thread_id, cur_uop->m_uop_num, src_uop_num, src_uop->m_uop_num, 
        src_uop->m_done_cycle, unsstr64(src_uop->m_uop_num), unsstr64(src_uop_num));

    // Check if the source uop is ready
    if ((src_uop->m_done_cycle == 0) || 
        (m_simBase->m_core_cycle[m_core_id] < src_uop->m_done_cycle))  {
      // Source is not ready. 
      // Hence we update the last_dep_exec field of this uop and return. 
      if (!cur_uop->m_last_dep_exec || 
          (*(cur_uop->m_last_dep_exec) < src_uop->m_done_cycle)) {
        DEBUG("*cur_uop->last_dep_exec:%lld src_uop->uop_num:%lld src_uop->done_cycle:%lld \n",
              cur_uop->m_last_dep_exec ? *(cur_uop->m_last_dep_exec): 0, 
              src_uop?src_uop->m_uop_num: 0, src_uop? src_uop->m_done_cycle: 1);

        cur_uop->m_last_dep_exec = &(src_uop->m_done_cycle);
      }
      ready = false;
      return ready;
    }
  }


  //The uop is ready since we didnt find any source uop that was not ready
  cur_uop->m_srcs_rdy = ready;

  return ready;
}


// schedule an uop from reorder buffer
// called by schedule_io_c::run_a_cycle
// call exec_c::exec function for uop execution
bool schedule_smc_c::uop_schedule_smc(int thread_id, int entry, SCHED_FAIL_TYPE* sched_fail_reason)
{
  uop_c *cur_uop = NULL;
  rob_c *thread_m_rob = m_gpu_rob->get_thread_rob(thread_id);

  cur_uop     = (*thread_m_rob)[entry];
  int q_num  = cur_uop->m_allocq_num;
  bool bogus = cur_uop->m_bogus;

  *sched_fail_reason = SCHED_SUCCESS;
  
    
  DEBUG("uop_schedule core_id:%d thread_id:%d uop_num:%lld inst_num:%lld "
      "uop.va:%s allocq:%d mem_type:%d last_dep_exec:%llu done_cycle:%llu\n",
      m_core_id, cur_uop->m_thread_id, cur_uop->m_uop_num, cur_uop->m_inst_num, 
      hexstr64s(cur_uop->m_vaddr), cur_uop->m_allocq_num, cur_uop->m_mem_type, 
      (cur_uop->m_last_dep_exec? *(cur_uop->m_last_dep_exec) : 0), cur_uop->m_done_cycle); 


  // Return if sources are not ready 
  if (!bogus && !(cur_uop->m_srcs_rdy) && cur_uop->m_last_dep_exec &&
		  (m_cur_core_cycle < *(cur_uop->m_last_dep_exec))) {
    *sched_fail_reason = SCHED_FAIL_OPERANDS_NOT_READY;
    return false;
  }


  if (!bogus) {
    // source registers are not ready
    if (!check_srcs_smc(thread_id, entry)) {
      *sched_fail_reason = SCHED_FAIL_OPERANDS_NOT_READY;
      DEBUG("core_id:%d thread_id:%d uop_num:%lld operands are not ready \n", 
            m_core_id, cur_uop->m_thread_id, cur_uop->m_uop_num); 

      return false;
    }   
  

    // Check for port availability.
    if (!m_exec->port_available(q_num)) {
      *sched_fail_reason = SCHED_FAIL_NO_AVAILABLE_PORTS;
      DEBUG("core_id:%d thread_id:%d uop_num:%lld ports are not ready \n", 
          m_core_id, cur_uop->m_thread_id, cur_uop->m_uop_num); 
      return false;
    }


    // check available mshr spaces for scheduling
    core_c *core = m_simBase->m_core_pointers[m_core_id];
    if ("ptx" == core->get_core_type() && 
        cur_uop->m_mem_type != NOT_MEM && 
        cur_uop->m_num_child_uops > 0) {
      // constant or texture memory access
      if (cur_uop->m_mem_type == MEM_LD_CM || cur_uop->m_mem_type == MEM_LD_TM) {
        if (!m_simBase->m_memory->get_num_avail_entry(m_core_id)) {
          *sched_fail_reason = SCHED_FAIL_NO_MEM_REQ_SLOTS;
          return false;
        }
      }
    }
  }

  cur_uop->m_state = OS_SCHEDULE;
  POWER_CORE_EVENT(m_core_id, POWER_RESERVATION_STATION_R);


  // -------------------------------------
  // execute current uop
  // -------------------------------------
  if (!m_exec->exec(thread_id, entry, cur_uop)) {
    // uop could not execute
    DEBUG("core_id:%d thread_id:%d uop_num:%lld just cannot be executed\n", 
          m_core_id, cur_uop->m_thread_id, cur_uop->m_uop_num); 

    return false;
  }


  // Generate Stat events
  STAT_EVENT(DISPATCHED_INST);
  STAT_EVENT_N(DISPATCH_WAIT, m_cur_core_cycle - cur_uop->m_alloc_cycle);
  STAT_CORE_EVENT(m_core_id, CORE_DISPATCHED_INST);
  STAT_CORE_EVENT_N(m_core_id, CORE_DISPATCH_WAIT, 
      m_cur_core_cycle - cur_uop->m_alloc_cycle);

  POWER_CORE_EVENT(m_core_id, POWER_RESERVATION_STATION_R_TAG);
  POWER_CORE_EVENT(m_core_id, POWER_INST_ISSUE_SEL_LOGIC_R);
  POWER_CORE_EVENT(m_core_id, POWER_PAYLOAD_RAM_R);

  // Decrement dispatch m_count for the current thread
  --m_simBase->m_core_pointers[m_core_id]->m_ops_to_be_dispatched[cur_uop->m_thread_id];


  // Uop m_exec ok; update scheduler
  cur_uop->m_in_scheduler = false;
  --m_num_in_sched;
  POWER_CORE_EVENT(m_core_id, POWER_INST_ISSUE_SEL_LOGIC_W);
  POWER_CORE_EVENT(m_core_id, POWER_PAYLOAD_RAM_W);


  switch (q_num) {
    case gen_ALLOCQ : 
      --m_num_per_sched[gen_ALLOCQ]; 
      break;
    case mem_ALLOCQ : 
      --m_num_per_sched[mem_ALLOCQ];  
      break;
    case fp_ALLOCQ : 
      --m_num_per_sched[fp_ALLOCQ]; 
      break;
    default:
     printf("unknown queue\n");
    exit(EXIT_FAILURE);
  }
  
  DEBUG("done schedule core_id:%d thread_id:%d uop_num:%lld inst_num:%lld "
      "entry:%d queue:%d m_num_in_sched:%d m_num_per_sched[general]:%d "
      "m_num_per_sched[mem]:%d m_num_per_sched[fp]:%d done_cycle:%lld\n",
      m_core_id, cur_uop->m_thread_id, cur_uop->m_uop_num,
      cur_uop->m_inst_num, entry, cur_uop->m_allocq_num, m_num_in_sched,
      m_num_per_sched[gen_ALLOCQ], m_num_per_sched[mem_ALLOCQ],
      m_num_per_sched[fp_ALLOCQ], cur_uop->m_done_cycle); 

  return true;
}


// main execution routine
// In every cycle, schedule uops from rob
void schedule_smc_c::run_a_cycle(void) 
{
  // check if the scheduler is running
  if (!is_running()) {
    return;
  }

  m_cur_core_cycle = m_simBase->m_core_cycle[m_core_id];

  // GPU : schedule every N cycles (G80:4, Fermi:2)
  m_schedule_modulo = (m_schedule_modulo + 1) % *KNOB(KNOB_GPU_SCHEDULE_RATIO);
  if (m_schedule_modulo) 
    return;

  // clear execution port
  m_exec->clear_ports(); 


  // GPU : recent GPUs have dual warp schedulers. In each schedule cycle, each warp scheduler
  // can schedule instructions from different threads. We enforce threads selected by
  // each warp scheduler should be different. 
  int count = 0;
  for (int ii = m_first_schlist; ii != m_last_schlist; ii = (ii + 1) % m_schlist_size) { 
    // -------------------------------------
    // Schedule stops when
    // 1) no uops in the scheduler (m_num_in_sched and first == last)
    // 2) # warp scheduler
    // 3) FIXME add width condition
    // -------------------------------------
    if (!m_num_in_sched || 
        m_first_schlist == m_last_schlist || 
        count == *KNOB(KNOB_NUM_WARP_SCHEDULER)) 
      break;

    SCHED_FAIL_TYPE sched_fail_reason;

    int thread_id = m_schlist_tid[ii];
    int entry     = m_schlist_entry[ii];

    bool uop_scheduled = false;

    if (entry != -1) {
      // schedule a uop from a thread
      if (uop_schedule_smc(thread_id, entry, &sched_fail_reason)) {
        STAT_CORE_EVENT(m_core_id, SCHED_FAILED_REASON_SUCCESS);

        m_schlist_entry[ii] = -1;
        m_schlist_tid[ii] = -1;
        if (ii == m_first_schlist) {
          m_first_schlist = (m_first_schlist + 1) % m_schlist_size;
        }

        uop_scheduled = true;
        ++count;
      }
      else {
        STAT_CORE_EVENT(m_core_id, 
            SCHED_FAILED_REASON_SUCCESS + MIN2(sched_fail_reason, 2));
      }
    }
    else if (ii == m_first_schlist) {
      m_first_schlist = (m_first_schlist + 1) % m_schlist_size;
    }
  }


  // no uop is scheduled in this cycle
  if (count == 0) {
    STAT_CORE_EVENT(m_core_id, NUM_NO_SCHED_CYCLE);
    STAT_EVENT(AVG_CORE_IDLE_CYCLE);
  }


  // advance entries from alloc queue to schedule queue 
  for (int ii = 0; ii < max_ALLOCQ; ++ii) {
    advance(ii);
  }
}



