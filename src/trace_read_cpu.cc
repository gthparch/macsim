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
 * File         : trace_read_cpu.cc
 * Author       : HPArch Research Group
 * Date         : 
 * SVN          : $Id: trace_read.cc 912 2009-11-20 19:09:21Z kacear $
 * Description  : Trace handling class 
 *********************************************************************************************/


#include <iostream>
#include <set>

#include "assert_macros.h"
#include "trace_read_cpu.h"
#include "trace_read.h"
#include "uop.h"
#include "global_types.h"
#include "core.h"
#include "knob.h"
#include "process_manager.h"
#include "debug_macros.h"
#include "statistics.h"
#include "frontend.h"
#include "statsEnums.h"
#include "utils.h"
#include "pref_common.h"
#include "readonly_cache.h"
#include "sw_managed_cache.h"
#include "memory.h"
#include "inst_info.h"
#include "page_mapping.h"

#include "all_knobs.h"

///////////////////////////////////////////////////////////////////////////////////////////////


#define DEBUG(args...)   _DEBUG(*KNOB(KNOB_DEBUG_TRACE_READ), ## args)
#define DEBUG_CORE(m_core_id, args...)       \
  if (m_core_id == *m_simBase->m_knobs->KNOB_DEBUG_CORE_ID) {     \
    _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_TRACE_READ, ## args); \
  }


///////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Constructor
 */
cpu_decoder_c::cpu_decoder_c(macsim_c* simBase, ofstream* m_dprint_output)
  : trace_read_c(simBase, m_dprint_output)
{
  m_trace_size = CPU_TRACE_SIZE;

  // physical page mapping
  m_enable_physical_mapping = *KNOB(KNOB_ENABLE_PHYSICAL_MAPPING);

  if (m_enable_physical_mapping)
  {
    std::string policy = simBase->m_knobs->KNOB_PAGE_MAPPING_POLICY->getValue();
    if (policy == "FCFS")
      m_page_mapper = new FCFSPageMapper(simBase, *KNOB(KNOB_PAGE_SIZE));
    else if (policy == "REGION_FCFS")
      m_page_mapper = new RegionBasedFCFSPageMapper(simBase, *KNOB(KNOB_PAGE_SIZE), *KNOB(KNOB_REGION_SIZE));
    else
      assert(0);
  }
}


/**
 * Destructor
 */
cpu_decoder_c::~cpu_decoder_c()
{
  delete m_page_mapper;
}

/**
 * In case of GPU simulation, from our design, each uncoalcesd accesses will be one 
 * trace instruction. To make these accesses in one instruction, we need to read trace
 * file ahead.
 * @param core_id - core id
 * @param trace_info - trace information to store an instruction
 * @param sim_thread_id - thread id
 * @param inst_read - indicate instruction read successful
 * @see get_uops_from_traces
 */
bool cpu_decoder_c::peek_trace(int core_id, void *trace_info, int sim_thread_id, 
    bool *inst_read)
{
  assert(0);
  return false;
}


/**
 * After peeking trace, in case of failture, we need to rewind trace file.
 * @param core_id - core id
 * @param sim_thread_id - thread id
 * @param num_inst - number of instructions to rewind
 * @see peek_trace
 */
bool cpu_decoder_c::ungetch_trace(int core_id, int sim_thread_id, int num_inst)
{
  assert(0);
  return false;
}




/**
 * Dump out instruction information to the file. At most 50000 instructions will be printed
 * @param t_info - trace information
 * @param core_id - core id
 * @param thread_id - thread id
 */
void cpu_decoder_c::dprint_inst(void *trace_info, int core_id, int thread_id) 
{
  if (m_dprint_count++ >= 50000 || !*KNOB(KNOB_DEBUG_PRINT_TRACE))
    return ;

  trace_info_cpu_s *t_info = static_cast<trace_info_cpu_s *>(trace_info);
 
  *m_dprint_output << "*** begin of the data strcture *** " << endl;
  *m_dprint_output << "core_id:" << core_id << " thread_id:" << thread_id << endl;
  *m_dprint_output << "uop_opcode " <<g_tr_opcode_names[(uint32_t) t_info->m_opcode]  << endl;
  *m_dprint_output << "num_read_regs: " << hex <<  (uint32_t) t_info->m_num_read_regs << endl;
  *m_dprint_output << "num_dest_regs: " << hex << (uint32_t) t_info->m_num_dest_regs << endl;
  for (uint32_t ii = 0; ii < (uint32_t) t_info->m_num_read_regs; ++ii)
    *m_dprint_output << "src" << ii << ": " 
      << hex << g_tr_reg_names[static_cast<uint32_t>(t_info->m_src[ii])] << endl;

  for (uint32_t ii = 0; ii < (uint32_t) t_info->m_num_dest_regs; ++ii)
    *m_dprint_output << "dst" << ii << ": " 
      << hex << g_tr_reg_names[static_cast<uint32_t>(t_info->m_dst[ii])] << endl;
  *m_dprint_output << "cf_type: " << hex << g_tr_cf_names[(uint32_t) t_info->m_cf_type] << endl;
  *m_dprint_output << "has_immediate: " << hex << (uint32_t) t_info->m_has_immediate << endl;
  *m_dprint_output << "r_dir:" << (uint32_t) t_info->m_rep_dir << endl;
  *m_dprint_output << "has_st: " << hex << (uint32_t) t_info->m_has_st << endl;
  *m_dprint_output << "num_ld: " << hex << (uint32_t) t_info->m_num_ld << endl;
  *m_dprint_output << "mem_read_size: " << hex << (uint32_t) t_info->m_mem_read_size << endl;
  *m_dprint_output << "mem_write_size: " << hex << (uint32_t) t_info->m_mem_write_size << endl;
  *m_dprint_output << "is_fp: " << (uint32_t) t_info->m_is_fp << endl;
  *m_dprint_output << "ld_vaddr1: " << hex << (uint32_t) t_info->m_ld_vaddr1 << endl;
  *m_dprint_output << "ld_vaddr2: " << hex << (uint32_t) t_info->m_ld_vaddr2 << endl;
  *m_dprint_output << "st_vaddr: " << hex << (uint32_t) t_info->m_st_vaddr << endl;
  *m_dprint_output << "instruction_addr: " << hex << (uint32_t)t_info->m_instruction_addr << endl;
  *m_dprint_output << "branch_target: " << hex << (uint32_t)t_info->m_branch_target << endl;
  *m_dprint_output << "actually_taken: " << hex << (uint32_t)t_info->m_actually_taken << endl;
  *m_dprint_output << "write_flg: " << hex << (uint32_t)t_info->m_write_flg << endl;
  *m_dprint_output << "size: " << hex << (uint32_t) t_info->m_size << endl;
  *m_dprint_output << "*** end of the data strcture *** " << endl << endl;
}


///////////////////////////////////////////////////////////////////////////////////////////////


// FIXME
/**
 * GPU simulation : Read trace ahead to read synchronization information
 * @param trace_info - trace information
 * @see process_manager_c::sim_thread_schedule
 */
void cpu_decoder_c::pre_read_trace(thread_s* trace_info)
{
  assert(0);
}


///////////////////////////////////////////////////////////////////////////////////////////////



/**
 * From statis instruction, add dynamic information such as load address, branch target, ...
 * @param info - instruction information from the hash table
 * @param pi - raw trace information
 * @param trace_uop - MacSim uop type
 * @param rep_offset - repetition offet
 * @param core_id - core id
 */
