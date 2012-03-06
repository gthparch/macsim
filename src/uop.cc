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
 * File         : uop.cc
 * Author       : Hyesoon Kim
 * Date         : 12/16/2007
 * SVN          : $Id: uop.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : uop structure
 *********************************************************************************************/


#include "uop.h"
#include "knob.h"
#include "all_knobs.h"
#include "global_defs.h"
#include "global_types.h"
#include "debug_macros.h"
#include "map.h"
#include "core.h"


// uop memory type string
const char *uop_c::g_mem_type_name[NUM_MEM_TYPES] = {
  "NOT",
  "LD",
  "ST",
  "PF",
  "WH",
  "EVICT",
  "SWPREF_NTA",
  "SWPREF_T0",
  "SWPREF_T1",
  "SWPREF_T2",
  "LD_LM",
  "LD_SM",
  "LD_GM",
  "ST_LM",
  "ST_SM",
  "ST_GM",
  "LD_CM",
  "LD_TM",
  "LD_PM",
};


// uop state string
const char *uop_c::g_uop_state_name[NUM_OP_STATES] = {
  "OS_FETCHED",
  "OS_ISSUED",
  "OS_SCHEDULED",
  "OS_MISS",
  "OS_WAIT_MEM",
  "OS_DONE",
  "OS_ALLOCATE",
  "OS_EXEC",
  "OS_MERGED",
  "OS_SCHEDULE",
  "OS_SCHEDULE_DONE",
  "OS_DCACHE_BEGIN",
  "OS_DCACHE_HIT",
  "OS_DCACHE_ACCESS",
  "OS_DCACHE_MEM_ACCESS_DENIED",
  "OS_EXEC_BEGIN",
};


// branch type string
const char *uop_c::g_cf_type_name[NUM_CF_TYPES] = {
  "NOT_CF",
  "CF_BR",
  "CF_CBR",
  "CF_CALL",
  "CF_IBR",
  "CF_ICALL",
  "CF_ICO",
  "CF_RET",
  "CF_MITE",
};


// uop dependence type string
const char *uop_c::g_dep_type_name[NUM_DEP_TYPES] = {
  "REG_DATA_DEP",
  "MEM_ADDR_DEP",
  "MEM_DATA_DEP",
  "PREV_UOP_DEP",
};


// uop type string
const char *uop_c::g_uop_type_name[NUM_UOP_TYPES] = {
  "UOP_INV",
  "UOP_SPEC",
  "UOP_NOP",
  "UOP_CF",
  "UOP_CMOV",
  "UOP_LDA",
  "UOP_IMEM",
  "UOP_IADD",
  "UOP_IMUL",
  "UOP_ICMP",
  "UOP_LOGIC",
  "UOP_SHIFT",
  "UOP_BYTE",
  "UOP_MM",
  "UOP_FMEM",
  "UOP_FCF",
  "UOP_FCVT",
  "UOP_FADD",
  "UOP_FMUL",
  "UOP_FDIV",
  "UOP_FCMP",
  "UOP_FBIT",
  "UOP_FCMOV",
  "UOP_LD",
  "UOP_ST",
  "UOP_SSE",
};


// constructor
uop_c::uop_c(macsim_c* simBase)
{
  m_simBase = simBase;
  init();
  m_valid = false;
}

uop_c::uop_c()
{
  init();
  m_valid = false;
}


// initialize an uop
void uop_c::init()
{
  for (int ii = 0; ii < MAX_SRCS; ++ii) {
    m_map_src_info[ii].m_uop = NULL;
  }

  for (int ii = 0; ii < MAX_DESTS; ++ii) {
    m_dest_info[ii] = 0;
  }

  m_uop_num                           = 0; 
  m_inst_num                          = 0; 
  m_thread_id                         = -1; 
  m_unique_thread_id                  = -1; 
  m_orig_thread_id                    = -1;
  m_off_path                          = 0; 
  m_fetched_cycle                     = 0;
  m_bp_cycle                          = 0;
  m_alloc_cycle                       = 0;
  m_sched_cycle                       = 0;
  m_exec_cycle                        = 0;
  m_done_cycle                        = 0;
  m_mem_start_cycle                   = 0;
  m_srcs_not_rdy_vector               = 0;
  m_last_dep_exec                     = NULL;
  m_srcs_rdy                          = 0;
  m_mem_type                          = NOT_MEM; 
  m_bogus                             = false;
  m_active_mask                       = 0;
  m_taken_mask                        = 0;
  m_reconverge_addr                   = 0;
  m_target_addr                       = 0;
  m_uop_info.m_misfetch               = false;
  m_uop_info.m_mispred                = false;
  m_uop_info.m_originally_mispred     = false;
  m_uop_info.m_originally_misfetch    = false;
  m_uop_info.m_btb_miss               = false;
  m_uop_info.m_btb_miss_resolved      = false; 
  m_uop_info.m_no_target              = false;
  m_uop_info.m_ibp_miss               = false;
  m_uop_info.m_icmiss                 = false;
  m_uop_info.m_dcmiss                 = false;
  m_uop_info.m_l2_miss                = false;
  m_num_child_uops                    = 0;
  m_num_child_uops_done               = 0;
  m_child_uops                        = NULL;
  m_parent_uop                        = NULL;
  m_pending_child_uops                = 0;
  m_uncoalesced_flag                  = false;
  m_last_uop                          = false;
  m_req_sb                            = false;
  m_req_lb                            = false;
  m_req_int_reg                       = false;
  m_req_fp_reg                        = false;
  m_dcache_bank_id                    = 128;
  m_bypass_llc                        = false;
  m_skip_llc                          = false;
}


// initialize a new uop
void uop_c::allocate()
{
  m_valid = true;
  init();
}


// deallocate an uop
uop_c* uop_c::free()
{
  m_uop_num        = 0 ; 
  m_inst_num       = 0 ; 
  m_valid = false;
  
  if ((m_mem_type == MEM_ST) || (m_mem_type == MEM_ST_LM)) {
    delete_store_hash_entry_wrapper(m_simBase->m_core_pointers[m_core_id]->get_map(), this);
  }
  m_thread_id        = -1; 
  m_unique_thread_id = -1; 
  m_orig_thread_id   = -1;
  m_dcache_bank_id   = 128;
  m_bypass_llc       = false;
  m_skip_llc         = false;

  return this;
}
