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
 * File         : schedule.cc
 * Author       : Hyesoon Kim
 * Date         : 1/1/2008 
 * SVN          : $Id: main.cc,v 1.26 2008-09-21 00:02:54 kacear Exp $:
 * Description  : schedule instructions
 *********************************************************************************************/


#include "schedule.h"
#include "uop.h"
#include "rob.h"
#include "exec.h"
#include "core.h"
#include "statistics.h"

#include "config.h"

#include "debug_macros.h"

#include "all_knobs.h"


#define DEBUG(args...)   _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_SCHEDULE_STAGE, ## args) 


// schedule_c constructor
schedule_c::schedule_c(exec_c* exec, int core_id, Unit_Type unit_type, frontend_c* frontend, 
    pqueue_c<int>** alloc_q, macsim_c* simBase)
{
  m_simBase           = simBase;
  m_alloc_q           = alloc_q;
  m_exec              = exec;
  m_core_id           = core_id;
  m_unit_type         = unit_type;
  m_frontend          = frontend;
  m_schedule_running  = false;
  m_num_in_sched      = 0;
  m_first_schlist_ptr = 0;
  m_last_schlist_ptr  = 0;

  m_sched_size    = new int [max_ALLOCQ];
  m_sched_rate    = new int [max_ALLOCQ];
  m_num_per_sched = new int [max_ALLOCQ];

  SCHED_CONFIG();

  std::fill_n(m_schedule_list, MAX_SCHED_SIZE, - 1); 
}


// schedule_c destructor
schedule_c::~schedule_c()
{
  delete [] m_sched_size;
  delete [] m_sched_rate;
  delete [] m_num_per_sched;
}


schedule_c::schedule_c()
{
}


// enable scheduler
void schedule_c::start()
{
  m_schedule_running = true;
}


// disable scheduler
void schedule_c::stop()
{
  m_schedule_running = false;
}


// check scheduler is running
bool schedule_c::is_running()
{
  return m_schedule_running;
}


// check source registers are ready
bool schedule_c::check_srcs(int entry)
{
  bool   ready   = true;
  uop_c *cur_uop = NULL;
  cur_uop        = (*m_rob)[entry];


  // check if all sources are already ready
  if (cur_uop->m_srcs_rdy) {
    return true;
  }


  // search all source (dependent) uops
  for (int i = 0; i < cur_uop->m_num_srcs; ++i) {

    if (cur_uop->m_map_src_info[i].m_uop == NULL) {
      continue;
    }

    // Extract the source uop info
    uop_c* src_uop      = cur_uop->m_map_src_info[i].m_uop;
    Counter src_uop_num = cur_uop->m_map_src_info[i].m_uop_num;

    // Check if source uop is valid
    if (!src_uop || 
        !src_uop->m_valid ||
        (src_uop->m_uop_num != src_uop_num) ||
        (src_uop->m_thread_id != cur_uop->m_thread_id))  {
      continue;
    }

    DEBUG("core_cycle_m_count:%lld m_core_id:%d thread_id:%d uop_num:%lld "
          "src_uop_num:%lld src_uop->uop_num:%lld src_uop->done_cycle:%lld "
          "src_uop->uop_num:%s  src_uop_num:%s \n", m_cur_core_cycle,
          m_core_id, cur_uop->m_thread_id, cur_uop->m_uop_num, src_uop_num, src_uop->m_uop_num, 
          src_uop->m_done_cycle, unsstr64(src_uop->m_uop_num), unsstr64(src_uop_num));


    // Check if the source uop is ready
    if ((src_uop->m_done_cycle  == 0) || 
        (m_simBase->m_core_cycle[m_core_id] < src_uop->m_done_cycle)) {
      // Source is not ready. 
      // Hence we update the last_dep_exec field of this uop and return. 
      if (!cur_uop->m_last_dep_exec || (*(cur_uop->m_last_dep_exec) < src_uop->m_done_cycle)) {
        cur_uop->m_last_dep_exec = &(src_uop->m_done_cycle);
      }
      DEBUG("*cur_uop->last_dep_exec:%lld src_uop->uop_num:%lld src_uop->done_cycle:%lld \n", 
          cur_uop->m_last_dep_exec ? *(cur_uop->m_last_dep_exec): 0, 
          src_uop? src_uop->m_uop_num: 0, src_uop? src_uop->m_done_cycle: 1);

      ready = false;
      return ready;
    }
  }

  // The uop is ready since we didnt find any source uop that was not ready
  cur_uop->m_srcs_rdy = ready;

  return ready;
}