void cpu_decoder_c::convert_dyn_uop(inst_info_s *info, void *trace_info, trace_uop_s *trace_uop, 
    Addr rep_offset, int core_id)
{
  trace_info_cpu_s *pi = static_cast<trace_info_cpu_s *>(trace_info);
  core_c* core    = m_simBase->m_core_pointers[core_id];
  trace_uop->m_va = 0;

  if (info->m_table_info->m_cf_type) {
    trace_uop->m_actual_taken = pi->m_actually_taken;
    trace_uop->m_target       = pi->m_branch_target;
  
    trace_uop->m_active_mask     = pi->m_ld_vaddr2;
    trace_uop->m_taken_mask      = pi->m_ld_vaddr1;
    trace_uop->m_reconverge_addr = pi->m_st_vaddr;
  } 
  else if (info->m_table_info->m_mem_type) {
    int amp_val = 1;
    if (pi->m_opcode != XED_CATEGORY_STRINGOP) {
      amp_val = *KNOB(KNOB_MEM_SIZE_AMP);
    }

    if (info->m_table_info->m_mem_type == MEM_ST) {
      trace_uop->m_va = MIN2((pi->m_st_vaddr + rep_offset)*amp_val, MAX_ADDR);
      trace_uop->m_mem_size = MIN2((pi->m_mem_write_size)*amp_val, REP_MOV_MEM_SIZE_MAX_NEW);
    } 
    else if ((info->m_table_info->m_mem_type == MEM_LD) || 
        (info->m_table_info->m_mem_type == MEM_PF) || 
        (info->m_table_info->m_mem_type >= MEM_SWPREF_NTA && 
         info->m_table_info->m_mem_type <= MEM_SWPREF_T2)) {
      if (info->m_trace_info.m_second_mem)
        trace_uop->m_va = MIN2((pi->m_ld_vaddr2 + rep_offset)*amp_val, MAX_ADDR);
      else 
        trace_uop->m_va = MIN2((pi->m_ld_vaddr1 +  rep_offset)*amp_val, MAX_ADDR);

      trace_uop->m_mem_size = MIN2((pi->m_mem_read_size)*amp_val, REP_MOV_MEM_SIZE_MAX_NEW);
    }
  }

  // next pc
  trace_uop->m_npc = trace_uop->m_addr;
}


///////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Function to decode an instruction from the trace file into a sequence of uops
 * @param pi - raw trace format
 * @param trace_uop - micro uops storage for this instruction
 * @param core_id - core id
 * @param sim_thread_id - thread id  
 */
