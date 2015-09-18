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
 * File         : rob.cc
 * Author       : Hyesoon Kim 
 * Date         : 1/1/2008
 * SVN          : $Id: main.cc,v 1.26 2008-09-21 00:02:54 kacear Exp $:
 * Description  : reorder buffer
 *********************************************************************************************/


#include "assert.h"
#include "config.h"
#include "global_types.h"
#include "knob.h"
#include "rob.h"
#include "uop.h"
#include "debug_macros.h"

#include "all_knobs.h"


// TOCHECK FIXME
// max_cnt is why 2 times more than rob size?


// rob_c constructor
rob_c::rob_c(Unit_Type type, macsim_c* simBase) 
  : m_fence(simBase)
{

  m_simBase = simBase;

  m_unit_type = type;

  ROB_CONFIG();

  m_max_cnt = m_knob_rob_size * 2; 

  m_rob = new uop_c *[m_max_cnt];

  for (int i = 0; i < m_max_cnt; ++i) {
    m_rob[i] = static_cast<uop_c*>(0); 
  }

  m_usable_cnt = m_max_cnt/2; // don't count space for STD

  m_first_entry = 0;
  m_last_entry  = 0;
  m_free_cnt    = m_usable_cnt;

  // load and store buffers
  m_max_sb_cnt = m_knob_meu_nsb;
  m_num_sb     = m_knob_meu_nsb;

  m_max_lb_cnt = m_knob_meu_nlb;
  m_num_lb     = m_knob_meu_nlb;

  // int and fp registers
  m_max_int_regs = *m_simBase->m_knobs->KNOB_INT_REGFILE_SIZE;
  m_num_int_regs = *m_simBase->m_knobs->KNOB_INT_REGFILE_SIZE;

  m_max_fp_regs = *m_simBase->m_knobs->KNOB_FP_REGFILE_SIZE;
  m_num_fp_regs = *m_simBase->m_knobs->KNOB_FP_REGFILE_SIZE;

  m_wb_empty = true;
}


// rob_c destructor
rob_c::~rob_c()
{
  delete [] m_rob; 
  ASSERT(m_fence.is_list_empty());
}


// insert an uop to reorder buffer
void rob_c::push(uop_c *uop) 
{
  m_rob[m_last_entry] = uop;
  m_last_entry        = (m_last_entry + 1) % m_max_cnt;
  --m_free_cnt;
}


// pop an uop from reorder buffer (doesn't actually return an uop)
// one can get an uop using [] operator (defined in rob.h)
void rob_c::pop()
{
  m_first_entry = (m_first_entry + 1) % m_max_cnt;
  ++m_free_cnt;
}


// initialize reorder buffer
void rob_c::reinit() 
{
  for (int i = 0; i < m_max_cnt; ++i) {
    m_rob[i] = static_cast<uop_c*>(0); 
  }

  m_first_entry = 0;
  m_last_entry  = 0;
  m_usable_cnt  = m_max_cnt / 2;
  m_free_cnt    = m_usable_cnt;
}


// allocate a physical integer register
void rob_c::alloc_int_reg()
{
  ASSERT(m_num_int_regs > 0);
  --m_num_int_regs;
}


// allocate a physical fp register
void rob_c::alloc_fp_reg()
{
  ASSERT(m_num_fp_regs > 0);
  --m_num_fp_regs;
}


// deallocate a physical fp register
void rob_c::dealloc_fp_reg()
{
  ++m_num_fp_regs;
}


// deallocate a physical int register
void rob_c::dealloc_int_reg()
{
  ++m_num_int_regs;
}

bool rob_c::pending_mem_ops(int entry)
{
  int search_idx = m_first_entry;

  while (search_idx != entry) {
    uop_c *uop = m_rob[search_idx];

    if (uop->m_mem_type != NOT_MEM)
      return true;

    search_idx = (search_idx + 1) % m_max_cnt;
  }

  if (KNOB(KNOB_USE_WB)->getValue())
    return m_wb_empty == false;

  return false;
}

int rob_c::get_relative_index(int entry)
{
  if (m_first_entry <= entry)
    return entry - m_first_entry;
  else
    return entry + m_max_cnt - m_first_entry;
}

bool rob_c::is_later_entry(int first_fence_entry, int entry)
{
  int relative_fence_entry = get_relative_index(first_fence_entry);
  int relative_mem_entry   = get_relative_index(entry);

  if (relative_mem_entry < relative_fence_entry)
    return false;

  return true;
}

// true: ensure ordering, false: no ordering necessary
bool rob_c::ensure_mem_ordering(int entry)
{
  // ordering is ensured using bitmap method
  if (KNOB(KNOB_ACQ_REL)->getValue())
    return false;

  // no fence active, no ordering necessary
  if (m_fence.is_list_empty())
    return false;

  bool ret = false;

  for (int i = NOT_FENCE+1; i < FENCE_NUM; i++) {
    fence_type ft = static_cast<fence_type>(i);
    for (auto it = m_fence.cbegin(ft); !ret && it != m_fence.cend(ft); it++) {
      int fence_entry = *it;
      ret = is_later_entry(fence_entry, entry);
    }
  }

  return ret;
}

// false -> empty: no fence active
// true  -> a fence is active
bool rob_c::is_fence_active()
{
  return m_fence.is_fence_active();
}

void rob_c::ins_fence_entry(int entry, fence_type ft)
{
  m_fence.ins_fence_entry(entry, ft);
}

// Deactivate the first fence entry
void rob_c::del_fence_entry(fence_type ft)
{
  m_fence.del_fence_entry(ft);
}

void rob_c::print_fence_entries(fence_type ft)
{
  m_fence.print_fence_entries(ft);
}

void rob_c::set_wb_empty(bool state)
{
  m_wb_empty = state;
}
