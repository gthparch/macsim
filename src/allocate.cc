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
 * File         : allocate.cc
 * Author       : Hyesoon Kim 
 * Date         : 1/1/2008
 * SVN          : $Id: main.cc,v 1.26 2008-09-21 00:02:54 kacear Exp $:
 * Description  : allocate an instruction (uop) into reorder buffer
 *********************************************************************************************/


/*
 * Summary: Allocation stage
 *
 *   Allocate instructions (uops) from frontend_queue to reorder buffer 
 *   until it reaches pipeline depth
 *
 *   frontend_q -> (alloc_q, rob)
 */


#include "allocate.h"
#include "core.h"
#include "pqueue.h"
#include "rob.h"
#include "uop.h"
#include "utils.h"
#include "statistics.h"
#include "bp.h"

#include "debug_macros.h"
#include "all_knobs.h"

#define DEBUG(args...)   _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_ALLOC_STAGE, ## args) 


///////////////////////////////////////////////////////////////////////////////////////////////


// allocate_c constructor
allocate_c::allocate_c(int core_id, pqueue_c<int *> *q_frontend, pqueue_c<int> **alloc_q, 
    pool_c<uop_c> *uop_pool, rob_c *rob, Unit_Type unit_type, int num_queues,
    macsim_c* simBase) 
{
  m_core_id          = core_id;
  m_frontend_q       = q_frontend;
  m_alloc_q          = alloc_q;
  m_uop_pool         = uop_pool;
  m_rob              = rob;
  m_unit_type        = unit_type;
  m_allocate_running = true; 
  m_num_queues       = num_queues;
  m_simBase          = simBase;
  
  // configuration
  switch (unit_type) {
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
}


// Allocate rob entries for instructions from frontend queue every cycle
void allocate_c::run_a_cycle(void)
{
  // pipeline is not running
  if (!m_allocate_running)
    return;

  // check if the first element of frontend queue is ready i.e. 
  // has gone through all intermediate pipeline stages
  for (int cnt = 0; cnt < m_knob_width; ++cnt) {
    // frontend queue is empty
    if (!m_frontend_q->ready())
      break;

    // fetch an uop from frontend queue
    uop_c *uop = (uop_c *)m_frontend_q->peek(0);

    DEBUG("core_id:%d thread_id:%d uop_num:%s is peeked\n", 
        m_core_id, uop->m_thread_id, unsstr64(uop->m_uop_num));

    // -------------------------------------
    // check resource requirement
    // -------------------------------------
    int req_rob     = 1;        // require rob entries
    int req_sb      = 0;        // require store buffer entries
    int req_lb      = 0;        // require load buffer entries
    int req_int_reg = 0;        // require integer register
    int req_fp_reg  = 0;        // require fp register
    int q_type      = *m_simBase->m_knobs->KNOB_GEN_ALLOCQ_INDEX;

    if (uop->m_mem_type == MEM_LD) // load queue
      req_lb = 1;
    else if (uop->m_mem_type == MEM_ST) // store queue
      req_sb = 1;
    else if (uop->m_uop_type == UOP_IADD || // integer register
        uop->m_uop_type == UOP_IMUL || 
        uop->m_uop_type == UOP_ICMP) 
      req_int_reg = 1;
    else if (uop->m_uop_type == UOP_FCVT || uop->m_uop_type == UOP_FADD) // fp register
      req_fp_reg = 1;


    // single allocation queue
    if (m_num_queues == 1) {
      q_type = *m_simBase->m_knobs->KNOB_GEN_ALLOCQ_INDEX;
    }
    // multiple allocation queues
    else { 
      if (req_fp_reg) 
        q_type = *m_simBase->m_knobs->KNOB_FLOAT_ALLOCQ_INDEX;
      else if (req_sb || req_lb) 
        q_type = *m_simBase->m_knobs->KNOB_MEM_ALLOCQ_INDEX;
      else 
        q_type = *m_simBase->m_knobs->KNOB_GEN_ALLOCQ_INDEX;
    }  

    pqueue_c<int> *alloc_q = m_alloc_q[q_type];

    // check rob and other physical resources
    if (m_rob->space() < req_rob || 
        m_rob->get_num_sb() < req_sb || 
        m_rob->get_num_lb() < req_lb || 
        alloc_q->space() < 1 || 
        m_rob->get_num_int_regs () < req_int_reg || 
        m_rob->get_num_fp_regs() < req_fp_reg) {
      break;
    }

    // no stall allocate resources 
    uop->m_alloc_cycle = m_simBase->m_core_cycle[m_core_id];


    // allocate physical resources
    if (req_sb) {
      m_rob->alloc_sb(); 
      uop->m_req_sb = true; 
    }
    else if (req_lb) {
      m_rob->alloc_lb(); 
      uop->m_req_lb = true;
    }
    else if (req_int_reg) {
      m_rob->alloc_int_reg();
      uop->m_req_int_reg = true;
    }
    else if (req_fp_reg) {
      m_rob->alloc_fp_reg();
      uop->m_req_fp_reg = true;
    }

    // -------------------------------------
    // enqueue an entry in allocate queue
    // -------------------------------------
    alloc_q->enqueue(0, m_rob->last_rob());

    POWER_CORE_EVENT(m_core_id, POWER_INST_QUEUE_W);
    POWER_CORE_EVENT(m_core_id, POWER_INST_COMMIT_SEL_LOGIC_W);
    POWER_CORE_EVENT(m_core_id, POWER_UOP_QUEUE_W);	// FIXME jieun 0109
    POWER_CORE_EVENT(m_core_id, POWER_REG_RENAMING_TABLE_W);
    POWER_CORE_EVENT(m_core_id, POWER_FREELIST_W);
    POWER_CORE_EVENT(m_core_id, POWER_REORDER_BUF_W);

    // -------------------------------------
    // insert an uop into reorder buffer
    // -------------------------------------
    uop->m_allocq_num = (q_type == *m_simBase->m_knobs->KNOB_GEN_ALLOCQ_INDEX) ? gen_ALLOCQ :
                         (q_type == *m_simBase->m_knobs->KNOB_MEM_ALLOCQ_INDEX) ? mem_ALLOCQ : fp_ALLOCQ;
    m_rob->push(uop);

    POWER_CORE_EVENT(m_core_id, POWER_REORDER_BUF_W);

    // -------------------------------------
    // dequeue from frontend queue
    // -------------------------------------
    m_frontend_q->dequeue(); 

    DEBUG("cycle_count:%lld core_id:%d uop_num:%lld inst_num:%lld uop.va:0x%s "
        "alloc_q:%d mem_type:%d\n", 
        m_simBase->m_core_cycle[m_core_id], m_core_id, uop->m_uop_num, uop->m_inst_num, 
        hexstr64s(uop->m_vaddr), uop->m_allocq_num, uop->m_mem_type); 

    DEBUG("core_id:%d thread_id:%d id:%lld uop is pushed. inst_count:%lld\n", 
        m_core_id, uop->m_thread_id, uop->m_uop_num, uop->m_inst_num);
  

    // BTB miss is resolved 
    if (uop->m_uop_info.m_btb_miss && !(uop->m_uop_info.m_btb_miss_resolved)) {
      // indirect branch and indirect call cannot resolve the target address in the decode stage 
      if ((uop->m_cf_type < CF_IBR) && (uop->m_cf_type > CF_ICO)) {
        m_bp_data->m_bp_targ_pred->update(uop); 
        m_bp_data->m_bp_redirect_cycle[uop->m_thread_id] = 
          m_cur_core_cycle + 1 + *m_simBase->m_knobs->KNOB_EXTRA_RECOVERY_CYCLES; // redirect cycle 
        uop->m_uop_info.m_btb_miss_resolved = true; 

        DEBUG("cycle_count:%lld core_id:%d uop_num:%lld inst_num:%lld btb_miss "
            "resolved redirect_cycle:%lld\n", 
            m_simBase->m_core_cycle[m_core_id], m_core_id, uop->m_uop_num, uop->m_inst_num, 
            m_bp_data->m_bp_redirect_cycle[uop->m_thread_id]); 

        STAT_CORE_EVENT(m_core_id, BP_REDIRECT_RESOLVED); 
      }
    }
  }
}