inst_info_s* cpu_decoder_c::convert_pinuop_to_t_uop(void *trace_info, trace_uop_s **trace_uop, 
    int core_id, int sim_thread_id)
{
  trace_info_cpu_s *pi = static_cast<trace_info_cpu_s *>(trace_info);
  core_c* core = m_simBase->m_core_pointers[core_id];

  // simulator maintains a cache of decoded instructions (uop) for each process, 
  // this avoids decoding of instructions everytime an instruction is executed
  int process_id = core->get_trace_info(sim_thread_id)->m_process->m_process_id;
  hash_c<inst_info_s>* htable = m_simBase->m_inst_info_hash[process_id];


  // since each instruction can be decoded into multiple uops, the key to the 
  // hashtable has to be (instruction addr + something else)
  // the instruction addr is shifted left by 3-bits and the number of the uop 
  // in the decoded sequence is added to the shifted value to obtain the key
  bool new_entry = false;
  Addr key_addr = (pi->m_instruction_addr << 3);


  // Get instruction information from the hash table if exists. 
  // Else create a new entry
  inst_info_s *info = htable->hash_table_access_create(key_addr, &new_entry);

  inst_info_s *first_info = info;
  int  num_uop = 0;
  int  dyn_uop_counter = 0;
  bool tmp_reg_needed = false;
  bool inst_has_ALU_uop = false;
  bool inst_has_ld_uop = false;
  int  ii, jj, kk;

  if (new_entry) {
    // Since we found a new instruction, we need to decode this instruction and store all
    // uops to the hash table
    int write_dest_reg = 0;

    trace_uop[0]->m_rep_uop_num = 0;
    trace_uop[0]->m_opcode = pi->m_opcode;

    // temporal register rules:
    // load->dest_reg (through tmp), load->store (through tmp), dest_reg->store (real reg)
    // load->cf (through tmp), dest_reg->cf (thought dest), st->cf (no dependency)
    

    ///
    /// 1. This instruction has a memory load operation
    ///
    if (pi->m_num_ld) {
      num_uop = 1;

      // set memory type
      switch (pi->m_opcode) {
        case XED_CATEGORY_PREFETCH:
          trace_uop[0]->m_mem_type = MEM_PF;
          break;
        default:
          trace_uop[0]->m_mem_type = MEM_LD;
          break;
      }
      
      // prefetch instruction
      if (pi->m_opcode == XED_CATEGORY_PREFETCH 
          || (pi->m_opcode >= PREFETCH_NTA && pi->m_opcode <= PREFETCH_T2)) {
        switch (pi->m_opcode) {
          case PREFETCH_NTA:
            trace_uop[0]->m_mem_type = MEM_SWPREF_NTA;
            break;

          case PREFETCH_T0:
            trace_uop[0]->m_mem_type = MEM_SWPREF_T0;
            break;

          case PREFETCH_T1:
            trace_uop[0]->m_mem_type = MEM_SWPREF_T1;
            break;

          case PREFETCH_T2:
            trace_uop[0]->m_mem_type = MEM_SWPREF_T2;
            break;
        }
      }
   

      trace_uop[0]->m_cf_type  = NOT_CF;
      trace_uop[0]->m_op_type  = (pi->m_is_fp) ? UOP_FMEM : UOP_IMEM;
      trace_uop[0]->m_bar_type = NOT_BAR;

      if (pi->m_opcode == XED_CATEGORY_DATAXFER) {
        trace_uop[0]->m_num_dest_regs = pi->m_num_dest_regs;
      }
      else {
        trace_uop[0]->m_num_dest_regs = 0;
      }

      trace_uop[0]->m_num_src_regs = pi->m_num_read_regs;
      trace_uop[0]->m_pin_2nd_mem  = 0;
      trace_uop[0]->m_eom      = 0;
      trace_uop[0]->m_alu_uop  = false;
      trace_uop[0]->m_inst_size = pi->m_size;

      // m_has_immediate is overloaded - in case of ptx simulations, for uncoalesced 
      // accesses, multiple memory access are generated and for each access there is
      // an instruction in the trace. the m_has_immediate flag is used to mark the
      // first and last accesses of an uncoalesced memory instruction
      trace_uop[0]->m_mul_mem_uops = pi->m_has_immediate; 


      ///
      /// There are two load operations in an instruction. Note that now array index becomes 1
      ///
      if (pi->m_num_ld == 2) {
        trace_uop[1]->m_opcode = pi->m_opcode;

        switch (pi->m_opcode) {
          case XED_CATEGORY_PREFETCH:
            trace_uop[1]->m_mem_type = MEM_PF;
            break;
          default:
            trace_uop[1]->m_mem_type = MEM_LD;
            break;
        }
        
        // prefetch instruction
        if (pi->m_opcode == XED_CATEGORY_PREFETCH || 
            (pi->m_opcode >= PREFETCH_NTA && pi->m_opcode <= PREFETCH_T2)) {
          switch (pi->m_opcode) {
            case PREFETCH_NTA:
              trace_uop[1]->m_mem_type = MEM_SWPREF_NTA;
              break;
            case PREFETCH_T0:
              trace_uop[1]->m_mem_type = MEM_SWPREF_T0;
              break;
            case PREFETCH_T1:
              trace_uop[1]->m_mem_type = MEM_SWPREF_T1;
              break;
            case PREFETCH_T2:
              trace_uop[1]->m_mem_type = MEM_SWPREF_T2;
              break;
          }
        }

        trace_uop[1]->m_cf_type    = NOT_CF;
        trace_uop[1]->m_op_type    = (pi->m_is_fp) ? UOP_FMEM : UOP_IMEM;
        trace_uop[1]->m_bar_type   = NOT_BAR;
        trace_uop[1]->m_num_dest_regs = 0;
        trace_uop[1]->m_num_src_regs = pi->m_num_read_regs;

        if (pi->m_opcode == XED_CATEGORY_DATAXFER) {
          trace_uop[1]->m_num_dest_regs = pi->m_num_dest_regs;
        }
        else {
          trace_uop[1]->m_num_dest_regs = 0;
        }

        trace_uop[1]->m_pin_2nd_mem  = 1;
        trace_uop[1]->m_eom          = 0;
        trace_uop[1]->m_alu_uop      = false;
        trace_uop[1]->m_inst_size    = pi->m_size;
        trace_uop[1]->m_mul_mem_uops = pi->m_has_immediate; // uncoalesced memory accesses

        num_uop = 2;
      } // NUM_LOAD == 2


      if (pi->m_opcode == XED_CATEGORY_DATAXFER) {
        write_dest_reg = 1;
      }

      if (trace_uop[0]->m_mem_type == MEM_LD) {
        inst_has_ld_uop = true;
      }
    } // HAS_LOAD


    // Add one more uop when temporary register is required
    if (pi->m_num_dest_regs && !write_dest_reg) {
      trace_uop_s* cur_trace_uop = trace_uop[num_uop++];
      if (inst_has_ld_uop) {
       tmp_reg_needed = true;
      }
   
      cur_trace_uop->m_opcode        = pi->m_opcode;
      cur_trace_uop->m_mem_type      = NOT_MEM;
      cur_trace_uop->m_cf_type       = NOT_CF;
      cur_trace_uop->m_op_type       = (Uop_Type)((pi->m_is_fp) ? 
          m_fp_uop_table[pi->m_opcode] : 
          m_int_uop_table[pi->m_opcode]);
      cur_trace_uop->m_bar_type      = NOT_BAR;
      cur_trace_uop->m_num_src_regs  = pi->m_num_read_regs;
      cur_trace_uop->m_num_dest_regs = pi->m_num_dest_regs;
      cur_trace_uop->m_pin_2nd_mem   = 0;
      cur_trace_uop->m_eom           = 0;
      cur_trace_uop->m_alu_uop       = true;

      inst_has_ALU_uop = true;
    }


    ///
    /// 2. Instruction has a memory store operation
    ///
    if (pi->m_has_st) {
      trace_uop_s* cur_trace_uop = trace_uop[num_uop++];
      if (inst_has_ld_uop) 
        tmp_reg_needed = true;
   
      cur_trace_uop->m_mem_type      = MEM_ST;
      cur_trace_uop->m_opcode        = pi->m_opcode;
      cur_trace_uop->m_cf_type       = NOT_CF;
      cur_trace_uop->m_op_type       = (pi->m_is_fp) ? UOP_FMEM : UOP_IMEM;
      cur_trace_uop->m_bar_type      = NOT_BAR;
      cur_trace_uop->m_num_src_regs  = pi->m_num_read_regs;
      cur_trace_uop->m_num_dest_regs = 0;
      cur_trace_uop->m_pin_2nd_mem   = 0;
      cur_trace_uop->m_eom           = 0;
      cur_trace_uop->m_alu_uop       = false;
      cur_trace_uop->m_inst_size     = pi->m_size;
      cur_trace_uop->m_mul_mem_uops  = pi->m_has_immediate; // uncoalesced memory accesses
    }


    ///
    /// 3. Instruction has a branch operation
    ///
    if (pi->m_cf_type) {
      trace_uop_s* cur_trace_uop = trace_uop[num_uop++];

      if (inst_has_ld_uop)
        tmp_reg_needed = true;
   
      cur_trace_uop->m_mem_type      = NOT_MEM;
      cur_trace_uop->m_cf_type       = (Cf_Type)((pi->m_cf_type >= PIN_CF_SYS) ? 
          CF_ICO : pi->m_cf_type);
      cur_trace_uop->m_op_type       = UOP_CF;
      cur_trace_uop->m_bar_type      = NOT_BAR;
      cur_trace_uop->m_num_src_regs  = pi->m_num_read_regs;
      cur_trace_uop->m_num_dest_regs = 0;
      cur_trace_uop->m_pin_2nd_mem   = 0;
      cur_trace_uop->m_eom           = 0;
      cur_trace_uop->m_alu_uop       = false;
      cur_trace_uop->m_inst_size     = pi->m_size;
    }

    // Fence instruction: m_opcode == MISC && actually_taken == 1
    if (num_uop == 0 && pi->m_opcode == XED_CATEGORY_MISC && pi->m_ld_vaddr2 == 1) {
      trace_uop[0]->m_opcode        = pi->m_opcode;
      trace_uop[0]->m_mem_type      = NOT_MEM;
      trace_uop[0]->m_cf_type       = NOT_CF;
      trace_uop[0]->m_op_type       = UOP_FULL_FENCE;
      trace_uop[0]->m_bar_type      = NOT_BAR;
      trace_uop[0]->m_num_dest_regs = 0;
      trace_uop[0]->m_num_src_regs  = 0;
      trace_uop[0]->m_pin_2nd_mem   = 0;
      trace_uop[0]->m_eom           = 1;
      trace_uop[0]->m_inst_size     = pi->m_size;
      ++num_uop;
    }

    ///
    /// Non-memory, non-branch instruction
    ///
    if (num_uop == 0) {
      trace_uop[0]->m_opcode        = pi->m_opcode;
      trace_uop[0]->m_mem_type      = NOT_MEM;
      trace_uop[0]->m_cf_type       = NOT_CF;
      trace_uop[0]->m_op_type       = UOP_NOP;
      trace_uop[0]->m_bar_type      = NOT_BAR;
      trace_uop[0]->m_num_dest_regs = 0;
      trace_uop[0]->m_num_src_regs  = 0;
      trace_uop[0]->m_pin_2nd_mem   = 0;
      trace_uop[0]->m_eom           = 1;
      trace_uop[0]->m_inst_size     = pi->m_size;
      ++num_uop;
    }


    info->m_trace_info.m_bom     = true;
    info->m_trace_info.m_eom     = false;
    info->m_trace_info.m_num_uop = num_uop;


    ///
    /// Process each static uop to dynamic uop
    ///
    for (ii = 0; ii < num_uop; ++ii) {
      // For the first uop, we have already created hash entry. However, for following uops
      // we need to create hash entries
      if (ii > 0) {
        key_addr                 = ((pi->m_instruction_addr << 3) + ii);
        info                     = htable->hash_table_access_create(key_addr, &new_entry);
        info->m_trace_info.m_bom = false;
        info->m_trace_info.m_eom = false;
      }
      ASSERTM(new_entry, "Add new uops to hash_table for core id::%d\n", core_id);

      trace_uop[ii]->m_addr = pi->m_instruction_addr;

      DEBUG_CORE(core_id, "pi->instruction_addr:0x%llx trace_uop[%d]->addr:0x%llx num_src_regs:%d num_read_regs:%d "
          "pi:num_dst_regs:%d uop:num_dst_regs:%d \n", (Addr)(pi->m_instruction_addr), ii, trace_uop[ii]->m_addr, 
          trace_uop[ii]->m_num_src_regs, pi->m_num_read_regs, pi->m_num_dest_regs, trace_uop[ii]->m_num_dest_regs);

      // set source register
      for (jj = 0; jj < trace_uop[ii]->m_num_src_regs; ++jj) {
        (trace_uop[ii])->m_srcs[jj].m_type = (Reg_Type)0;
        (trace_uop[ii])->m_srcs[jj].m_id   = pi->m_src[jj];
        (trace_uop[ii])->m_srcs[jj].m_reg  = pi->m_src[jj];
      }

      // store or control flow has a dependency whoever the last one
      if ((trace_uop[ii]->m_mem_type == MEM_ST) || 
          (trace_uop[ii]->m_cf_type != NOT_CF)) {

        if (tmp_reg_needed && !inst_has_ALU_uop) {
          (trace_uop[ii])->m_srcs[jj].m_type = (Reg_Type)0;
          (trace_uop[ii])->m_srcs[jj].m_id  = TR_REG_TMP0;
          (trace_uop[ii])->m_srcs[jj].m_reg  = TR_REG_TMP0;
          trace_uop[ii]->m_num_src_regs += 1;
        } 
        else if (inst_has_ALU_uop) {
          for (kk = 0; kk < pi->m_num_dest_regs; ++kk) {
            (trace_uop[ii])->m_srcs[jj+kk].m_type = (Reg_Type)0;
            (trace_uop[ii])->m_srcs[jj+kk].m_id   = pi->m_dst[kk];
            (trace_uop[ii])->m_srcs[jj+kk].m_reg  = pi->m_dst[kk];
          }

          trace_uop[ii]->m_num_src_regs += pi->m_num_dest_regs;
        }
      }

      // alu uop only has a dependency with a temp register
      if (trace_uop[ii]->m_alu_uop) {
        if (tmp_reg_needed) {
          (trace_uop[ii])->m_srcs[jj].m_type = (Reg_Type)0;
          (trace_uop[ii])->m_srcs[jj].m_id  = TR_REG_TMP0;
          (trace_uop[ii])->m_srcs[jj].m_reg  = TR_REG_TMP0;
          trace_uop[ii]->m_num_src_regs     += 1;
        }
      }

      for (jj = 0; jj < trace_uop[ii]->m_num_dest_regs; ++jj) {
        (trace_uop[ii])->m_dests[jj].m_type = (Reg_Type)0;
        (trace_uop[ii])->m_dests[jj].m_id   = pi->m_dst[jj];
        (trace_uop[ii])->m_dests[jj].m_reg = pi->m_dst[jj];
      }

      // add tmp register as a destination register
      if (tmp_reg_needed && trace_uop[ii]->m_mem_type == MEM_LD) { 
        (trace_uop[ii])->m_dests[jj].m_type = (Reg_Type)0;
        (trace_uop[ii])->m_dests[jj].m_id   = TR_REG_TMP0;
        (trace_uop[ii])->m_dests[jj].m_reg = TR_REG_TMP0;
        trace_uop[ii]->m_num_dest_regs     += 1;
      }


      // the last uop
      if (ii == (num_uop - 1) && trace_uop[num_uop-1]->m_mem_type == NOT_MEM) {
        /* last uop's info */
        if ((pi->m_opcode == XED_CATEGORY_SEMAPHORE) ||
            (pi->m_opcode == XED_CATEGORY_IO) ||
            (pi->m_opcode == XED_CATEGORY_INTERRUPT) ||
            (pi->m_opcode == XED_CATEGORY_SYSTEM) ||
            (pi->m_opcode == XED_CATEGORY_SYSCALL) ||
            (pi->m_opcode == XED_CATEGORY_SYSRET)) {
          // only the last instruction will have bar type
          trace_uop[(num_uop-1)]->m_bar_type = BAR_FETCH;
          trace_uop[(num_uop-1)]->m_cf_type  = CF_ICO;
        }
      }

      // update instruction information with MacSim trace
      convert_t_uop_to_info(trace_uop[ii], info);

      DEBUG_CORE(core_id, "tuop: pc 0x%llx num_src_reg:%d num_dest_reg:%d \n", trace_uop[ii]->m_addr, 
          trace_uop[ii]->m_num_src_regs, trace_uop[ii]->m_num_dest_regs);
      
      trace_uop[ii]->m_info = info;

      // Add dynamic information to the uop
      convert_dyn_uop(info, pi, trace_uop[ii], 0, core_id);
    }
  
    // set end of macro flag to the last uop
    trace_uop[num_uop - 1]->m_eom = 1;

    ASSERT(num_uop > 0);
  } // NEW_ENTRY
  ///
  /// Hash table already has matching instruction, we can skip above decoding process
  ///
  else {
    ASSERT(info);

    num_uop = info->m_trace_info.m_num_uop;
    for (ii = 0; ii < num_uop; ++ii) {
      if (ii > 0) {
        key_addr  = ((pi->m_instruction_addr << 3) + ii);
        info      = htable->hash_table_access_create(key_addr, &new_entry);
      }
      ASSERTM(!new_entry, "Core id %d index %d\n", core_id, ii);

      // convert raw instruction trace to MacSim trace format
      convert_info_uop(info, trace_uop[ii]);

      // add dynamic information
      convert_dyn_uop(info, pi, trace_uop[ii], 0, core_id);

      trace_uop[ii]->m_info   = info;
      trace_uop[ii]->m_eom    = 0;
      trace_uop[ii]->m_addr   = pi->m_instruction_addr;
      trace_uop[ii]->m_opcode = pi->m_opcode;
    }

    // set end of macro flag to the last uop
    trace_uop[num_uop-1]->m_eom = 1;

    ASSERT(num_uop > 0);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////
  // end of instruction decoding
  /////////////////////////////////////////////////////////////////////////////////////////////

  dyn_uop_counter = num_uop;

  ///
  /// Repeat move
  ///
  if (pi->m_opcode == XED_CATEGORY_STRINGOP) {
    int rep_counter = 1;
    int rep_dir     = 0;

    // generate multiple uops with different memory addresses
    key_addr  = ((pi->m_instruction_addr << 3));
    info      = htable->hash_table_access_create(key_addr, &new_entry);

    int rep_mem_size = (int) pi->m_mem_read_size;

    if (rep_mem_size == 0) {
      trace_uop[0]->m_mem_type      = NOT_MEM;
      trace_uop[0]->m_cf_type       = NOT_CF;
      trace_uop[0]->m_op_type       = UOP_NOP;
      trace_uop[0]->m_bar_type      = NOT_BAR;
      trace_uop[0]->m_num_dest_regs = 0;
      trace_uop[0]->m_num_src_regs  = 0;
      trace_uop[0]->m_pin_2nd_mem   = 0;
      trace_uop[0]->m_eom           = 1;
      trace_uop[0]->m_inst_size     = pi->m_size;
      
      num_uop = 1;

      // update instruction information with MacSim trace
      convert_t_uop_to_info(trace_uop[0], info);

      trace_uop[0]->m_info = info;

      // add dynamic information
      convert_dyn_uop(info, pi, trace_uop[ii], 0, core_id);

      dyn_uop_counter      = 1;
      trace_uop[0]->m_eom  = 1;
      trace_uop[0]->m_addr = pi->m_instruction_addr;
    } 
    else {
      rep_mem_size -= REP_MOV_MEM_SIZE_MAX_NEW;
      rep_dir       = pi->m_rep_dir;

      while ((rep_mem_size > 0 ) && (dyn_uop_counter < MAX_PUP)) {
        int rep_offset = (rep_dir == 1) ? 
          (REP_MOV_MEM_SIZE_MAX_NEW * (rep_counter) * (-1)) : 
          (REP_MOV_MEM_SIZE_MAX_NEW * (rep_counter) * (1));

        trace_uop[dyn_uop_counter-1]->m_npc = pi->m_instruction_addr;

        ASSERT(num_uop < 8);
        for (ii = 0; ii < num_uop; ++ii) {
          // can't skip when ii = 0; because this routine is repeating ...
          key_addr  = ((pi->m_instruction_addr << 3) + ii);
          info      = htable->hash_table_access_create(key_addr, &new_entry);

          info->m_trace_info.m_bom = false;
          info->m_trace_info.m_eom = false;

          ASSERT(!new_entry);

          // add dynamic information
          convert_dyn_uop(info, pi, trace_uop[dyn_uop_counter], rep_offset, core_id);

          trace_uop[dyn_uop_counter]->m_mem_size = MIN2(rep_mem_size, REP_MOV_MEM_SIZE_MAX_NEW);
          trace_uop[dyn_uop_counter]->m_info     = info;
          trace_uop[dyn_uop_counter]->m_eom      = 0;
          trace_uop[dyn_uop_counter]->m_addr     = pi->m_instruction_addr;
          ++dyn_uop_counter;
        }

        rep_mem_size -= REP_MOV_MEM_SIZE_MAX_NEW;
        ++rep_counter;
      }

      ASSERT(dyn_uop_counter > 0);
    }

    trace_uop[0]->m_rep_uop_num = dyn_uop_counter;
  } // XED_CATEGORY_STRINGOP

  ASSERT(dyn_uop_counter);


  // set eom flag and next pc address for the last uop of this instruction
  trace_uop[dyn_uop_counter-1]->m_eom = 1;
  trace_uop[dyn_uop_counter-1]->m_npc = pi->m_instruction_next_addr;

  STAT_CORE_EVENT(core_id, OP_CAT_XED_CATEGORY_INVALID + (pi->m_opcode)); 

  switch (pi->m_opcode) {
    case XED_CATEGORY_INTERRUPT:
    case XED_CATEGORY_IO:
    case XED_CATEGORY_IOSTRINGOP:
    case XED_CATEGORY_MISC:
    case XED_CATEGORY_RET:
    case XED_CATEGORY_ROTATE:
    case XED_CATEGORY_SEMAPHORE:
    case XED_CATEGORY_SYSCALL:
    case XED_CATEGORY_SYSRET:
    case XED_CATEGORY_SYSTEM:
    case XED_CATEGORY_VTX:
    case XED_CATEGORY_XSAVE:
      POWER_CORE_EVENT(core_id, POWER_CONTROL_REGISTER_W);
      break;

  }

  if (pi->m_num_ld || pi->m_has_st) {
    POWER_CORE_EVENT(core_id, POWER_SEGMENT_REGISTER_W);
  }

  if (pi->m_write_flg) {
    POWER_CORE_EVENT(core_id, POWER_FLAG_REGISTER_W);
  }

  ASSERT(num_uop > 0);
  first_info->m_trace_info.m_num_uop = num_uop;

  return first_info;
}

inst_info_s* cpu_decoder_c::get_inst_info(thread_s *thread_trace_info, int core_id, int sim_thread_id)
{
  trace_info_cpu_s trace_info;
  inst_info_s *info;
  // Copy current instruction to data structure
  memcpy(&trace_info, thread_trace_info->m_prev_trace_info, sizeof(trace_info_cpu_s));

  // Set next pc address
  trace_info_cpu_s *next_trace_info = static_cast<trace_info_cpu_s *>(thread_trace_info->m_next_trace_info);
  trace_info.m_instruction_next_addr = next_trace_info->m_instruction_addr;

  // Copy next instruction to current instruction field
  memcpy(thread_trace_info->m_prev_trace_info, thread_trace_info->m_next_trace_info, 
      sizeof(trace_info_cpu_s));

  DEBUG_CORE(core_id, "trace_read core_id:%d thread_id:%d pc:0x%llx opcode:%d inst_count:%llu\n", core_id, sim_thread_id, 
      (Addr)(trace_info.m_instruction_addr), static_cast<int>(trace_info.m_opcode), (Counter)(thread_trace_info->m_temp_inst_count));

  // So far we have raw instruction format, so we need to MacSim specific trace format
  info = convert_pinuop_to_t_uop(&trace_info, thread_trace_info->m_trace_uop_array, 
      core_id, sim_thread_id);

  return info;
}

///////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Get an uop from trace
 * Called by frontend.cc
 * @param core_id - core id
 * @param uop - uop object to hold instruction information
 * @param sim_thread_id thread id
 */
bool cpu_decoder_c::get_uops_from_traces(int core_id, uop_c *uop, int sim_thread_id)
{
  // use an alternative uop fetch function, if hmc is enabled  
  if (*KNOB(KNOB_ENABLE_HMC_INST))
  {
    return hmc_function_c::get_uops_from_traces_with_hmc_inst(this, core_id, uop, sim_thread_id);
  }

  ASSERT(uop);

  trace_uop_s *trace_uop;
  int num_uop  = 0;
  core_c* core = m_simBase->m_core_pointers[core_id];
  inst_info_s *info;

  // fetch ended : no uop to fetch
  if (core->m_fetch_ended[sim_thread_id]) 
    return false;

  // uop number is specific to the core
  uop->m_unique_num = core->inc_and_get_unique_uop_num();

  bool read_success = true;
  thread_s* thread_trace_info = core->get_trace_info(sim_thread_id);

  if (thread_trace_info->m_thread_init) {
    thread_trace_info->m_thread_init = false;
  }


  ///
  /// BOM (beginning of macro) : need to get a next instruction
  ///
  if (thread_trace_info->m_bom) {
    bool inst_read; // indicate new instruction has been read from a trace file
    
    if (core->m_inst_fetched[sim_thread_id] < *KNOB(KNOB_MAX_INSTS)) {
      // read next instruction
      read_success = read_trace(core_id, thread_trace_info->m_next_trace_info, 
          sim_thread_id, &inst_read);
    }
    else {
      inst_read = false;
      if (!core->get_trace_info(sim_thread_id)->m_trace_ended) { 
        core->get_trace_info(sim_thread_id)->m_trace_ended = true;
      }
    }

    info = get_inst_info(thread_trace_info, core_id, sim_thread_id);

    ///
    /// Trace read failed
    ///
    if (!read_success) 
      return false;


    // read a new instruction, so update stats
    if (inst_read) { 
      ++core->m_inst_fetched[sim_thread_id];
      DEBUG_CORE(core_id, "core_id:%d thread_id:%d inst_num:%llu\n", core_id, sim_thread_id, 
          (Counter)(thread_trace_info->m_temp_inst_count + 1));

      if (core->m_inst_fetched[sim_thread_id] > core->m_max_inst_fetched) 
        core->m_max_inst_fetched = core->m_inst_fetched[sim_thread_id];
    }



    trace_uop = thread_trace_info->m_trace_uop_array[0];
    num_uop   = info->m_trace_info.m_num_uop;
    ASSERT(info->m_trace_info.m_num_uop > 0);

    thread_trace_info->m_num_sending_uop = 1;
    thread_trace_info->m_eom             = thread_trace_info->m_trace_uop_array[0]->m_eom;
    thread_trace_info->m_bom             = false;

    uop->m_isitBOM = true;
    POWER_CORE_EVENT(core_id, POWER_INST_DECODER_R);
    POWER_CORE_EVENT(core_id, POWER_OPERAND_DECODER_R);
  } // END EOM
  // read remaining uops from the same instruction
  else { 
    trace_uop                = 
      thread_trace_info->m_trace_uop_array[thread_trace_info->m_num_sending_uop];
    info                     = trace_uop->m_info;
    thread_trace_info->m_eom = trace_uop->m_eom;
    info->m_trace_info.m_bom = 0; // because of repeat instructions ....
    uop->m_isitBOM           = false;
    ++thread_trace_info->m_num_sending_uop;
  }


  // set end of macro flag
  if (thread_trace_info->m_eom) {
    uop->m_isitEOM           = true; // mark for current uop
    thread_trace_info->m_bom = true; // mark for next instruction
  }
  else {
    uop->m_isitEOM           = false;
    thread_trace_info->m_bom = false;
  }


  if (core->get_trace_info(sim_thread_id)->m_trace_ended && uop->m_isitEOM) {
    --core->m_fetching_thread_num;
    core->m_fetch_ended[sim_thread_id] = true;
    uop->m_last_uop                    = true;
    DEBUG_CORE(core_id, "core_id:%d thread_id:%d inst_num:%lld uop_num:%lld fetched:%lld last uop\n",
        core_id, sim_thread_id, uop->m_inst_num, uop->m_uop_num, core->m_inst_fetched[sim_thread_id]);
  }


  ///
  /// Set up actual uop data structure
  ///
  uop->m_opcode      = trace_uop->m_opcode;
  uop->m_uop_type    = info->m_table_info->m_op_type;
  uop->m_cf_type     = info->m_table_info->m_cf_type;
  uop->m_mem_type    = info->m_table_info->m_mem_type;
  ASSERT(uop->m_mem_type >= 0 && uop->m_mem_type < NUM_MEM_TYPES);
  uop->m_bar_type    = trace_uop->m_bar_type;
  uop->m_npc         = trace_uop->m_npc;
  uop->m_active_mask = trace_uop->m_active_mask;

  if (uop->m_cf_type) { 
    uop->m_taken_mask      = trace_uop->m_taken_mask;
    uop->m_reconverge_addr = trace_uop->m_reconverge_addr;
    uop->m_target_addr     = trace_uop->m_target;
  }

  if (uop->m_opcode == GPU_EN) {
    m_simBase->m_gpu_paused = false;	
  }

  // address translation
  if (trace_uop->m_va == 0) {
    uop->m_vaddr = 0;
  } 
  else {
    // since we can have 64-bit address space and each trace has 32-bit address,
    // using extra bits to differentiate address space of each application
    uop->m_vaddr = trace_uop->m_va + m_simBase->m_memory->base_addr(core_id,
        (unsigned long)UINT_MAX * 
        (core->get_trace_info(sim_thread_id)->m_process->m_process_id) * 10ul);

    // virtual-to-physical translation 
    // physical page is allocated at this point for the time being
    if (m_enable_physical_mapping)
      uop->m_vaddr = m_page_mapper->translate(uop->m_vaddr);
  }


  uop->m_mem_size = trace_uop->m_mem_size;
  uop->m_dir      = trace_uop->m_actual_taken;
  uop->m_pc       = info->m_addr;
  uop->m_core_id  = core_id;

  if (uop->m_mem_type != NOT_MEM) {
    int temp_num_req = (uop->m_mem_size + *KNOB(KNOB_MAX_TRANSACTION_SIZE) - 1) / 
      *KNOB(KNOB_MAX_TRANSACTION_SIZE);

    ASSERTM(temp_num_req > 0, "pc:%llx vaddr:%llx opcode:%d size:%d max:%d num:%d type:%d num:%d\n", 
        uop->m_pc, uop->m_vaddr, uop->m_opcode, uop->m_mem_size, 
        (int)*KNOB(KNOB_MAX_TRANSACTION_SIZE), temp_num_req, uop->m_mem_type, 
        trace_uop->m_info->m_trace_info.m_num_uop);
  }

  // we found first uop of an instruction, so add instruction count
  if (uop->m_isitBOM) 
    ++thread_trace_info->m_temp_inst_count;

  uop->m_inst_num  = thread_trace_info->m_temp_inst_count;
  uop->m_num_srcs  = trace_uop->m_num_src_regs;
  uop->m_num_dests = trace_uop->m_num_dest_regs;

  ASSERTM(uop->m_num_dests < MAX_DST_NUM, "uop->num_dests=%d MAX_DST_NUM=%d\n", 
      uop->m_num_dests, MAX_DST_NUM);

  DEBUG_CORE(uop->m_core_id, "uop_num:%llu num_srcs:%d  trace_uop->num_src_regs:%d  num_dsts:%d num_seing_uop:%d "
      "pc:0x%llx dir:%d \n", uop->m_uop_num, uop->m_num_srcs, trace_uop->m_num_src_regs, uop->m_num_dests, 
      thread_trace_info->m_num_sending_uop, uop->m_pc, uop->m_dir);

  // filling the src_info, dest_info
  if (uop->m_num_srcs < MAX_SRCS) {
    for (int index=0; index < uop->m_num_srcs; ++index) {
      uop->m_src_info[index] = trace_uop->m_srcs[index].m_id;
      //DEBUG("uop_num:%lld src_info[%d]:%d\n", uop->uop_num, index, uop->src_info[index]);
    }
  } 
  else {
    ASSERTM(uop->m_num_srcs < MAX_SRCS, "src_num:%d MAX_SRC:%d", uop->m_num_srcs, MAX_SRCS);
  }



  for (int index = 0; index < uop->m_num_dests; ++index) {
    uop->m_dest_info[index] = trace_uop->m_dests[index].m_id;
    ASSERT(trace_uop->m_dests[index].m_reg < NUM_REG_IDS);
  }

  uop->m_uop_num          = (thread_trace_info->m_temp_uop_count++);
  uop->m_thread_id        = sim_thread_id;
  uop->m_block_id         = ((core)->get_trace_info(sim_thread_id))->m_block_id; 
  uop->m_orig_block_id    = ((core)->get_trace_info(sim_thread_id))->m_orig_block_id;
  uop->m_unique_thread_id = ((core)->get_trace_info(sim_thread_id))->m_unique_thread_id;
  uop->m_orig_thread_id   = ((core)->get_trace_info(sim_thread_id))->m_orig_thread_id;

  
  ///
  /// GPU simulation : handling uncoalesced accesses
  /// removed
  ///

  DEBUG_CORE(uop->m_core_id, "new uop: uop_num:%lld inst_num:%lld thread_id:%d unique_num:%lld \n",
      uop->m_uop_num, uop->m_inst_num, uop->m_thread_id, uop->m_unique_num);

  return read_success;
}


///////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Initialize the mapping between trace opcode and uop type
 */
void cpu_decoder_c::init_pin_convert(void)
{
  m_int_uop_table[XED_CATEGORY_INVALID]     = UOP_INV;
  m_int_uop_table[XED_CATEGORY_3DNOW]       = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_AES]         = UOP_IMUL;
  m_int_uop_table[XED_CATEGORY_AVX]         = UOP_FADD;
  m_int_uop_table[XED_CATEGORY_AVX2]        = UOP_FADD; // new
  m_int_uop_table[XED_CATEGORY_AVX2GATHER]  = UOP_FADD; // new
  m_int_uop_table[XED_CATEGORY_BDW]         = UOP_FADD; // new
  m_int_uop_table[XED_CATEGORY_BINARY]      = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_BITBYTE]     = UOP_BYTE;
  m_int_uop_table[XED_CATEGORY_BMI1]        = UOP_BYTE; // new
  m_int_uop_table[XED_CATEGORY_BMI2]        = UOP_BYTE; // new
  m_int_uop_table[XED_CATEGORY_BROADCAST]   = UOP_IADD; // tocheck
  m_int_uop_table[XED_CATEGORY_CALL]        = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_CMOV]        = UOP_CMOV;
  m_int_uop_table[XED_CATEGORY_COND_BR]     = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_CONVERT]     = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_DATAXFER]    = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_DECIMAL]     = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_FCMOV]       = UOP_FADD;
  m_int_uop_table[XED_CATEGORY_FLAGOP]      = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_FMA4]        = UOP_IADD; // new
  m_int_uop_table[XED_CATEGORY_INTERRUPT]   = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_IO]          = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_IOSTRINGOP]  = UOP_IMUL;
  m_int_uop_table[XED_CATEGORY_LOGICAL]     = UOP_LOGIC;
  m_int_uop_table[XED_CATEGORY_LZCNT]       = UOP_LOGIC; // new
  m_int_uop_table[XED_CATEGORY_MISC]        = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_MMX]         = UOP_FADD;
  m_int_uop_table[XED_CATEGORY_NOP]         = UOP_NOP;
  m_int_uop_table[XED_CATEGORY_PCLMULQDQ]   = UOP_IMUL;
  m_int_uop_table[XED_CATEGORY_POP]         = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_PREFETCH]    = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_PUSH]        = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_RDRAND]      = UOP_IADD; // new
  m_int_uop_table[XED_CATEGORY_RDSEED]      = UOP_IADD; // new
  m_int_uop_table[XED_CATEGORY_RDWRFSGS]    = UOP_IADD; // new
  m_int_uop_table[XED_CATEGORY_RET]         = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_ROTATE]      = UOP_SHIFT;
  m_int_uop_table[XED_CATEGORY_SEGOP]       = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_SEMAPHORE]   = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_SHIFT]       = UOP_SHIFT;
  m_int_uop_table[XED_CATEGORY_SSE]         = UOP_FADD;
  m_int_uop_table[XED_CATEGORY_STRINGOP]    = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_STTNI]       = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_SYSCALL]     = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_SYSRET]      = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_SYSTEM]      = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_TBM]         = UOP_IADD; // new
  m_int_uop_table[XED_CATEGORY_UNCOND_BR]   = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_VFMA]        = UOP_IADD; // new
  m_int_uop_table[XED_CATEGORY_VTX]         = UOP_IADD;
  m_int_uop_table[XED_CATEGORY_WIDENOP]     = UOP_NOP;
  m_int_uop_table[XED_CATEGORY_X87_ALU]     = UOP_FADD;
  m_int_uop_table[XED_CATEGORY_XSAVE]       = UOP_IMUL;
  m_int_uop_table[XED_CATEGORY_XSAVEOPT]    = UOP_IMUL;
  m_int_uop_table[TR_MUL]                   = UOP_IMUL;
  m_int_uop_table[TR_DIV]                   = UOP_IMUL;
  m_int_uop_table[TR_FMUL]                  = UOP_FMUL;
  m_int_uop_table[TR_FDIV]                  = UOP_FDIV;
  m_int_uop_table[TR_NOP]                   = UOP_NOP;
  m_int_uop_table[PREFETCH_NTA]             = UOP_IADD;
  m_int_uop_table[PREFETCH_T0]              = UOP_IADD;
  m_int_uop_table[PREFETCH_T1]              = UOP_IADD;
  m_int_uop_table[PREFETCH_T2]              = UOP_IADD;


  m_fp_uop_table[XED_CATEGORY_INVALID]     = UOP_INV;
  m_fp_uop_table[XED_CATEGORY_3DNOW]       = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_AES]         = UOP_FMUL;
  m_fp_uop_table[XED_CATEGORY_AVX]         = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_AVX2]        = UOP_FADD; // new
  m_fp_uop_table[XED_CATEGORY_AVX2GATHER]  = UOP_FADD; // new
  m_fp_uop_table[XED_CATEGORY_BDW]         = UOP_FADD; // new
  m_fp_uop_table[XED_CATEGORY_BINARY]      = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_BITBYTE]     = UOP_BYTE;
  m_fp_uop_table[XED_CATEGORY_BMI1]        = UOP_BYTE; // new
  m_fp_uop_table[XED_CATEGORY_BMI2]        = UOP_BYTE; // new
  m_fp_uop_table[XED_CATEGORY_BROADCAST]   = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_CALL]        = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_CMOV]        = UOP_CMOV;
  m_fp_uop_table[XED_CATEGORY_COND_BR]     = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_CONVERT]     = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_DATAXFER]    = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_DECIMAL]     = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_FCMOV]       = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_FLAGOP]      = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_FMA4]        = UOP_IADD; // new
  m_fp_uop_table[XED_CATEGORY_INTERRUPT]   = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_IO]          = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_IOSTRINGOP]  = UOP_FMUL;
  m_fp_uop_table[XED_CATEGORY_LOGICAL]     = UOP_LOGIC;
  m_fp_uop_table[XED_CATEGORY_LZCNT]       = UOP_LOGIC; // new
  m_fp_uop_table[XED_CATEGORY_MISC]        = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_MMX]         = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_NOP]         = UOP_NOP;
  m_fp_uop_table[XED_CATEGORY_PCLMULQDQ]   = UOP_FMUL;
  m_fp_uop_table[XED_CATEGORY_POP]         = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_PREFETCH]    = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_PUSH]        = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_RDRAND]      = UOP_IADD; // new
  m_fp_uop_table[XED_CATEGORY_RDSEED]      = UOP_IADD; // new
  m_fp_uop_table[XED_CATEGORY_RDWRFSGS]    = UOP_IADD; // new
  m_fp_uop_table[XED_CATEGORY_RET]         = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_ROTATE]      = UOP_SHIFT;
  m_fp_uop_table[XED_CATEGORY_SEGOP]       = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_SEMAPHORE]   = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_SHIFT]       = UOP_SHIFT;
  m_fp_uop_table[XED_CATEGORY_SSE]         = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_STRINGOP]    = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_STTNI]       = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_SYSCALL]     = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_SYSRET]      = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_SYSTEM]      = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_TBM]         = UOP_IADD; // new
  m_fp_uop_table[XED_CATEGORY_UNCOND_BR]   = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_VFMA]        = UOP_IADD; // new
  m_fp_uop_table[XED_CATEGORY_VTX]         = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_WIDENOP]     = UOP_NOP;
  m_fp_uop_table[XED_CATEGORY_X87_ALU]     = UOP_FADD;
  m_fp_uop_table[XED_CATEGORY_XSAVE]       = UOP_FMUL;
  m_fp_uop_table[XED_CATEGORY_XSAVEOPT]    = UOP_FMUL;
  m_fp_uop_table[TR_MUL]                   = UOP_IMUL;
  m_fp_uop_table[TR_DIV]                   = UOP_IMUL;
  m_fp_uop_table[TR_FMUL]                  = UOP_FMUL;
  m_fp_uop_table[TR_FDIV]                  = UOP_FDIV;
  m_fp_uop_table[TR_NOP]                   = UOP_NOP;
  m_fp_uop_table[PREFETCH_NTA]             = UOP_FADD;
  m_fp_uop_table[PREFETCH_T0]              = UOP_FADD;
  m_fp_uop_table[PREFETCH_T1]              = UOP_FADD;
  m_fp_uop_table[PREFETCH_T2]              = UOP_FADD;
}