// schedule an uop from reorder buffer
// called by schedule_io_c::run_a_cycle
// call exec_c::exec function for uop execution
bool schedule_c::uop_schedule(int entry, SCHED_FAIL_TYPE* sched_fail_reason)
{
  uop_c *cur_uop     = (*m_rob)[entry];
  int    q_num       = cur_uop->m_allocq_num;
  bool   bogus       = cur_uop->m_bogus;
  *sched_fail_reason = SCHED_SUCCESS;


  DEBUG("cycle_m_count:%llu m_core_id:%d thread_id:%d uop_num:%lld inst_num:%lld uop.va:%s "
      "allocq:%d mem_type:%d last_dep_exec:%llu done_cycle:%llu\n",
      m_cur_core_cycle, m_core_id, cur_uop->m_thread_id, cur_uop->m_uop_num,
      cur_uop->m_inst_num, hexstr64s(cur_uop->m_vaddr), cur_uop->m_allocq_num,
      cur_uop->m_mem_type, (cur_uop->m_last_dep_exec? *(cur_uop->m_last_dep_exec) : 0),
      cur_uop->m_done_cycle);


  // Return if sources are not ready 
  if (!bogus && 
      !(cur_uop->m_srcs_rdy) && 
      cur_uop->m_last_dep_exec &&
      (m_cur_core_cycle < *(cur_uop->m_last_dep_exec))) {
    *sched_fail_reason = SCHED_FAIL_OPERANDS_NOT_READY;
    return false;
  }


  if (!bogus) {
    // check whether source registers are ready
    if (!check_srcs(entry)) {
      *sched_fail_reason = SCHED_FAIL_OPERANDS_NOT_READY;
      DEBUG("m_core_id:%d thread_id:%d uop_num:%lld operands are not ready \n", 
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
  }
  
  cur_uop->m_state = OS_SCHEDULE;
  POWER_CORE_EVENT(m_core_id, POWER_RESERVATION_STATION_R);
  POWER_CORE_EVENT(m_core_id, POWER_RESERVATION_STATION_R_TAG);
  POWER_CORE_EVENT(m_core_id, POWER_INST_ISSUE_SEL_LOGIC_R);
  POWER_CORE_EVENT(m_core_id, POWER_PAYLOAD_RAM_R);


  // -------------------------------------
  // execute current uop
  // -------------------------------------
  if (!m_exec->exec(-1, entry, cur_uop)) {
    // uop could not m_execute
    DEBUG("m_core_id:%d thread_id:%d uop_num:%lld just cannot be m_executed\n", 
        m_core_id, cur_uop->m_thread_id, cur_uop->m_uop_num); 

    return false;
  }


  // Generate Stat events
  STAT_EVENT(DISPATCHED_INST);
  STAT_EVENT_N(DISPATCH_WAIT, m_cur_core_cycle - cur_uop->m_alloc_cycle);
  STAT_CORE_EVENT(m_core_id, CORE_DISPATCHED_INST);
  STAT_CORE_EVENT_N(m_core_id, CORE_DISPATCH_WAIT, 
      m_cur_core_cycle - cur_uop->m_alloc_cycle);

  // Decrement dispatch m_count for the current thread
  --m_simBase->m_core_pointers[m_core_id]->m_ops_to_be_dispatched[cur_uop->m_thread_id];

  // Uop exec ok; update scheduler
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
     printf("unknown allocq\n");
      exit(EXIT_FAILURE);
  }

  DEBUG("cycle_m_count:%lld m_core_id:%d thread_id:%d uop_num:%lld inst_num:%lld entry:%d "
      "allocq:%d m_num_in_sched:%d m_num_per_sched[general]:%d m_num_per_sched[mem]:%d "
      "m_num_per_sched[fp]:%d done_cycle:%lld\n",
      m_cur_core_cycle, m_core_id, cur_uop->m_thread_id, cur_uop->m_uop_num,
      cur_uop->m_inst_num, entry, cur_uop->m_allocq_num, m_num_in_sched,
      m_num_per_sched[gen_ALLOCQ], m_num_per_sched[mem_ALLOCQ], m_num_per_sched[fp_ALLOCQ], 
      cur_uop->m_done_cycle);

  return true;
}


// move uops from alloc queue to schedule queue
void schedule_c::advance(int q_index)
{
  // Initialize the m_count array with zeros
  fill_n(m_count, static_cast<std::size_t>(max_ALLOCQ), 0);

  // Iterate until alloc queue has ready members
  while (m_alloc_q[q_index]->ready()) {
    // this prevents scheduler overwritten
    if ((m_last_schlist_ptr + 1) % MAX_SCHED_SIZE == m_first_schlist_ptr)
      break ;

    int entry      = (int) m_alloc_q[q_index]->peek(0);  
    uop_c *cur_uop = (uop_c *)(*m_rob)[entry]; 

    POWER_CORE_EVENT(m_core_id, POWER_INST_QUEUE_R);
    POWER_CORE_EVENT(m_core_id, POWER_UOP_QUEUE_R);
    POWER_CORE_EVENT(m_core_id, POWER_REG_RENAMING_TABLE_R);
    POWER_CORE_EVENT(m_core_id, POWER_FREELIST_R);
    switch (cur_uop->m_uop_type) {
      case UOP_FMEM:
      case UOP_FCF:
      case UOP_FCVT:
      case UOP_FADD:                   
      case UOP_FMUL:                    
      case UOP_FDIV:                   
      case UOP_FCMP:                  
      case UOP_FBIT:                   
      case UOP_FCMOV:
        POWER_CORE_EVENT(m_core_id, POWER_FP_RENAME_R);
        break;
      default:
        break;
    }

    DEBUG("cycle_m_count:%lld entry:%d m_core_id:%d thread_id:%d uop_num:%lld "
        "inst_num:%lld uop.va:%s allocq:%d mem_type:%d \n", m_cur_core_cycle,
        entry, m_core_id, cur_uop->m_thread_id, cur_uop->m_uop_num, cur_uop->m_inst_num, 
        hexstr64s(cur_uop->m_vaddr), cur_uop->m_allocq_num, cur_uop->m_mem_type);

    // Take out the corresponding entries queue type 
    // and check if the sched queue of the corresponding type has space. 
    // If not then break out.
    ALLOCQ_Type q_type = (*m_rob)[entry]->m_allocq_num;
    if ((m_count[q_type] >= m_sched_rate[q_type]) ||
        (m_num_per_sched[q_type] >= m_sched_size[q_type])) {
      break;
    }

    // -------------------------------------
    // Dequeue the element from the alloc queue
    // -------------------------------------
    m_alloc_q[q_index]->dequeue();

    // Check if the entry has been flushed. If so just move ahead.
    if (cur_uop->m_bogus || (cur_uop->m_done_cycle)) {
      cur_uop->m_done_cycle = (m_simBase->m_core_cycle[m_core_id]);
      continue;
    }

    // Update the element m_count for corresponding scheduled queue
    ++m_count[q_type];

    // Update uop has been tranferred from alloc queue to the sched queue
    cur_uop->m_in_iaq       = false;
    cur_uop->m_in_scheduler = true;

    ++m_num_in_sched;

   	POWER_CORE_EVENT(m_core_id, POWER_RESERVATION_STATION_W);
   	
    // -------------------------------------
    // Add the uop's entry identifier in the ROB to the schedule list (scheduler insertion) 
    // -------------------------------------
    m_schedule_list[m_last_schlist_ptr] = entry;
    m_last_schlist_ptr = (m_last_schlist_ptr + 1) % MAX_SCHED_SIZE;

    // update the element m_count for the corresponding sched queue
    m_num_per_sched[q_type] = m_num_per_sched[q_type] + 1;
  }
}
