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


/***************************************************************************************
 * File         : bp_targ.cc
 * Author       : Hyesoon Kim 
 * Date         : 4/5/2010 
 * SVN          : $Id: bp.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : branch target predictor 
 ***************************************************************************************/

/* 
 * Summary: Branch Target Prediction Mechanisms. The default behavior: BTB 
 */

#include "global_types.h"
#include "global_defs.h"
#include "bp_targ.h"
#include "all_knobs.h"
#include "cache.h"
#include "statistics.h"
#include "statsEnums.h"

#include "debug_macros.h"

#define DEBUG(args...)   _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_BTB, ## args)


bp_targ_c::bp_targ_c(uns core_id_arg, macsim_c* simBase) 
{
  btb = new cache_c("btb", simBase->m_knobs->KNOB_BTB_ENTRIES->getValue() * 4, 
      simBase->m_knobs->KNOB_BTB_ASSOC->getValue(), 4, sizeof(Addr), 
      simBase->m_knobs->KNOB_BTB_BANK_NUM->getValue(), false, core_id_arg, CACHE_BTB,
      false, simBase); 
		    
  m_core_id = core_id_arg; 
  m_simBase = simBase; 
}


Addr bp_targ_c::pred (uop_c *uop)
{
  Addr line_addr, return_addr=0;
  Addr* return_addr_ptr = NULL;
  bool perfect_pred = false; 
  
  perfect_pred = (m_simBase->m_knobs->KNOB_PERFECT_BTB); 

  if (perfect_pred) { 
    return_addr = uop->m_npc; 
    STAT_CORE_EVENT(m_core_id, PERFECT_TARGET_PRED); 
  }
  else { 
    int appl_id = m_simBase->m_core_pointers[uop->m_core_id]->get_appl_id(uop->m_thread_id);
    return_addr_ptr = (Addr *) btb->access_cache (uop->m_pc, &line_addr, true, appl_id);
    return_addr = return_addr_ptr ? *return_addr_ptr : 0; 
  }
  
  Addr tag; 
  int set; 
  btb->find_tag_and_set(uop->m_pc, &tag, &set);
  uop->m_uop_info.m_btb_set = set; 

  return return_addr; 
}


void bp_targ_c::update (uop_c *uop)
{
  Addr fetch_addr = uop->m_pc; 
  Addr *btb_line = NULL;
  Addr btb_line_addr, repl_line_addr; 
  bool insert_btb = false; 

  if (uop->m_off_path) 
    return; 
  
  Addr tag; 
  int set; 
  btb->find_tag_and_set(uop->m_pc, &tag, &set);

  int appl_id = m_simBase->m_core_pointers[uop->m_core_id]->get_appl_id(uop->m_thread_id);
  btb_line = (Addr *)btb->access_cache(fetch_addr, &btb_line_addr, false, appl_id);

  if (btb_line == NULL) { 
    btb_line = (Addr *)btb->insert_cache(fetch_addr, &btb_line_addr, &repl_line_addr, appl_id, 
        false); 
    insert_btb = true; 
  }

  *btb_line = uop->m_target_addr; 

  DEBUG("Writing BTB pc:0x%s target:0x%s m_uop_num:%s core_id:%d thread_id:%d cf_type:%d "
      "btb_line:%s set:%d tag:0x%s insert_btb:%d\n",
      hexstr64s(uop->m_pc), hexstr64s(uop->m_target_addr), unsstr64(uop->m_uop_num),
      m_core_id, uop->m_thread_id, uop->m_cf_type, (btb_line) ? hexstr64s(*btb_line) : "-1", 
      set, hexstr64s(tag), insert_btb);
}