const char* cpu_decoder_c::g_tr_reg_names[MAX_TR_REG] = {
  "*invalid*",
  "*none*",
  "*imm8*",
  "*imm*",
  "*imm32*",
  "*mem*",
  "*mem*",
  "*mem*",
  "*off*",
  "*off*",
  "*off*",
  "*modx*",
  "rdi",
  "rsi",
  "rbp",
  "rsp",
  "rbx",
  "rdx",
  "rcx",
  "rax",
  "r8",
  "r9",
  "r10",
  "r11",
  "r12",
  "r13",
  "r14",
  "r15",
  "cs",
  "ss",
  "ds",
  "es",
  "fs",
  "gs",
  "rflags",
  "rip",
  "al",
  "ah",
  "ax",
  "cl",
  "ch",
  "cx",
  "dl",
  "dh",
  "dx",
  "bl",
  "bh",
  "bx",
  "bp",
  "si",
  "di",
  "sp",
  "flags",
  "ip",
  "edi",
  "dil",
  "esi",
  "sil",
  "ebp",
  "bpl",
  "esp",
  "spl",
  "ebx",
  "edx",
  "ecx",
  "eax",
  "eflags",
  "eip",
  "r8b",
  "r8w",
  "r8d",
  "r9b",
  "r9w",
  "r9d",
  "r10b",
  "r10w",
  "r10d",
  "r11b",
  "r11w",
  "r11d",
  "r12b",
  "r12w",
  "r12d",
  "r13b",
  "r13w",
  "r13d",
  "r14b",
  "r14w",
  "r14d",
  "r15b",
  "r15w",
  "r15d",
  "mm0",
  "mm1",
  "mm2",
  "mm3",
  "mm4",
  "mm5",
  "mm6",
  "mm7",
  "emm0",
  "emm1",
  "emm2",
  "emm3",
  "emm4",
  "emm5",
  "emm6",
  "emm7",
  "mxt",
  "x87", // new
  "xmm0",
  "xmm1",
  "xmm2",
  "xmm3",
  "xmm4",
  "xmm5",
  "xmm6",
  "xmm7",
  "xmm8",
  "xmm9",
  "xmm10",
  "xmm11",
  "xmm12",
  "xmm13",
  "xmm14",
  "xmm15",
  "ymm0",
  "ymm1",
  "ymm2",
  "ymm3",
  "ymm4",
  "ymm5",
  "ymm6",
  "ymm7",
  "ymm8",
  "ymm9",
  "ymm10",
  "ymm11",
  "ymm12",
  "ymm13",
  "ymm14",
  "ymm15",
  "mxcsr",
  "mxcsrmask",
  "orig_rax",
  "dr0",
  "dr1",
  "dr2",
  "dr3",
  "dr4",
  "dr5",
  "dr6",
  "dr7",
  "cr0",
  "cr1",
  "cr2",
  "cr3",
  "cr4",
  "tssr",
  "ldtr",
  "tr0",
  "tr1",
  "tr2",
  "tr3",
  "tr4",
  "tr5",
  "fpcw",
  "fpsw",
  "fptag",
  "fpip_off",
  "fpip_sel",
  "fpopcode",
  "fpdp_off",
  "fpdp_sel",
  "fptag_full",
  "st0",
  "st1",
  "st2",
  "st3",
  "st4",
  "st5",
  "st6",
  "st7",
  "r_status_flags",
  "rdf",
};



