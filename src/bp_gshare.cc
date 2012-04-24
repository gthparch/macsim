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


/* ********************************************************************************************
 * File         : bp_gshare.cc
 * Author       : Hyesoon Kim 
 * Date         : 10/28/2008 
 * CVS          : $Id: frontend.cc,v 1.9 2008-09-12 03:07:29 kacear Exp $:
 * Description  : gshare branch predictor
 *********************************************************************************************/


#include <cstdlib>

#include "bp.h"
#include "bp_gshare.h"
#include "utils.h"
#include "debug_macros.h"
#include "uop.h"

#include "all_knobs.h"

#define DEBUG(args...)   _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_BP_DIR, ## args) 

#define PHT_INIT_VALUE ((0x1 << *KNOB(KNOB_PHT_CTR_BITS)) - 1) /* weakly taken */
#define COOK_HIST_BITS(hist,len,untouched) \
  ((uns32)(hist) >> (32 - (len) + (untouched)) << (untouched))
#define COOK_ADDR_BITS(addr,len,shift) (((uns32)(addr) >> (shift)) & (N_BIT_MASK((len))))
#define SAT_INC(val, max) ((val) == (max) ? (max) : (val) + 1)
#define SAT_DEC(val, min) ((val) == (min) ? (min) : (val) - 1)


///////////////////////////////////////////////////////////////////////////////////////////////


// bp_gshare_c constructor
bp_gshare_c::bp_gshare_c(macsim_c* simBase) : bp_dir_base_c(simBase)
{
  m_pht = (uns8 *)malloc(sizeof(uns8) * (0x1 << *KNOB(KNOB_BP_HIST_LENGTH)));
  for (int ii = 0; ii < (0x1 << *KNOB(KNOB_BP_HIST_LENGTH)); ++ii) {
    m_pht[ii] = PHT_INIT_VALUE;
  }
}


// branch prediction
uns8 bp_gshare_c::pred (uop_c *uop)
{
  Addr  addr        = uop->m_pc; 
  uns32 hist        = m_global_hist;
  uns32 cooked_hist = COOK_HIST_BITS(hist, *m_simBase->m_knobs->KNOB_BP_HIST_LENGTH, 0);
  uns32 cooked_addr = COOK_ADDR_BITS(addr, *m_simBase->m_knobs->KNOB_BP_HIST_LENGTH, 2);
  uns32 pht_index   = cooked_hist ^ cooked_addr;
  uns8  pht_entry   = m_pht[pht_index];
  uns8  pred        = ((pht_entry >> *KNOB(KNOB_PHT_CTR_BITS)) - 1) & 0x1;
  

  uop->m_uop_info.m_pred_global_hist = m_global_hist;
  m_global_hist >>= 1;
  uop->m_recovery_info.m_global_hist = this->m_global_hist | uop->m_dir << 31; 
  m_global_hist |= pred << 31;

  DEBUG("Predicting core:%d thread_id:%d uop_num:%s addr:%s  index:%d  pred:%d  dir:%d\n", 
        uop->m_core_id, uop->m_thread_id, unsstr64(uop->m_uop_num), hexstr64s(addr), 
        pht_index, pred, uop->m_dir);

  return pred;
}


// update branch predictor
void bp_gshare_c::update (uop_c *uop)
{
  Addr  addr        = uop->m_pc;
  uns32 hist        = uop->m_uop_info.m_pred_global_hist;
  uns32 cooked_hist = COOK_HIST_BITS(hist, *m_simBase->m_knobs->KNOB_BP_HIST_LENGTH, 0);
  uns32 cooked_addr = COOK_ADDR_BITS(addr, *m_simBase->m_knobs->KNOB_BP_HIST_LENGTH, 2);
  uns32 pht_index   = cooked_hist ^ cooked_addr;
  uns8  pht_entry   = m_pht[pht_index];

  DEBUG("Writing gshare PHT for  op_num:%s  index:%d  dir:%d max value is :%lld \n", 
      unsstr64(uop->m_uop_num), pht_index, uop->m_dir, 
      N_BIT_MASK(*m_simBase->m_knobs->KNOB_PHT_CTR_BITS));


  if (uop->m_dir) {
    m_pht[pht_index] = SAT_INC(pht_entry, N_BIT_MASK(*m_simBase->m_knobs->KNOB_PHT_CTR_BITS));
  }
  else {
    m_pht[pht_index] = SAT_DEC(pht_entry, 0);
  }


  DEBUG("Updating addr:%s  pht:%u  ent:%u  dir:%d\n", 
        hexstr64s(addr), pht_index, m_pht[pht_index], uop->m_dir);
}


// recovery from branch-mis prediction
void bp_gshare_c::recover(recovery_info_c *recovery_info)
{
  m_global_hist    = recovery_info->m_global_hist;
  m_global_hist_64 = recovery_info->m_global_hist_64;
}

