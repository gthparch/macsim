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
 * File         : allocate_smc.cc
 * Author       : Hyesoon Kim 
 * Date         : 1/1/2008
 * SVN          : $Id: main.cc,v 1.26 2008-09-21 00:02:54 kacear Exp $:
 * Description  : allocate an instruction (uop) into reorder buffer
 *                (small many core) allocate stage
 *********************************************************************************************/


#include "allocate.h"
#include "core.h"
#include "allocate_smc.h"
#include "pqueue.h"
#include "rob_smc.h"
#include "uop.h"
#include "utils.h"
#include "statistics.h"

#include "debug_macros.h"
#include "all_knobs.h"

///////////////////////////////////////////////////////////////////////////////////////////////


#define DEBUG(args...)   _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_ALLOC_STAGE, ## args) 


///////////////////////////////////////////////////////////////////////////////////////////////


// constructor
smc_allocate_c::smc_allocate_c(int core_id, pqueue_c<int*> *q_frontend, 
    pqueue_c<gpu_allocq_entry_s> **gpu_alloc_q, pool_c<uop_c> *uop_pool, smc_rob_c *gpu_rob, 
    Unit_Type unit_type, int num_queues, macsim_c* simBase)
{
  m_core_id          = core_id;
  m_frontend_q       = q_frontend;
  m_gpu_alloc_q      = gpu_alloc_q;
  m_uop_pool         = uop_pool;
  m_gpu_rob          = gpu_rob;
  m_unit_type        = unit_type;
  m_allocate_running = true;
  m_num_queues       = num_queues;

  m_simBase          = simBase;

  switch (m_unit_type) {
    case UNIT_SMALL:
      m_knob_width = *m_simBase->m_knobs->KNOB_WIDTH;
      break;
    case UNIT_MEDIUM:
      m_knob_width = *m_simBase->m_knobs->KNOB_MEDIUM_WIDTH;
      break;
    case UNIT_LARGE:
      m_knob_width = *m_simBase->m_knobs->KNOB_LARGE_WIDTH;
      break;
  }

  gpu_rob = (smc_rob_c *)m_gpu_rob;//dynamic cast requires base class to be polymorphic
}


// destructor
smc_allocate_c::~smc_allocate_c()
{
}


// run_a_cycle : Allocate rob entries for instructions from frontend queue every cycle
void smc_allocate_c::run_a_cycle(void)
{
  if (!m_allocate_running)
    return;

  int cnt = 0;
  while (cnt < m_knob_width && m_frontend_q->ready()) {
    ++cnt;

    // fetch an uop from frontend queue
    uop_c *uop = (uop_c *) m_frontend_q->peek(0);
    DEBUG("core_id:%d thread_id:%d inst_num:%s uop_num:%s is peeked\n", 
        m_core_id, uop->m_thread_id, unsstr64(uop->m_inst_num),
        unsstr64(uop->m_uop_num));
    ASSERT(uop);
    
    // check resource requirement
    int req_rob     = 1;        // required rob entries
    int req_sb      = 0;        // required store buffer entries
    int req_lb      = 0;        // required load buffer entries
    int req_int_reg = 0;        // required alloc queue type
    int req_fp_reg  = 0;        // required integer registers 

    if (uop->m_uop_type == UOP_LD) // load queue
      req_lb = 1;
    else if (uop->m_uop_type == UOP_ST) // store queue 
      req_sb = 1;
    else if (uop->m_uop_type == UOP_IADD || // integer register
             uop->m_uop_type == UOP_IMUL ||
             uop->m_uop_type == UOP_ICMP) 
      req_int_reg = 1;
    else if (uop->m_uop_type == UOP_FCVT || uop->m_uop_type == UOP_FADD) // fp register
      req_fp_reg = 1;


    pqueue_c<gpu_allocq_entry_s>* gpu_alloc_q;
    ALLOCQ_Type gpu_alloc_q_type;

    if (*KNOB(KNOB_GPU_USE_SINGLE_ALLOCQ_TYPE) && 
        *KNOB(KNOB_GPU_SHARE_ALLOCQS_BETWEEN_THREADS)) {
      gpu_alloc_q      = m_gpu_alloc_q[*m_simBase->m_knobs->KNOB_GEN_ALLOCQ_INDEX];
      gpu_alloc_q_type = gen_ALLOCQ;
    }
    else {
      int q_type = *m_simBase->m_knobs->KNOB_GEN_ALLOCQ_INDEX;
      
      // multiple Alloc queues
      if (req_fp_reg) {
        gpu_alloc_q_type = fp_ALLOCQ;  
        q_type           = *KNOB(KNOB_FLOAT_ALLOCQ_INDEX);
      }   
      else if (req_sb || req_lb) {
        gpu_alloc_q_type = mem_ALLOCQ;
        q_type           = *KNOB(KNOB_MEM_ALLOCQ_INDEX);
      }     
      else {
        gpu_alloc_q_type = gen_ALLOCQ;
        q_type           = *KNOB(KNOB_GEN_ALLOCQ_INDEX);
      }   
      gpu_alloc_q = m_gpu_alloc_q[q_type];
    }

    // FIXME
    // check rob and load store spaces 
    rob_c *thread_rob = m_gpu_rob->get_thread_rob(uop->m_thread_id);
    if (thread_rob->space() < req_rob || thread_rob->get_num_sb() < req_sb ||
        thread_rob->get_num_lb() < req_lb || gpu_alloc_q->space() < 1 ||
        thread_rob->get_num_int_regs () < req_int_reg ||
        thread_rob->get_num_fp_regs() < req_fp_reg) {
      break;
    }

    // no stall allocate resources 
    uop->m_alloc_cycle = m_simBase->m_core_cycle[m_core_id];

    // allocate physical resources
    if (req_sb) {
      thread_rob->alloc_sb(); 
      uop->m_req_sb = true; 
    }
    else if (req_lb) {
      thread_rob->alloc_lb(); 
      uop->m_req_lb = true;
    }
    else if (req_int_reg) {
      thread_rob->alloc_int_reg();
      uop->m_req_int_reg = true;
    }
    else if (req_fp_reg) {
      thread_rob->alloc_fp_reg();
      uop->m_req_fp_reg = true;
    }
    
    // enqueue an entry in allocate queue
    gpu_allocq_entry_s allocq_entry;
    allocq_entry.m_thread_id = uop->m_thread_id;
    allocq_entry.m_rob_entry = thread_rob->last_rob(); 
    bool success = gpu_alloc_q->enqueue(0, allocq_entry);
    ASSERT(success);
    
    POWER_CORE_EVENT(m_core_id, POWER_INST_QUEUE_W);
    POWER_CORE_EVENT(m_core_id, POWER_INST_COMMIT_SEL_LOGIC_W);
    POWER_CORE_EVENT(m_core_id, POWER_UOP_QUEUE_W);	  

    // insert an uop into reorder buffer
    uop->m_allocq_num = gpu_alloc_q_type;
    uop->m_state      = OS_ALLOCATE;
    thread_rob->push(uop);

    
    // dequeue from frontend queue
    m_frontend_q->dequeue();

    DEBUG("cycle_count:%lld core_id:%d uop_num:%lld inst_num:%lld uop.va:0x%s "
          "gpu_alloc_q:%d mem_type:%d thread_id:%d uop is pushed.\n", 
          m_cur_core_cycle, m_core_id, uop->m_uop_num, uop->m_inst_num, 
          hexstr64s(uop->m_vaddr), uop->m_allocq_num, uop->m_mem_type, uop->m_thread_id);
  }
}