const char* cpu_decoder_c::g_tr_opcode_names[MAX_TR_OPCODE_NAME] = {
  "INVALID",
  "3DNOW",
  "AES",
  "AVX",
  "AVX2",
  "AVX2GATHER",
  "BDW",
  "BINARY",
  "BITBYTE",
  "BMI1",
  "BMI2",
  "BROADCAST",
  "CALL",
  "CMOV",
  "COND_BR",
  "CONVERT",
  "DATAXFER",
  "DECIMAL",
  "FCMOV",
  "FLAGOP",
  "FMA4",
  "INTERRUPT",
  "IO",
  "IOSTRINGOP",
  "LOGICAL",
  "LZCNT",
  "MISC",
  "MMX",
  "NOP",
  "PCLMULQDQ",
  "POP",
  "PREFETCH",
  "PUSH",
  "RDRAND",
  "RDSEED",
  "RDWRFSGS",
  "RET",
  "ROTATE",
  "SEGOP",
  "SEMAPHORE",
  "SHIFT",
  "SSE",
  "STRINGOP",
  "STTNI",
  "SYSCALL",
  "SYSRET",
  "SYSTEM",
  "TBM",
  "UNCOND_BR",
  "VFMA",
  "VTX",
  "WIDENOP",
  "X87_ALU",
  "XOP",
  "XSAVE",
  "XSAVEOPT",
  "TR_MUL",
  "TR_DIV",
  "TR_FMUL",
  "TR_FDIV",
  "TR_NOP",
  "PREFETCH_NTA",
  "PREFETCH_T0",
  "PREFETCH_T1",
  "PREFETCH_T2",
  "GPU_EN",
};


