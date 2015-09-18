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
  "UOP_LFENCE",
  "UOP_FULL_FENCE",
  "UOP_ACQ_FENCE",
  "UOP_REL_FENCE",
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

  "UOP_GPU_ABS",
  "UOP_GPU_ABS64",
  "UOP_GPU_ADD", 
  "UOP_GPU_ADD64", 
	"UOP_GPU_ADDC",
	"UOP_GPU_AND",
	"UOP_GPU_AND64",
	"UOP_GPU_ATOM",
	"UOP_GPU_ATOM64",
	"UOP_GPU_BAR",
	"UOP_GPU_BFE",
	"UOP_GPU_BFE64",
	"UOP_GPU_BFI",
	"UOP_GPU_BFI64",
	"UOP_GPU_BFIND",
	"UOP_GPU_BFIND64",
	"UOP_GPU_BRA",
	"UOP_GPU_BREV",
	"UOP_GPU_BREV64",
	"UOP_GPU_BRKPT",
	"UOP_GPU_CALL",
	"UOP_GPU_CLZ",
	"UOP_GPU_CLZ64",
	"UOP_GPU_CNOT",
	"UOP_GPU_CNOT64",
	"UOP_GPU_COPYSIGN",
	"UOP_GPU_COPYSIGN64",
	"UOP_GPU_COS",
	"UOP_GPU_CVT",
	"UOP_GPU_CVT64",
	"UOP_GPU_CVTA",
	"UOP_GPU_CVTA64",
	"UOP_GPU_DIV",
	"UOP_GPU_DIV64",
	"UOP_GPU_EX2",
	"UOP_GPU_EXIT",
	"UOP_GPU_FMA",
	"UOP_GPU_FMA64",
	"UOP_GPU_ISSPACEP",
	"UOP_GPU_LD",
	"UOP_GPU_LD64",
	"UOP_GPU_LDU",
	"UOP_GPU_LDU64",
	"UOP_GPU_LG2",
	"UOP_GPU_MAD24",
	"UOP_GPU_MAD",
	"UOP_GPU_MAD64",
	"UOP_GPU_MADC",
	"UOP_GPU_MADC64",
	"UOP_GPU_MAX",
	"UOP_GPU_MAX64",
	"UOP_GPU_MEMBAR",
	"UOP_GPU_MIN",
	"UOP_GPU_MIN64",
	"UOP_GPU_MOV",
	"UOP_GPU_MOV64",
	"UOP_GPU_MUL24",
	"UOP_GPU_MUL",
	"UOP_GPU_MUL64",
	"UOP_GPU_NEG",
	"UOP_GPU_NEG64",
	"UOP_GPU_NOT",
	"UOP_GPU_NOT64",
	"UOP_GPU_OR",
	"UOP_GPU_OR64",
	"UOP_GPU_PMEVENT",
	"UOP_GPU_POPC",
	"UOP_GPU_POPC64",
	"UOP_GPU_PREFETCH",
	"UOP_GPU_PREFETCHU",
	"UOP_GPU_PRMT",
	"UOP_GPU_RCP",
	"UOP_GPU_RCP64",
	"UOP_GPU_RED",
	"UOP_GPU_RED64",
	"UOP_GPU_REM",
	"UOP_GPU_REM64",
	"UOP_GPU_RET",
	"UOP_GPU_RSQRT",
	"UOP_GPU_RSQRT64",
	"UOP_GPU_SAD",
	"UOP_GPU_SAD64",
	"UOP_GPU_SELP",
	"UOP_GPU_SELP64",
	"UOP_GPU_SET",
	"UOP_GPU_SET64",
	"UOP_GPU_SETP",
	"UOP_GPU_SETP64",
	"UOP_GPU_SHFL",
	"UOP_GPU_SHFL64",
	"UOP_GPU_SHL",
	"UOP_GPU_SHL64",
	"UOP_GPU_SHR",
	"UOP_GPU_SHR64",
	"UOP_GPU_SIN",
	"UOP_GPU_SLCT",
	"UOP_GPU_SLCT64",
	"UOP_GPU_SQRT",
	"UOP_GPU_SQRT64",
	"UOP_GPU_ST",
	"UOP_GPU_ST64",
	"UOP_GPU_SUB",
	"UOP_GPU_SUB64",
	"UOP_GPU_SUBC",
	"UOP_GPU_SULD",
	"UOP_GPU_SULD64",
	"UOP_GPU_SURED",
	"UOP_GPU_SURED64",
	"UOP_GPU_SUST",
	"UOP_GPU_SUST64",
	"UOP_GPU_SUQ",
  "UOP_GPU_TESTP",
  "UOP_GPU_TESTP64",
  "UOP_GPU_TEX",
  "UOP_GPU_TLD4",
  "UOP_GPU_TXQ",
  "UOP_GPU_TRAP",
  "UOP_GPU_VABSDIFF",
  "UOP_GPU_VADD",
  "UOP_GPU_VMAD",
  "UOP_GPU_VMAX",
  "UOP_GPU_VMIN",
  "UOP_GPU_VSET",
  "UOP_GPU_VSHL",
  "UOP_GPU_VSHR",
  "UOP_GPU_VSUB",
  "UOP_GPU_VOTE",
  "UOP_GPU_XOR",
  "UOP_GPU_XOR64",
  "UOP_GPU_RECONVERGE",
  "UOP_GPU_PHI",

  "UOP_GPU_FABS",
  "UOP_GPU_FABS64",
  "UOP_GPU_FADD", 
  "UOP_GPU_FADD64", 
	"UOP_GPU_FADDC",
	"UOP_GPU_FAND",
	"UOP_GPU_FAND64",
	"UOP_GPU_FATOM",
	"UOP_GPU_FATOM64",
	"UOP_GPU_FBAR",
	"UOP_GPU_FBFE",
	"UOP_GPU_FBFE64",
	"UOP_GPU_FBFI",
	"UOP_GPU_FBFI64",
	"UOP_GPU_FBFIND",
	"UOP_GPU_FBFIND64",
	"UOP_GPU_FBRA",
	"UOP_GPU_FBREV",
	"UOP_GPU_FBREV64",
	"UOP_GPU_FBRKPT",
	"UOP_GPU_FCALL",
	"UOP_GPU_FCLZ",
	"UOP_GPU_FCLZ64",
	"UOP_GPU_FCNOT",
	"UOP_GPU_FCNOT64",
	"UOP_GPU_FCOPYSIGN",
	"UOP_GPU_FCOPYSIGN64",
	"UOP_GPU_FCOS",
	"UOP_GPU_FCVT",
	"UOP_GPU_FCVT64",
	"UOP_GPU_FCVTA",
	"UOP_GPU_FCVTA64",
	"UOP_GPU_FDIV",
	"UOP_GPU_FDIV64",
	"UOP_GPU_FEX2",
	"UOP_GPU_FEXIT",
	"UOP_GPU_FFMA",
	"UOP_GPU_FFMA64",
	"UOP_GPU_FISSPACEP",
	"UOP_GPU_FLD",
	"UOP_GPU_FLD64",
	"UOP_GPU_FLDU",
	"UOP_GPU_FLDU64",
	"UOP_GPU_FLG2",
	"UOP_GPU_FMAD24",
	"UOP_GPU_FMAD",
	"UOP_GPU_FMAD64",
	"UOP_GPU_FMAX",
	"UOP_GPU_FMAX64",
	"UOP_GPU_FMEMBAR",
	"UOP_GPU_FMIN",
	"UOP_GPU_FMIN64",
	"UOP_GPU_FMOV",
	"UOP_GPU_FMOV64",
	"UOP_GPU_FMUL24",
	"UOP_GPU_FMUL",
	"UOP_GPU_FMUL64",
	"UOP_GPU_FNEG",
	"UOP_GPU_FNEG64",
	"UOP_GPU_FNOT",
	"UOP_GPU_FNOT64",
	"UOP_GPU_FOR",
	"UOP_GPU_FOR64",
	"UOP_GPU_FPMEVENT",
	"UOP_GPU_FPOPC",
	"UOP_GPU_FPOPC64",
	"UOP_GPU_FPREFETCH",
	"UOP_GPU_FPREFETCHU",
	"UOP_GPU_FPRMT",
	"UOP_GPU_FRCP",
	"UOP_GPU_FRCP64",
	"UOP_GPU_FRED",
	"UOP_GPU_FRED64",
	"UOP_GPU_FREM",
	"UOP_GPU_FREM64",
	"UOP_GPU_FRET",
	"UOP_GPU_FRSQRT",
	"UOP_GPU_FRSQRT64",
	"UOP_GPU_FSAD",
	"UOP_GPU_FSAD64",
	"UOP_GPU_FSELP",
	"UOP_GPU_FSELP64",
	"UOP_GPU_FSET",
	"UOP_GPU_FSET64",
	"UOP_GPU_FSETP",
	"UOP_GPU_FSETP64",
	"UOP_GPU_FSHL",
	"UOP_GPU_FSHL64",
	"UOP_GPU_FSHR",
	"UOP_GPU_FSHR64",
	"UOP_GPU_FSIN",
	"UOP_GPU_FSLCT",
	"UOP_GPU_FSLCT64",
	"UOP_GPU_FSQRT",
	"UOP_GPU_FSQRT64",
	"UOP_GPU_FST",
	"UOP_GPU_FST64",
	"UOP_GPU_FSUB",
	"UOP_GPU_FSUB64",
	"UOP_GPU_FSUBC",
	"UOP_GPU_FSULD",
	"UOP_GPU_FSULD64",
	"UOP_GPU_FSURED",
	"UOP_GPU_FSURED64",
	"UOP_GPU_FSUST",
	"UOP_GPU_FSUST64",
	"UOP_GPU_FSUQ",
  "UOP_GPU_FTESTP",
  "UOP_GPU_FTESTP64",
  "UOP_GPU_FTEX",
  "UOP_GPU_FTLD4",
  "UOP_GPU_FTXQ",
  "UOP_GPU_FTRAP",
  "UOP_GPU_FVABSDIFF",
  "UOP_GPU_FVADD",
  "UOP_GPU_FVMAD",
  "UOP_GPU_FVMAX",
  "UOP_GPU_FVMIN",
  "UOP_GPU_FVSET",
  "UOP_GPU_FVSHL",
  "UOP_GPU_FVSHR",
  "UOP_GPU_FVSUB",
  "UOP_GPU_FVOTE",
  "UOP_GPU_FXOR",
  "UOP_GPU_FXOR64",
  "UOP_GPU_FRECONVERGE",
  "UOP_GPU_FPHI"
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

  m_hmc_inst                          = HMC_NONE;
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