const char* cpu_decoder_c::g_tr_cf_names[10] = {
  "NOT_CF",       // not a control flow instruction
  "CF_BR",       // an unconditional branch
  "CF_CBR",       // a conditional branch
  "CF_CALL",      // a call
  "CF_IBR",       // an indirect branch
  "CF_ICALL",      // an indirect call
  "CF_ICO",       // an indirect jump to co-routine
  "CF_RET",       // a return
  "CF_SYS",
  "CF_ICBR"
};


const char *cpu_decoder_c::g_optype_names[37] = {
  "OP_INV",       // invalid opcode
  "OP_SPEC",      // something weird (rpcc)
  "OP_NOP",       // is a decoded nop
  "OP_CF",       // change of flow
  "OP_CMOV",      // conditional move
  "OP_LDA",         // load address
  "OP_IMEM",      // int memory instruction
  "OP_IADD",      // integer add
  "OP_IMUL",      // integer multiply
  "OP_ICMP",      // integer compare
  "OP_LOGIC",      // logical
  "OP_SHIFT",      // shift
  "OP_BYTE",      // byte manipulation
  "OP_MM",       // multimedia instructions
  "OP_FMEM",      // fp memory instruction
  "OP_FCF",
  "OP_FCVT",      // floating point convert
  "OP_FADD",      // floating point add
  "OP_FMUL",      // floating point multiply
  "OP_FDIV",      // floating point divide
  "OP_FCMP",      // floating point compare
  "OP_FBIT",      // floating point bit
  "OP_FCMOV"        // floating point cond move
};


const char *cpu_decoder_c::g_mem_type_names[20] = {
  "NOT_MEM",     // not a memory instruction
  "MEM_LD",       // a load instruction
  "MEM_ST",       // a store instruction
  "MEM_PF",       // a prefetch
  "MEM_WH",       // a write hint
  "MEM_EVICT",     // a cache block eviction hint
  "MEM_SWPREF_NTA", 
  "MEM_SWPREF_T0",
  "MEM_SWPREF_T1",
  "MEM_SWPREF_T2",
  "MEM_LD_LM",
  "MEM_LD_SM",
  "MEM_LD_GM",
  "MEM_ST_LM",
  "MEM_ST_SM",
  "MEM_ST_GM",
  "NUM_MEM_TYPES"
};


