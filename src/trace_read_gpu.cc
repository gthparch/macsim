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
 * File         : trace_read_gpu.cc
 * Author       : HPArch Research Group
 * Date         : 
 * SVN          : $Id: trace_read.cc 912 2009-11-20 19:09:21Z kacear $
 * Description  : Trace read and decode for CPU traces
 *********************************************************************************************/


#include <iostream>
#include <set>

#include "assert_macros.h"
#include "trace_read.h"
#include "trace_read_gpu.h"
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

#include "all_knobs.h"

///////////////////////////////////////////////////////////////////////////////////////////////


#define DEBUG(args...)   _DEBUG(*KNOB(KNOB_DEBUG_TRACE_READ), ## args)


///////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Constructor
 */
gpu_decoder_c::gpu_decoder_c(macsim_c* simBase, ofstream* m_dprint_output)
  : trace_read_c(simBase, m_dprint_output)
{
  m_trace_size = GPU_TRACE_SIZE;

  // map opcode type to uop type 
  init_pin_convert();
}


/**
 * Destructor
 */
gpu_decoder_c::~gpu_decoder_c()
{
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
bool gpu_decoder_c::peek_trace(int core_id, trace_info_s *trace_info, int sim_thread_id, 
    bool *inst_read)
{
  core_c *core                = m_simBase->m_core_pointers[core_id];
  int bytes_read              = -1;
  thread_s *thread_trace_info = core->get_trace_info(sim_thread_id);
  bool ret_val                = false;

  if (!thread_trace_info->m_buffer_exhausted) {
    memcpy(trace_info, 
        &thread_trace_info->m_buffer[m_trace_size * thread_trace_info->m_buffer_index], 
        m_trace_size); 
    thread_trace_info->m_buffer_index = 
      (thread_trace_info->m_buffer_index + 1) % k_trace_buffer_size;

    if (thread_trace_info->m_buffer_index >= thread_trace_info->m_buffer_index_max) {
      bytes_read = 0;
    }
    else {
      bytes_read = m_trace_size;
    }

    // we consume all trace buffer entries
    if (thread_trace_info->m_buffer_index == 0) {
      thread_trace_info->m_buffer_exhausted = true;
    }
  }
  // read one instruction each
  else {
    bytes_read = gzread(thread_trace_info->m_trace_file, trace_info, m_trace_size);
  }

  if (m_trace_size == bytes_read) {
    *inst_read = true;
    ret_val    = true;
  }
  else if (0 == bytes_read) {
    *inst_read = false;
    ret_val    = true;
  }
  else {
    *inst_read = false;
    ret_val    = false;
  }

  return ret_val;
}


/**
 * After peeking trace, in case of failture, we need to rewind trace file.
 * @param core_id - core id
 * @param sim_thread_id - thread id
 * @param num_inst - number of instructions to rewind
 * @see peek_trace
 */
bool gpu_decoder_c::ungetch_trace(int core_id, int sim_thread_id, int num_inst)
{
  core_c *core = m_simBase->m_core_pointers[core_id];
  thread_s *thread_trace_info = core->get_trace_info(sim_thread_id);

  // if we read instructions and store it in the buffer, reduce buffer index only
  if (thread_trace_info->m_buffer_index >= num_inst) {
    thread_trace_info->m_buffer_index -= num_inst;
    return true;
  }
  // more instructions to rewind.
  else if (thread_trace_info->m_buffer_index) {
    num_inst -= thread_trace_info->m_buffer_index;
    thread_trace_info->m_buffer_index = 0;
  }


  // rewind trace file
  off_t offset = gzseek(thread_trace_info->m_trace_file, -1*num_inst*m_trace_size, SEEK_CUR);

  if (offset == -1) {
    return false;
  }
  return true;
}


/**
 * Dump out instruction information to the file. At most 50000 instructions will be printed
 * @param t_info - trace information
 * @param core_id - core id
 * @param thread_id - thread id
 */
void gpu_decoder_c::dprint_inst(trace_info_s *t_info, int core_id, int thread_id) 
{
  if (m_dprint_count++ >= 50000 || !*KNOB(KNOB_DEBUG_PRINT_TRACE))
    return ;
 
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
void gpu_decoder_c::pre_read_trace(thread_s* trace_info)
{
  int bytes_read;
  trace_info_s inst_info;
  Section_Type_enum prev_type;
  Section_Type_enum cur_type;
  int32_t m_count;
  section_info_s *sec_info;

  Section_Type_enum bar_prev_type;
  Section_Type_enum bar_cur_type;
  int32_t bar_count;

  Section_Type_enum mem_prev_type;
  Section_Type_enum mem_cur_type;
  int32_t mem_count;

  Section_Type_enum mem_bar_prev_type;
  Section_Type_enum mem_bar_cur_type;
  int32_t mem_bar_count;

  int32_t mem_for_bar_count; 

  int32_t total_count;

  prev_type = bar_prev_type = mem_prev_type = mem_bar_prev_type = SECTION_INVALID;
  m_count = bar_count = mem_count = mem_bar_count = mem_for_bar_count = total_count = 0;


  while ((bytes_read = 
        gzread(trace_info->m_trace_file, &inst_info, m_trace_size)) == m_trace_size) {
    ++total_count;

    switch (inst_info.m_opcode) {
      case GPU_MEM_LD_LM:
      case GPU_MEM_LD_GM:
      case GPU_MEM_ST_LM:
      case GPU_MEM_ST_GM:
      case GPU_DATA_XFER_LM:
      case GPU_DATA_XFER_GM:
        cur_type = SECTION_MEM;
        bar_cur_type = SECTION_COMP;
        mem_cur_type = SECTION_MEM;
        mem_bar_cur_type = SECTION_MEM;

        ++mem_for_bar_count;

        break;
      default:
        cur_type = SECTION_COMP;
        bar_cur_type = SECTION_COMP;
        mem_cur_type = SECTION_COMP;
        mem_bar_cur_type = SECTION_COMP;

        break;
    }


    ++bar_count;
    if (bar_cur_type == SECTION_BAR) {
      sec_info = m_simBase->m_section_pool->acquire_entry();
      sec_info->m_section_type = SECTION_COMP;
      sec_info->m_section_length = bar_count;

      trace_info->m_bar_sections.push_back(sec_info);

      bar_count = 0;

      sec_info = m_simBase->m_section_pool->acquire_entry();
      sec_info->m_section_type = SECTION_MEM;
      sec_info->m_section_length = mem_for_bar_count;

      trace_info->m_mem_for_bar_sections.push_back(sec_info);

      mem_for_bar_count = 0;
    }

    ++mem_count;
    if (mem_cur_type == SECTION_MEM) {
      sec_info = m_simBase->m_section_pool->acquire_entry();
      sec_info->m_section_type = SECTION_COMP;
      sec_info->m_section_length = mem_count;

      trace_info->m_mem_sections.push_back(sec_info);

      mem_count = 0;
    }

    ++mem_bar_count;
    if (mem_bar_cur_type == SECTION_MEM || mem_bar_cur_type == SECTION_BAR) {
      sec_info = m_simBase->m_section_pool->acquire_entry();
      sec_info->m_section_type = SECTION_COMP;
      sec_info->m_section_length = mem_bar_count;

      trace_info->m_mem_bar_sections.push_back(sec_info);

      mem_bar_count = 0;
    }
  }

  if (total_count) {
    sec_info = m_simBase->m_section_pool->acquire_entry();
    sec_info->m_section_type = SECTION_COMP;
    sec_info->m_section_length = total_count;

    trace_info->m_sections.push_back(sec_info);
  }


  if (bar_count) {
    sec_info = m_simBase->m_section_pool->acquire_entry();
    sec_info->m_section_type = SECTION_COMP;
    sec_info->m_section_length = bar_count;

    trace_info->m_bar_sections.push_back(sec_info);

    sec_info = m_simBase->m_section_pool->acquire_entry();
    sec_info->m_section_type = SECTION_MEM;
    sec_info->m_section_length = mem_for_bar_count;

    trace_info->m_mem_for_bar_sections.push_back(sec_info);
  }


  if (mem_count) {
    sec_info = m_simBase->m_section_pool->acquire_entry();
    sec_info->m_section_type = SECTION_COMP;
    sec_info->m_section_length = mem_count;

    trace_info->m_mem_sections.push_back(sec_info);
  }


  if (mem_bar_count) {
    sec_info = m_simBase->m_section_pool->acquire_entry();
    sec_info->m_section_type = SECTION_COMP;
    sec_info->m_section_length = mem_bar_count;

    trace_info->m_mem_bar_sections.push_back(sec_info);
  }

  // rewind the trace file
  gzrewind(trace_info->m_trace_file);
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
void gpu_decoder_c::convert_dyn_uop(inst_info_s *info, trace_info_s *pi, trace_uop_s *trace_uop, 
    Addr rep_offset, int core_id)
{
  core_c* core    = m_simBase->m_core_pointers[core_id];
  trace_uop->m_va = 0;

  if (info->m_table_info->m_cf_type) {
    trace_uop->m_actual_taken = pi->m_actually_taken;
    trace_uop->m_target       = pi->m_branch_target;
  
    trace_uop->m_active_mask     = pi->m_ld_vaddr2;
    trace_uop->m_taken_mask      = pi->m_ld_vaddr1;
    trace_uop->m_reconverge_addr = pi->m_st_vaddr;
  } 
  // TODO (jaekyu, 2-9-2013)
  // what is AMP_VAL?
  else if (info->m_table_info->m_mem_type) {
    int amp_val = *KNOB(KNOB_MEM_SIZE_AMP);

    if (info->m_table_info->m_mem_type == MEM_ST || 
        info->m_table_info->m_mem_type == MEM_ST_LM || 
        info->m_table_info->m_mem_type == MEM_ST_SM) {
      trace_uop->m_va = MIN2((pi->m_st_vaddr + rep_offset)*amp_val, MAX_ADDR);
      trace_uop->m_mem_size = pi->m_mem_write_size * amp_val;
    } 
    else if ((info->m_table_info->m_mem_type == MEM_LD) || 
        (info->m_table_info->m_mem_type == MEM_LD_LM) || 
        (info->m_table_info->m_mem_type == MEM_LD_SM) || 
        (info->m_table_info->m_mem_type == MEM_LD_CM) || 
        (info->m_table_info->m_mem_type == MEM_LD_TM) || 
        (info->m_table_info->m_mem_type == MEM_LD_PM) || 
        (info->m_table_info->m_mem_type == MEM_PF)) { 
      if (info->m_trace_info.m_second_mem)
        trace_uop->m_va = MIN2((pi->m_ld_vaddr2 + rep_offset)*amp_val, MAX_ADDR);
      else 
        trace_uop->m_va = MIN2((pi->m_ld_vaddr1 +  rep_offset)*amp_val, MAX_ADDR);

      trace_uop->m_mem_size = pi->m_mem_read_size * amp_val;
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
inst_info_s* gpu_decoder_c::convert_pinuop_to_t_uop(trace_info_s *pi, trace_uop_s **trace_uop, 
    int core_id, int sim_thread_id)
{
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
        case GPU_MEM_LD_GM:
        case GPU_DATA_XFER_GM:
          trace_uop[0]->m_mem_type = MEM_LD;
          break;
        // XFER_LM is not valid for current PTX
        // check ptx manual, Atom and Red instrutions can be in only global/shared
        case GPU_MEM_LD_LM:
        case GPU_DATA_XFER_LM:
          trace_uop[0]->m_mem_type = MEM_LD_LM;
          break;
        case GPU_MEM_LD_SM:
        case GPU_DATA_XFER_SM:
          trace_uop[0]->m_mem_type = MEM_LD_SM;
          break;
        case GPU_MEM_LD_CM:
          trace_uop[0]->m_mem_type = MEM_LD_CM;
          break;
        case GPU_MEM_LD_TM:
          trace_uop[0]->m_mem_type = MEM_LD_TM;
          break;
        // parameter access same as global access
        case GPU_MEM_LD_PM:
          trace_uop[0]->m_mem_type = MEM_LD; 
          break;
        default:
          trace_uop[0]->m_mem_type = MEM_LD;
          break;
      }
      

      trace_uop[0]->m_cf_type       = NOT_CF;
      trace_uop[0]->m_op_type       = (pi->m_is_fp) ? UOP_FMEM : UOP_IMEM;
      trace_uop[0]->m_bar_type      = NOT_BAR;
      trace_uop[0]->m_num_dest_regs = pi->m_num_dest_regs;
      trace_uop[0]->m_num_src_regs  = pi->m_num_read_regs;
      trace_uop[0]->m_pin_2nd_mem   = 0;
      trace_uop[0]->m_eom           = 0;
      trace_uop[0]->m_alu_uop       = false;
      trace_uop[0]->m_inst_size     = pi->m_size;

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
          // for ptx traces, aon, num_ld will always be 1
          case GPU_MEM_LD_GM:
          case GPU_MEM_LD_LM:
          case GPU_MEM_LD_SM:
          case GPU_MEM_LD_CM:
          case GPU_MEM_LD_TM:
          case GPU_MEM_LD_PM:
          case GPU_DATA_XFER_GM:
          case GPU_DATA_XFER_LM:
          case GPU_DATA_XFER_SM:
            ASSERT(0);
            break;
          default:
            trace_uop[1]->m_mem_type = MEM_LD;
            break;
        }
        

        trace_uop[1]->m_cf_type       = NOT_CF;
        trace_uop[1]->m_op_type       = (pi->m_is_fp) ? UOP_FMEM : UOP_IMEM;
        trace_uop[1]->m_bar_type      = NOT_BAR;
        trace_uop[1]->m_num_dest_regs = 0;
        trace_uop[1]->m_num_src_regs  = pi->m_num_read_regs;
        trace_uop[1]->m_num_dest_regs = pi->m_num_dest_regs;
        trace_uop[1]->m_pin_2nd_mem   = 1;
        trace_uop[1]->m_eom           = 0;
        trace_uop[1]->m_alu_uop       = false;
        trace_uop[1]->m_inst_size     = pi->m_size;
        trace_uop[1]->m_mul_mem_uops  = pi->m_has_immediate; // uncoalesced memory accesses

        num_uop = 2;
      } // NUM_LOAD == 2


      write_dest_reg = 1;

      if (trace_uop[0]->m_mem_type == MEM_LD || 
          trace_uop[0]->m_mem_type == MEM_LD_LM || 
          trace_uop[0]->m_mem_type == MEM_LD_SM || 
          trace_uop[0]->m_mem_type == MEM_LD_CM || 
          trace_uop[0]->m_mem_type == MEM_LD_TM || 
          trace_uop[0]->m_mem_type == MEM_LD_PM) {
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
      cur_trace_uop->m_mem_type    = NOT_MEM;
      cur_trace_uop->m_cf_type     = NOT_CF;
      cur_trace_uop->m_op_type     = (Uop_Type)((pi->m_is_fp) ? 
          m_fp_uop_table[pi->m_opcode] : 
          m_int_uop_table[pi->m_opcode]);
      cur_trace_uop->m_bar_type    = NOT_BAR;
      cur_trace_uop->m_num_src_regs = pi->m_num_read_regs;
      cur_trace_uop->m_num_dest_regs = pi->m_num_dest_regs;
      cur_trace_uop->m_pin_2nd_mem  = 0;
      cur_trace_uop->m_eom        = 0;
      cur_trace_uop->m_alu_uop     = true;

      inst_has_ALU_uop = true;
    }


    ///
    /// 2. Instruction has a memory store operation
    ///
    if (pi->m_has_st) {
      trace_uop_s* cur_trace_uop = trace_uop[num_uop++];
      if (inst_has_ld_uop) 
        tmp_reg_needed = true;
   
      // set memory type
      switch (pi->m_opcode) {
        case GPU_MEM_ST_GM:
        case GPU_DATA_XFER_GM:
          cur_trace_uop->m_mem_type = MEM_ST;
          break;

        case GPU_MEM_ST_LM:
        case GPU_DATA_XFER_LM: // XFER_LM is not valid for current PTX
          cur_trace_uop->m_mem_type = MEM_ST_LM;
          break;
        case GPU_MEM_ST_SM:
        case GPU_DATA_XFER_SM:
          cur_trace_uop->m_mem_type = MEM_ST_SM;
          break;
        default:
          cur_trace_uop->m_mem_type = MEM_ST;
          break;
      }

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

      DEBUG("pi->instruction_addr:0x%s trace_uop[%d]->addr:0x%s num_src_regs:%d "
          "num_read_regs:%d pi:num_dst_regs:%d uop:num_dst_regs:%d \n",
          hexstr64s(pi->m_instruction_addr), ii, hexstr64s(trace_uop[ii]->m_addr), 
          trace_uop[ii]->m_num_src_regs, pi->m_num_read_regs, pi->m_num_dest_regs, 
          trace_uop[ii]->m_num_dest_regs);

      // set source register
      for (jj = 0; jj < trace_uop[ii]->m_num_src_regs; ++jj) {
        (trace_uop[ii])->m_srcs[jj].m_type = (Reg_Type)0;
        (trace_uop[ii])->m_srcs[jj].m_id   = pi->m_src[jj];
        (trace_uop[ii])->m_srcs[jj].m_reg  = pi->m_src[jj];
      }

      // store or control flow has a dependency whoever the last one
      if ((trace_uop[ii]->m_mem_type == MEM_ST) || 
          (trace_uop[ii]->m_mem_type == MEM_ST_LM) || 
          (trace_uop[ii]->m_mem_type == MEM_ST_SM) || 
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
      if (tmp_reg_needed && 
          ((trace_uop[ii]->m_mem_type == MEM_LD) || 
           (trace_uop[ii]->m_mem_type == MEM_LD_LM) ||
           (trace_uop[ii]->m_mem_type == MEM_LD_SM) || 
           (trace_uop[ii]->m_mem_type == MEM_LD_CM) ||
           (trace_uop[ii]->m_mem_type == MEM_LD_TM) || 
           (trace_uop[ii]->m_mem_type == MEM_LD_PM))) {
        (trace_uop[ii])->m_dests[jj].m_type = (Reg_Type)0;
        (trace_uop[ii])->m_dests[jj].m_id   = TR_REG_TMP0;
        (trace_uop[ii])->m_dests[jj].m_reg = TR_REG_TMP0;
        trace_uop[ii]->m_num_dest_regs     += 1;
      }


      // the last uop
      if (ii == (num_uop - 1) && trace_uop[num_uop-1]->m_mem_type == NOT_MEM) {
        /* last uop's info */
#ifdef GPU_VALIDATION
        if (pi->m_opcode == GPU_BAR) {
          // only the last instruction will have bar type
          trace_uop[(num_uop-1)]->m_bar_type = BAR_FETCH;
          trace_uop[(num_uop-1)]->m_cf_type  = CF_ICO;
        }
#else
#endif
      }

      // update instruction information with MacSim trace
      convert_t_uop_to_info(trace_uop[ii], info);

      DEBUG("tuop: pc %s num_src_reg:%d num_dest_reg:%d \n",
          hexstr64s(trace_uop[ii]->m_addr), trace_uop[ii]->m_num_src_regs, 
          trace_uop[ii]->m_num_dest_regs);
      
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
      if (trace_uop[ii]->m_mem_type) {
        //nagesh mar-10-2010 - to form single uop for uncoalesced memory accesses
        //this checking should be done for every instance of the instruction,
        //not for only the first instance, because depending on the address
        //calculation, some accesses may be coalesced, some may be uncoalesced
        trace_uop[ii]->m_mul_mem_uops = pi->m_has_immediate; 
      }
    }

    // set end of macro flag to the last uop
    trace_uop[num_uop-1]->m_eom = 1;

    ASSERT(num_uop > 0);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////
  // end of instruction decoding
  /////////////////////////////////////////////////////////////////////////////////////////////

  dyn_uop_counter = num_uop;
  ASSERT(dyn_uop_counter);


  // set eom flag and next pc address for the last uop of this instruction
  trace_uop[dyn_uop_counter-1]->m_eom = 1;
  trace_uop[dyn_uop_counter-1]->m_npc = pi->m_instruction_next_addr;

  STAT_CORE_EVENT(core_id, OP_CAT_GPU_INVALID + (pi->m_opcode)); 

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


///////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Get an uop from trace
 * Called by frontend.cc
 * @param core_id - core id
 * @param uop - uop object to hold instruction information
 * @param sim_thread_id thread id
 */
bool gpu_decoder_c::get_uops_from_traces(int core_id, uop_c *uop, int sim_thread_id)
{
  ASSERT(uop);

  trace_uop_s *trace_uop;
  int num_uop  = 0;
  core_c* core = m_simBase->m_core_pointers[core_id];
  inst_info_s *info;

  // fetch ended : no uop to fetch
  if (core->m_fetch_ended[sim_thread_id]) 
    return false;

  trace_info_s trace_info;
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


    // Copy current instruction to data structure
    memcpy(&trace_info, thread_trace_info->m_prev_trace_info, sizeof(trace_info_s));

    // Set next pc address
    trace_info.m_instruction_next_addr = 
      thread_trace_info->m_next_trace_info->m_instruction_addr;

    // Copy next instruction to current instruction field
    memcpy(thread_trace_info->m_prev_trace_info, thread_trace_info->m_next_trace_info, 
        sizeof(trace_info_s));

    DEBUG("trace_read nm core_id:%d thread_id:%d pc:%s opcode:%d inst_count:%lu\n",
        core_id, sim_thread_id, hexstr64s(trace_info.m_instruction_addr), 
        static_cast<int>(trace_info.m_opcode), thread_trace_info->m_temp_inst_count);

#ifdef GPU_VALIDATION
    ASSERTM(trace_info.m_opcode > PREFETCH_T2 && trace_info.m_opcode < TR_OPCODE_LAST, "opcode is %d\n", trace_info.m_opcode);
#endif


    ///
    /// Trace read failed
    ///
    if (!read_success) 
      return false;


    // read a new instruction, so update stats
    if (inst_read) { 
      ++core->m_inst_fetched[sim_thread_id];
      DEBUG("core_id:%d thread_id:%d inst_num:%lu\n",
          core_id, sim_thread_id, thread_trace_info->m_temp_inst_count + 1);

      if (core->m_inst_fetched[sim_thread_id] > core->m_max_inst_fetched) 
        core->m_max_inst_fetched = core->m_inst_fetched[sim_thread_id];
    }


    // GPU simulation : if we use common cache for the shared memory
    // Set appropiriate opcode type (not using shared memory)
    if (*KNOB(KNOB_PTX_COMMON_CACHE)) {
      switch (trace_info.m_opcode) {
        case GPU_MEM_LD_SM:
          trace_info.m_opcode = GPU_MEM_LD_LM;
          break;
        case GPU_MEM_ST_SM:
          trace_info.m_opcode = GPU_MEM_ST_LM;
          break;
        case GPU_DATA_XFER_SM:
          trace_info.m_opcode = GPU_DATA_XFER_LM;
          break;
      }
    }


    // So far we have raw instruction format, so we need to MacSim specific trace format
    info = convert_pinuop_to_t_uop(&trace_info, thread_trace_info->m_trace_uop_array, 
        core_id, sim_thread_id);


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
    DEBUG("core_id:%d thread_id:%d inst_num:%lld uop_num:%lld fetched:%lld last uop\n",
        core_id, sim_thread_id, uop->m_inst_num, uop->m_uop_num, 
        core->m_inst_fetched[sim_thread_id]);
  }


  /* BAR_FETCH */
  if (trace_uop->m_bar_type == BAR_FETCH) { //only last uop with have BAR_FETCH set
    frontend_c *frontend   = core->get_frontend();
    frontend_s *fetch_data = core->get_trace_info(sim_thread_id)->m_fetch_data;

    fetch_data->m_fetch_blocked = true;

    bool new_entry = false;
    sync_thread_s* sync_info = frontend->m_sync_info->hash_table_access_create(
        core->get_trace_info(sim_thread_id)->m_block_id, &new_entry);

    // new synchronization information
    if (new_entry) {
      sync_info->m_block_id = core->get_trace_info(sim_thread_id)->m_block_id;
      sync_info->m_sync_count = 0;
      sync_info->m_num_threads_in_block = 
        m_simBase->m_block_schedule_info[sync_info->m_block_id]->m_total_thread_num;
    }

    ++fetch_data->m_sync_count;
    fetch_data->m_sync_wait_start = core->get_cycle_count();

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
  }


  uop->m_mem_size = trace_uop->m_mem_size;
  if (uop->m_mem_type != NOT_MEM && 
      uop->m_mem_type != MEM_LD_SM && 
      uop->m_mem_type != MEM_ST_SM && 
      uop->m_mem_type != MEM_LD_CM && 
      uop->m_mem_type != MEM_LD_TM) {
    int temp_num_req = (uop->m_mem_size + *KNOB(KNOB_MAX_TRANSACTION_SIZE) - 1) / 
      *KNOB(KNOB_MAX_TRANSACTION_SIZE);

    ASSERTM(temp_num_req > 0, "size:%d max:%d num:%d type:%d num:%d\n", uop->m_mem_size, 
        (int)*KNOB(KNOB_MAX_TRANSACTION_SIZE), temp_num_req, uop->m_mem_type, 
        trace_uop->m_info->m_trace_info.m_num_uop);
  }

  uop->m_dir     = trace_uop->m_actual_taken;
  uop->m_pc      = info->m_addr;
  uop->m_core_id = core_id;


  // we found first uop of an instruction, so add instruction count
  if (uop->m_isitBOM) 
    ++thread_trace_info->m_temp_inst_count;

  uop->m_inst_num  = thread_trace_info->m_temp_inst_count;
  uop->m_num_srcs  = trace_uop->m_num_src_regs;
  uop->m_num_dests = trace_uop->m_num_dest_regs;

  ASSERTM(uop->m_num_dests < MAX_DST_NUM, "uop->num_dests=%d MAX_DST_NUM=%d\n", 
      uop->m_num_dests, MAX_DST_NUM);


  // uop number is specific to the core
  uop->m_unique_num = core->inc_and_get_unique_uop_num();

  DEBUG("uop_num:%lld num_srcs:%d  trace_uop->num_src_regs:%d  num_dsts:%d num_seing_uop:%d "
      "pc:0x%s dir:%d \n",
      uop->m_uop_num, uop->m_num_srcs, trace_uop->m_num_src_regs, uop->m_num_dests, 
      thread_trace_info->m_num_sending_uop, hexstr64s(uop->m_pc), uop->m_dir);

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
  ///
  if (uop->m_mem_type != NOT_MEM) {
    int index                = thread_trace_info->m_num_sending_uop - 1;
    bool multiple_mem_access = thread_trace_info->m_trace_uop_array[index]->m_mul_mem_uops;
    bool coalesced;


    if (multiple_mem_access) {
      coalesced = false;
      STAT_EVENT(UNCOAL_INST);
    }
    else {
      coalesced = true;
      STAT_EVENT(COAL_INST);
    }

    // update stats
    switch (uop->m_mem_type) {
      case MEM_ST_SM:
      case MEM_LD_SM:
        if (coalesced) STAT_EVENT(SM_COAL_INST); else STAT_EVENT(SM_UNCOAL_INST);
        break;
      case MEM_LD_CM:
        if (coalesced) STAT_EVENT(CM_COAL_INST); else STAT_EVENT(CM_UNCOAL_INST);
        break;
      case MEM_LD_TM:
        if (coalesced) STAT_EVENT(TM_COAL_INST); else STAT_EVENT(TM_UNCOAL_INST);
        break;
      default:
        if (coalesced) STAT_EVENT(DM_COAL_INST); else STAT_EVENT(DM_UNCOAL_INST);
        break;
    }

    Addr cache_line_addr;
    int cache_line_size;
    switch (uop->m_mem_type) {
      // shared memory
      case MEM_LD_SM:
      case MEM_ST_SM:
        cache_line_addr = core->get_shared_memory()->base_cache_line(uop->m_vaddr);
        cache_line_size = core->get_shared_memory()->cache_line_size();
        break;
      // constant memory
      case MEM_LD_CM:
        cache_line_addr = core->get_const_cache()->base_cache_line(uop->m_vaddr);
        cache_line_size = core->get_const_cache()->cache_line_size();
        break;
      // texture memory
      case MEM_LD_TM:
        cache_line_addr = core->get_texture_cache()->base_cache_line(uop->m_vaddr);
        cache_line_size = core->get_texture_cache()->cache_line_size();
        break;
      // global memory, parameter memory
      default:
        if (*KNOB(KNOB_BYTE_LEVEL_ACCESS)) {
          cache_line_addr = uop->m_vaddr;
          cache_line_size = *KNOB(KNOB_MAX_TRANSACTION_SIZE);
        }
        else {
          cache_line_addr = m_simBase->m_memory->base_addr(core_id, uop->m_vaddr);
          cache_line_size = m_simBase->m_memory->line_size(core_id);
        }
        break;
    }


    // Even though all accesses are coalesced, based on the maximum transaction size,
    // we may have multiple uops. This only happens in GPU simulation
    int num_mem_req = ((uop->m_vaddr + uop->m_mem_size - cache_line_addr) + 
        (cache_line_size - 1)) / cache_line_size;


    // FIXME
    // multiple transactions due to 1) uncoalesced accesses or 2) too large memory request
    if (multiple_mem_access || num_mem_req > 1) {
      // update stats
      if (coalesced) {
        STAT_EVENT(COAL_INST_MUL_TRANS);

        switch (uop->m_mem_type) {
          case MEM_ST_SM:
          case MEM_LD_SM:
            STAT_EVENT(SM_COAL_INST_MUL_TRANS);
            break;
          case MEM_LD_CM:
            STAT_EVENT(CM_COAL_INST_MUL_TRANS);
            break;
          case MEM_LD_TM:
            STAT_EVENT(TM_COAL_INST_MUL_TRANS);
            break;
          default:
            STAT_EVENT(DM_COAL_INST_MUL_TRANS);
            break;
        }
      }
      else {
        STAT_EVENT(UNCOAL_INST_MUL_TRANS);
      }


      // FIXME
      bool subsequent_mem = false;
      if (multiple_mem_access) {
        if (!thread_trace_info->m_trace_uop_array[index]->m_eom) {
          for (int i = 0; ; ++i) {
            ASSERT((index + 1) < MAX_PUP);
            if (thread_trace_info->m_trace_uop_array[++index]->m_mem_type) {
              subsequent_mem = true;
              break;
            }
            ASSERT((index + 1) < MAX_PUP);
            if (thread_trace_info->m_trace_uop_array[++index]->m_eom) {
              break;
            }
          }
        }
      }

      if (multiple_mem_access && !thread_trace_info->m_trace_ended)
        ASSERT(ungetch_trace(core_id, sim_thread_id, 1));

      uop_c *child_mem_uop = NULL;
      int mem_req_count    = 0;
      uop_c *child_mem_reqs[128];

      bool inst_read         = false;
      bool last_inst         = false;
      int num_inst_to_unread = 0;

      static set<Addr> seen_block_addr;
      if (*m_simBase->m_knobs->KNOB_USE_NEW_COALESCING) seen_block_addr.clear();
      do {
        for (int i = 0; i < num_mem_req; ++i) {

          if (*m_simBase->m_knobs->KNOB_USE_NEW_COALESCING) {
            auto itr = seen_block_addr.find(cache_line_addr + i * cache_line_size);
            auto end = seen_block_addr.end();

            if (itr != end) continue;
            seen_block_addr.insert(cache_line_addr + i * cache_line_size);
          }

          child_mem_uop = core->get_frontend()->get_uop_pool()->acquire_entry(m_simBase);
          child_mem_uop->allocate();
          ASSERT(child_mem_uop); 

          memcpy(child_mem_uop, uop, sizeof(uop_c));

          child_mem_reqs[mem_req_count++] = child_mem_uop;
          child_mem_uop->m_parent_uop     = uop;

          child_mem_uop->m_vaddr    = cache_line_addr + i * cache_line_size;
          Addr vaddr = uop->m_vaddr + uop->m_mem_size;
          child_mem_uop->m_mem_size = (i != (num_mem_req - 1)) ? 
            cache_line_size : (vaddr - cache_line_addr) - (num_mem_req - 1) * cache_line_size;
          child_mem_uop->m_uop_num    = thread_trace_info->m_temp_uop_count++;
          child_mem_uop->m_unique_num = core->inc_and_get_unique_uop_num();
          child_mem_uop->m_uncoalesced_flag = uop->m_uncoalesced_flag;
        }

        if (!multiple_mem_access || last_inst) {
          if (subsequent_mem) {
            //rewind trace
            ASSERT(ungetch_trace(core_id, sim_thread_id, num_inst_to_unread));
          }
          else {
            if (multiple_mem_access) {
              if (!thread_trace_info->m_trace_ended) {
                read_success = peek_trace(core_id, thread_trace_info->m_prev_trace_info, 
                    sim_thread_id, &inst_read);
                if (read_success) {
                  if (inst_read) {
                    uop->m_npc = thread_trace_info->m_prev_trace_info->m_instruction_addr;
                  }
                  else {
                    thread_trace_info->m_trace_ended = true;
                    DEBUG("trace ended core_id:%d thread_id:%d\n", core_id, sim_thread_id);
                  }
                }
                else {
                  ASSERT(0);
                }
              }
            }
            else {
              uop->m_npc = thread_trace_info->m_prev_trace_info->m_instruction_addr;
            }
          }


          // create children uops for uncoalcesed accesses
          uop->m_child_uops = new uop_c *[mem_req_count];

          uop->m_num_child_uops      = mem_req_count;
          uop->m_num_child_uops_done = 0;
          if (uop->m_num_child_uops != 64) {
            uop->m_pending_child_uops  = N_BIT_MASK(uop->m_num_child_uops);
          }
          else {
            uop->m_pending_child_uops  = N_BIT_MASK_64;
          }
          uop->m_vaddr               = 0;
          uop->m_mem_size            = 0;

          for (int ii = 0; ii < mem_req_count; ++ii) {
            uop->m_child_uops[ii] = child_mem_reqs[ii];
            child_mem_reqs[ii]    = NULL;
          }

          break;
        } // end if (!multiple_mem_access || last_inst)


        // we dont want the side-effects of read_trace()
        read_success = peek_trace(core_id, &trace_info, sim_thread_id, &inst_read);
        if (!read_success || (read_success && !inst_read)) {
          ASSERT(0);
        }
        ++num_inst_to_unread;

        // has_immediate indicates whether the trace instruction is part of 
        // an uncoalesced memory access
        last_inst = trace_info.m_has_immediate; 

        //TODO: not using active mask value - here and in other places as well
        if (uop->m_mem_type == MEM_LD || 
            uop->m_mem_type == MEM_LD_LM ||
            uop->m_mem_type == MEM_LD_SM || 
            uop->m_mem_type == MEM_LD_CM ||
            uop->m_mem_type == MEM_LD_TM || 
            uop->m_mem_type == MEM_LD_PM) {
          uop->m_vaddr    = trace_info.m_ld_vaddr1;
          // BYTE
          uop->m_mem_size = trace_info.m_mem_read_size;
          ASSERTM(trace_info.m_mem_read_size > 0, "mem_type:%d\n", uop->m_mem_type);
          
          if (uop->m_mem_type != NOT_MEM && uop->m_mem_type != MEM_LD_SM && 
              uop->m_mem_type != MEM_ST_SM && uop->m_mem_type != MEM_LD_CM && 
              uop->m_mem_type != MEM_LD_TM) {
            int temp_num_req = (uop->m_mem_size + *KNOB(KNOB_MAX_TRANSACTION_SIZE) - 1) / 
              *KNOB(KNOB_MAX_TRANSACTION_SIZE);
            ASSERTM(temp_num_req > 0, "size:%d max:%d num:%d type:%s\n", 
                uop->m_mem_size, (int)*KNOB(KNOB_MAX_TRANSACTION_SIZE), temp_num_req, 
                uop_c::g_mem_type_name[uop->m_mem_type]);

          }
        }
        else if (uop->m_mem_type == MEM_ST || 
            uop->m_mem_type == MEM_ST_LM || 
            uop->m_mem_type == MEM_ST_SM) {
          uop->m_vaddr    = trace_info.m_st_vaddr;
          // BYTE
          uop->m_mem_size = trace_info.m_mem_write_size;

          if (uop->m_mem_type != NOT_MEM && 
              uop->m_mem_type != MEM_LD_SM && 
              uop->m_mem_type != MEM_ST_SM && 
              uop->m_mem_type != MEM_LD_CM && 
              uop->m_mem_type != MEM_LD_TM) {
            int temp_num_req = (uop->m_mem_size + *KNOB(KNOB_MAX_TRANSACTION_SIZE) - 1) / 
              *KNOB(KNOB_MAX_TRANSACTION_SIZE);
            ASSERTM(temp_num_req > 0, "size:%d max:%d num:%d type:%s\n", 
                uop->m_mem_size, (int)*KNOB(KNOB_MAX_TRANSACTION_SIZE), temp_num_req, 
                uop_c::g_mem_type_name[uop->m_mem_type]);
          }
        }
        else {
          ASSERT(0);
        }

        // address translation
        int process_id = thread_trace_info->m_process->m_process_id;
        unsigned long offset = UINT_MAX * process_id * 10;
        uop->m_vaddr += m_simBase->m_memory->base_addr(core_id, offset); 

        switch (uop->m_mem_type) {
          // shared memory
          case MEM_LD_SM:
          case MEM_ST_SM:
            cache_line_addr = core->get_shared_memory()->base_cache_line(uop->m_vaddr);
            break;
          // constant memory
          case MEM_LD_CM:
            cache_line_addr = core->get_const_cache()->base_cache_line(uop->m_vaddr);
            break;
          // texture cache
          case MEM_LD_TM:
            cache_line_addr = core->get_texture_cache()->base_cache_line(uop->m_vaddr);
            break;
          // global memory, parameter memory
          default:
            if (*KNOB(KNOB_BYTE_LEVEL_ACCESS)) {
              cache_line_addr = uop->m_vaddr;
            }
            else {
              cache_line_addr = m_simBase->m_memory->base_addr(core_id, uop->m_vaddr);
            }
            break;
        }
        Addr vaddr = uop->m_vaddr + uop->m_mem_size;
        num_mem_req = ((vaddr - cache_line_addr) + (cache_line_size - 1)) / cache_line_size;
      } while (1);
    } // MULTIPLE_TRANSACTION
    else {
      ASSERT(coalesced);

      // update stats
      STAT_EVENT(COAL_INST_SINGLE_TRANS);

      switch (uop->m_mem_type) {
        case MEM_LD_SM:
        case MEM_ST_SM:
          STAT_EVENT(SM_COAL_INST_SINGLE_TRANS);
          break;
        case MEM_LD_CM:
          STAT_EVENT(CM_COAL_INST_SINGLE_TRANS);
          break;
        case MEM_LD_TM:
          STAT_EVENT(TM_COAL_INST_SINGLE_TRANS);
          break;
        default:
          STAT_EVENT(DM_COAL_INST_SINGLE_TRANS);
          break;
      }
    }
  }

  DEBUG("new uop: uop_num:%lld inst_num:%lld thread_id:%d unique_num:%lld \n",
      uop->m_uop_num, uop->m_inst_num, uop->m_thread_id, uop->m_unique_num);

  return read_success;
}


///////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Initialize the mapping between trace opcode and uop type
 */
void gpu_decoder_c::init_pin_convert(void)
{
  m_int_uop_table[GPU_ABS]               		= UOP_GPU_ABS;
  m_int_uop_table[GPU_ABS64]             		= UOP_GPU_ABS64;
  m_int_uop_table[GPU_ADD]               		= UOP_GPU_ADD; 
  m_int_uop_table[GPU_ADD64]             		= UOP_GPU_ADD64; 
	m_int_uop_table[GPU_ADDC]             		= UOP_GPU_ADDC;
	m_int_uop_table[GPU_AND]               		= UOP_GPU_AND;
	m_int_uop_table[GPU_AND64]             		= UOP_GPU_AND64;
	m_int_uop_table[GPU_ATOM]             		= UOP_GPU_ATOM;
	m_int_uop_table[GPU_ATOM64]            		= UOP_GPU_ATOM64;
	m_int_uop_table[GPU_BAR]               		= UOP_GPU_BAR;
	m_int_uop_table[GPU_BFE]               		= UOP_GPU_BFE;
	m_int_uop_table[GPU_BFE64]             		= UOP_GPU_BFE64;
	m_int_uop_table[GPU_BFI]             	  	= UOP_GPU_BFI;
	m_int_uop_table[GPU_BFI64]             		= UOP_GPU_BFI64;
	m_int_uop_table[GPU_BFIND]             		= UOP_GPU_BFIND;
	m_int_uop_table[GPU_BFIND64]            	= UOP_GPU_BFIND64;
	m_int_uop_table[GPU_BRA]             	  	= UOP_GPU_BRA;
	m_int_uop_table[GPU_BREV]             		= UOP_GPU_BREV;
	m_int_uop_table[GPU_BREV64]             	= UOP_GPU_BREV64;
	m_int_uop_table[GPU_BRKPT]             		= UOP_GPU_BRKPT;
	m_int_uop_table[GPU_CALL]             		= UOP_GPU_CALL;
	m_int_uop_table[GPU_CLZ]             	  	= UOP_GPU_CLZ;
	m_int_uop_table[GPU_CLZ64]             		= UOP_GPU_CLZ64;
	m_int_uop_table[GPU_CNOT]             		= UOP_GPU_CNOT;
	m_int_uop_table[GPU_CNOT64]             	= UOP_GPU_CNOT64;
	m_int_uop_table[GPU_COPYSIGN]           	= UOP_GPU_COPYSIGN;
	m_int_uop_table[GPU_COPYSIGN64]         	= UOP_GPU_COPYSIGN64;
	m_int_uop_table[GPU_COS]             	  	= UOP_GPU_COS;
	m_int_uop_table[GPU_CVT]             	  	= UOP_GPU_CVT;
	m_int_uop_table[GPU_CVT64]             		= UOP_GPU_CVT64;
	m_int_uop_table[GPU_CVTA]             		= UOP_GPU_CVTA;
	m_int_uop_table[GPU_CVTA64]            		= UOP_GPU_CVTA64;
	m_int_uop_table[GPU_DIV]             	  	= UOP_GPU_DIV;
	m_int_uop_table[GPU_DIV64]             		= UOP_GPU_DIV64;
	m_int_uop_table[GPU_EX2]             	  	= UOP_GPU_EX2;
	m_int_uop_table[GPU_EXIT]             		= UOP_GPU_EXIT;
	m_int_uop_table[GPU_FMA]             	  	= UOP_GPU_FMA;
	m_int_uop_table[GPU_FMA64]             		= UOP_GPU_FMA64;
	m_int_uop_table[GPU_ISSPACEP]           	= UOP_GPU_ISSPACEP;
	m_int_uop_table[GPU_LD]             		  = UOP_GPU_LD;
	m_int_uop_table[GPU_LD64]             		= UOP_GPU_LD64;
	m_int_uop_table[GPU_LDU]             		  = UOP_GPU_LDU;
	m_int_uop_table[GPU_LDU64]             		= UOP_GPU_LDU64;
	m_int_uop_table[GPU_LG2]               		= UOP_GPU_LG2;
	m_int_uop_table[GPU_MAD24]             		= UOP_GPU_MAD24;
	m_int_uop_table[GPU_MAD]             	  	= UOP_GPU_MAD;
	m_int_uop_table[GPU_MAD64]             		= UOP_GPU_MAD64;
	m_int_uop_table[GPU_MAX]             	  	= UOP_GPU_MAX;
	m_int_uop_table[GPU_MAX64]             		= UOP_GPU_MAX64;
	m_int_uop_table[GPU_MEMBAR]             	= UOP_GPU_MEMBAR;
	m_int_uop_table[GPU_MIN]             		  = UOP_GPU_MIN;
	m_int_uop_table[GPU_MIN64]             		= UOP_GPU_MIN64;
	m_int_uop_table[GPU_MOV]             	  	= UOP_GPU_MOV;
	m_int_uop_table[GPU_MOV64]             		= UOP_GPU_MOV64;
	m_int_uop_table[GPU_MUL24]             		= UOP_GPU_MUL24;
	m_int_uop_table[GPU_MUL]             		  = UOP_GPU_MUL;
	m_int_uop_table[GPU_MUL64]             		= UOP_GPU_MUL64;
	m_int_uop_table[GPU_NEG]             		  = UOP_GPU_NEG;
	m_int_uop_table[GPU_NEG64]             		= UOP_GPU_NEG64;
	m_int_uop_table[GPU_NOT]             		  = UOP_GPU_NOT;
	m_int_uop_table[GPU_NOT64]             		= UOP_GPU_NOT64;
	m_int_uop_table[GPU_OR]             	  	= UOP_GPU_OR;
	m_int_uop_table[GPU_OR64]             		= UOP_GPU_OR64;
	m_int_uop_table[GPU_PMEVENT]            	= UOP_GPU_PMEVENT;
	m_int_uop_table[GPU_POPC]             		= UOP_GPU_POPC;
	m_int_uop_table[GPU_POPC64]             	= UOP_GPU_POPC64;
	m_int_uop_table[GPU_PREFETCH]           	= UOP_GPU_PREFETCH;
	m_int_uop_table[GPU_PREFETCHU]          	= UOP_GPU_PREFETCHU;
	m_int_uop_table[GPU_PRMT]             		= UOP_GPU_PRMT;
	m_int_uop_table[GPU_RCP]             	  	= UOP_GPU_RCP;
	m_int_uop_table[GPU_RCP64]             		= UOP_GPU_RCP64;
	m_int_uop_table[GPU_RED]             	  	= UOP_GPU_RED;
	m_int_uop_table[GPU_RED64]             		= UOP_GPU_RED64;
	m_int_uop_table[GPU_REM]             	  	= UOP_GPU_REM;
	m_int_uop_table[GPU_REM64]             		= UOP_GPU_REM64;
	m_int_uop_table[GPU_RET]             	  	= UOP_GPU_RET;
	m_int_uop_table[GPU_RSQRT]             		= UOP_GPU_RSQRT;
	m_int_uop_table[GPU_RSQRT64]            	= UOP_GPU_RSQRT64;
	m_int_uop_table[GPU_SAD]             	  	= UOP_GPU_SAD;
	m_int_uop_table[GPU_SAD64]             		= UOP_GPU_SAD64;
	m_int_uop_table[GPU_SELP]             		= UOP_GPU_SELP;
	m_int_uop_table[GPU_SELP64]             	= UOP_GPU_SELP64;
	m_int_uop_table[GPU_SET]             	  	= UOP_GPU_SET;
	m_int_uop_table[GPU_SET64]             		= UOP_GPU_SET64;
	m_int_uop_table[GPU_SETP]             		= UOP_GPU_SETP;
	m_int_uop_table[GPU_SETP64]             	= UOP_GPU_SETP64;
	m_int_uop_table[GPU_SHL]             	  	= UOP_GPU_SHL;
	m_int_uop_table[GPU_SHL64]             		= UOP_GPU_SHL64;
	m_int_uop_table[GPU_SHR]             		  = UOP_GPU_SHR;
	m_int_uop_table[GPU_SHR64]             		= UOP_GPU_SHR64;
	m_int_uop_table[GPU_SIN]             	  	= UOP_GPU_SIN;
	m_int_uop_table[GPU_SLCT]             		= UOP_GPU_SLCT;
	m_int_uop_table[GPU_SLCT64]            		= UOP_GPU_SLCT64;
	m_int_uop_table[GPU_SQRT]             		= UOP_GPU_SQRT;
	m_int_uop_table[GPU_SQRT64]            		= UOP_GPU_SQRT64;
	m_int_uop_table[GPU_ST]             	  	= UOP_GPU_ST;
	m_int_uop_table[GPU_ST64]             		= UOP_GPU_ST64;
	m_int_uop_table[GPU_SUB]             		  = UOP_GPU_SUB;
	m_int_uop_table[GPU_SUB64]             		= UOP_GPU_SUB64;
	m_int_uop_table[GPU_SUBC]             		= UOP_GPU_SUBC;
	m_int_uop_table[GPU_SULD]             		= UOP_GPU_SULD;
	m_int_uop_table[GPU_SULD64]             	= UOP_GPU_SULD64;
	m_int_uop_table[GPU_SURED]             		= UOP_GPU_SURED;
	m_int_uop_table[GPU_SURED64]            	= UOP_GPU_SURED64;
	m_int_uop_table[GPU_SUST]             		= UOP_GPU_SUST;
	m_int_uop_table[GPU_SUST64]             	= UOP_GPU_SUST64;
	m_int_uop_table[GPU_SUQ]             		  = UOP_GPU_SUQ;
  m_int_uop_table[GPU_TESTP]             		= UOP_GPU_TESTP;
  m_int_uop_table[GPU_TESTP64]            	= UOP_GPU_TESTP64;
  m_int_uop_table[GPU_TEX]             		  = UOP_GPU_TEX;
  m_int_uop_table[GPU_TLD4]             		= UOP_GPU_TLD4;
  m_int_uop_table[GPU_TXQ]             		  = UOP_GPU_TXQ;
  m_int_uop_table[GPU_TRAP]             		= UOP_GPU_TRAP;
  m_int_uop_table[GPU_VABSDIFF]             = UOP_GPU_VABSDIFF;
  m_int_uop_table[GPU_VADD]             		= UOP_GPU_VADD;
  m_int_uop_table[GPU_VMAD]             		= UOP_GPU_VMAD;
  m_int_uop_table[GPU_VMAX]             		= UOP_GPU_VMAX;
  m_int_uop_table[GPU_VMIN]             		= UOP_GPU_VMIN;
  m_int_uop_table[GPU_VSET]             		= UOP_GPU_VSET;
  m_int_uop_table[GPU_VSHL]             		= UOP_GPU_VSHL;
  m_int_uop_table[GPU_VSHR]             		= UOP_GPU_VSHR;
  m_int_uop_table[GPU_VSUB]             		= UOP_GPU_VSUB;
  m_int_uop_table[GPU_VOTE]             		= UOP_GPU_VOTE;
  m_int_uop_table[GPU_XOR]             		  = UOP_GPU_XOR;
  m_int_uop_table[GPU_XOR64]             		= UOP_GPU_XOR64;
  m_int_uop_table[GPU_RECONVERGE]           = UOP_GPU_RECONVERGE;
  m_int_uop_table[GPU_PHI]             		  = UOP_GPU_PHI;
  m_int_uop_table[GPU_MEM_LD_GM]            = UOP_IADD;
  m_int_uop_table[GPU_MEM_LD_LM]            = UOP_IADD;
  m_int_uop_table[GPU_MEM_LD_SM]            = UOP_IADD;
  m_int_uop_table[GPU_MEM_LD_CM]            = UOP_IADD;
  m_int_uop_table[GPU_MEM_LD_TM]            = UOP_IADD;
  m_int_uop_table[GPU_MEM_LD_PM]            = UOP_IADD;
  m_int_uop_table[GPU_MEM_ST_GM]            = UOP_IADD;
  m_int_uop_table[GPU_MEM_ST_LM]            = UOP_IADD;
  m_int_uop_table[GPU_MEM_ST_SM]            = UOP_IADD;
  m_int_uop_table[GPU_DATA_XFER_GM]         = UOP_IADD;
  m_int_uop_table[GPU_DATA_XFER_LM]         = UOP_IADD;
  m_int_uop_table[GPU_DATA_XFER_SM]         = UOP_IADD;


  m_fp_uop_table[GPU_ABS]                  = UOP_GPU_FABS;
  m_fp_uop_table[GPU_ABS64]              	 = UOP_GPU_FABS64;
  m_fp_uop_table[GPU_ADD]                	 = UOP_GPU_FADD; 
  m_fp_uop_table[GPU_ADD64]              	 = UOP_GPU_FADD64; 
	m_fp_uop_table[GPU_ADDC]              	 = UOP_GPU_FADDC;
	m_fp_uop_table[GPU_AND]                	 = UOP_GPU_FAND;
	m_fp_uop_table[GPU_AND64]              	 = UOP_GPU_FAND64;
	m_fp_uop_table[GPU_ATOM]              	 = UOP_GPU_FATOM;
	m_fp_uop_table[GPU_ATOM64]             	 = UOP_GPU_FATOM64;
	m_fp_uop_table[GPU_BAR]                	 = UOP_GPU_FBAR;
	m_fp_uop_table[GPU_BFE]               	 = UOP_GPU_FBFE;
	m_fp_uop_table[GPU_BFE64]             	 = UOP_GPU_FBFE64;
	m_fp_uop_table[GPU_BFI]             	   = UOP_GPU_FBFI;
	m_fp_uop_table[GPU_BFI64]             	 = UOP_GPU_FBFI64;
	m_fp_uop_table[GPU_BFIND]             	 = UOP_GPU_FBFIND;
	m_fp_uop_table[GPU_BFIND64]            	 = UOP_GPU_FBFIND64;
	m_fp_uop_table[GPU_BRA]             	   = UOP_GPU_FBRA;
	m_fp_uop_table[GPU_BREV]             		 = UOP_GPU_FBREV;
	m_fp_uop_table[GPU_BREV64]             	 = UOP_GPU_FBREV64;
	m_fp_uop_table[GPU_BRKPT]             	 = UOP_GPU_FBRKPT;
	m_fp_uop_table[GPU_CALL]             		 = UOP_GPU_FCALL;
	m_fp_uop_table[GPU_CLZ]             	   = UOP_GPU_FCLZ;
	m_fp_uop_table[GPU_CLZ64]             	 = UOP_GPU_FCLZ64;
	m_fp_uop_table[GPU_CNOT]             		 = UOP_GPU_FCNOT;
	m_fp_uop_table[GPU_CNOT64]             	 = UOP_GPU_FCNOT64;
	m_fp_uop_table[GPU_COPYSIGN]           	 = UOP_GPU_FCOPYSIGN;
	m_fp_uop_table[GPU_COPYSIGN64]         	 = UOP_GPU_FCOPYSIGN64;
	m_fp_uop_table[GPU_COS]             	   = UOP_GPU_FCOS;
	m_fp_uop_table[GPU_CVT]             	   = UOP_GPU_FCVT;
	m_fp_uop_table[GPU_CVT64]             	 = UOP_GPU_FCVT64;
	m_fp_uop_table[GPU_CVTA]             		 = UOP_GPU_FCVTA;
	m_fp_uop_table[GPU_CVTA64]            	 = UOP_GPU_FCVTA64;
	m_fp_uop_table[GPU_DIV]             	   = UOP_GPU_FDIV;
	m_fp_uop_table[GPU_DIV64]             	 = UOP_GPU_FDIV64;
	m_fp_uop_table[GPU_EX2]             	   = UOP_GPU_FEX2;
	m_fp_uop_table[GPU_EXIT]             		 = UOP_GPU_FEXIT;
	m_fp_uop_table[GPU_FMA]             	   = UOP_GPU_FFMA;
	m_fp_uop_table[GPU_FMA64]             	 = UOP_GPU_FFMA64;
	m_fp_uop_table[GPU_ISSPACEP]           	 = UOP_GPU_FISSPACEP;
	m_fp_uop_table[GPU_LD]             		   = UOP_GPU_FLD;
	m_fp_uop_table[GPU_LD64]             		 = UOP_GPU_FLD64;
	m_fp_uop_table[GPU_LDU]             		 = UOP_GPU_FLDU;
	m_fp_uop_table[GPU_LDU64]             	 = UOP_GPU_FLDU64;
	m_fp_uop_table[GPU_LG2]               	 = UOP_GPU_FLG2;
	m_fp_uop_table[GPU_MAD24]             	 = UOP_GPU_FMAD24;
	m_fp_uop_table[GPU_MAD]             	   = UOP_GPU_FMAD;
	m_fp_uop_table[GPU_MAD64]             	 = UOP_GPU_FMAD64;
	m_fp_uop_table[GPU_MAX]             	   = UOP_GPU_FMAX;
	m_fp_uop_table[GPU_MAX64]             	 = UOP_GPU_FMAX64;
	m_fp_uop_table[GPU_MEMBAR]             	 = UOP_GPU_FMEMBAR;
	m_fp_uop_table[GPU_MIN]             		 = UOP_GPU_FMIN;
	m_fp_uop_table[GPU_MIN64]             	 = UOP_GPU_FMIN64;
	m_fp_uop_table[GPU_MOV]             	   = UOP_GPU_FMOV;
	m_fp_uop_table[GPU_MOV64]             	 = UOP_GPU_FMOV64;
	m_fp_uop_table[GPU_MUL24]             	 = UOP_GPU_FMUL24;
	m_fp_uop_table[GPU_MUL]             		 = UOP_GPU_FMUL;
	m_fp_uop_table[GPU_MUL64]             	 = UOP_GPU_FMUL64;
	m_fp_uop_table[GPU_NEG]             		 = UOP_GPU_FNEG;
	m_fp_uop_table[GPU_NEG64]             	 = UOP_GPU_FNEG64;
	m_fp_uop_table[GPU_NOT]             		 = UOP_GPU_FNOT;
	m_fp_uop_table[GPU_NOT64]             	 = UOP_GPU_FNOT64;
	m_fp_uop_table[GPU_OR]             	  	 = UOP_GPU_FOR;
	m_fp_uop_table[GPU_OR64]             		 = UOP_GPU_FOR64;
	m_fp_uop_table[GPU_PMEVENT]            	 = UOP_GPU_FPMEVENT;
	m_fp_uop_table[GPU_POPC]             		 = UOP_GPU_FPOPC;
	m_fp_uop_table[GPU_POPC64]             	 = UOP_GPU_FPOPC64;
	m_fp_uop_table[GPU_PREFETCH]           	 = UOP_GPU_FPREFETCH;
	m_fp_uop_table[GPU_PREFETCHU]          	 = UOP_GPU_FPREFETCHU;
	m_fp_uop_table[GPU_PRMT]             		 = UOP_GPU_FPRMT;
	m_fp_uop_table[GPU_RCP]             	   = UOP_GPU_FRCP;
	m_fp_uop_table[GPU_RCP64]             	 = UOP_GPU_FRCP64;
	m_fp_uop_table[GPU_RED]             	   = UOP_GPU_FRED;
	m_fp_uop_table[GPU_RED64]             	 = UOP_GPU_FRED64;
	m_fp_uop_table[GPU_REM]             	   = UOP_GPU_FREM;
	m_fp_uop_table[GPU_REM64]             	 = UOP_GPU_FREM64;
	m_fp_uop_table[GPU_RET]             	   = UOP_GPU_FRET;
	m_fp_uop_table[GPU_RSQRT]             	 = UOP_GPU_FRSQRT;
	m_fp_uop_table[GPU_RSQRT64]            	 = UOP_GPU_FRSQRT64;
	m_fp_uop_table[GPU_SAD]             	   = UOP_GPU_FSAD;
	m_fp_uop_table[GPU_SAD64]             	 = UOP_GPU_FSAD64;
	m_fp_uop_table[GPU_SELP]             		 = UOP_GPU_FSELP;
	m_fp_uop_table[GPU_SELP64]             	 = UOP_GPU_FSELP64;
	m_fp_uop_table[GPU_SET]             	   = UOP_GPU_FSET;
	m_fp_uop_table[GPU_SET64]             	 = UOP_GPU_FSET64;
	m_fp_uop_table[GPU_SETP]             		 = UOP_GPU_FSETP;
	m_fp_uop_table[GPU_SETP64]             	 = UOP_GPU_FSETP64;
	m_fp_uop_table[GPU_SHL]             	   = UOP_GPU_FSHL;
	m_fp_uop_table[GPU_SHL64]             	 = UOP_GPU_FSHL64;
	m_fp_uop_table[GPU_SHR]             		 = UOP_GPU_FSHR;
	m_fp_uop_table[GPU_SHR64]             	 = UOP_GPU_FSHR64;
	m_fp_uop_table[GPU_SIN]             	   = UOP_GPU_FSIN;
	m_fp_uop_table[GPU_SLCT]             		 = UOP_GPU_FSLCT;
	m_fp_uop_table[GPU_SLCT64]            	 = UOP_GPU_FSLCT64;
	m_fp_uop_table[GPU_SQRT]             		 = UOP_GPU_FSQRT;
	m_fp_uop_table[GPU_SQRT64]            	 = UOP_GPU_FSQRT64;
	m_fp_uop_table[GPU_ST]             	  	 = UOP_GPU_FST;
	m_fp_uop_table[GPU_ST64]             		 = UOP_GPU_FST64;
	m_fp_uop_table[GPU_SUB]             		 = UOP_GPU_FSUB;
	m_fp_uop_table[GPU_SUB64]             	 = UOP_GPU_FSUB64;
	m_fp_uop_table[GPU_SUBC]             		 = UOP_GPU_FSUBC;
	m_fp_uop_table[GPU_SULD]             		 = UOP_GPU_FSULD;
	m_fp_uop_table[GPU_SULD64]             	 = UOP_GPU_FSULD64;
	m_fp_uop_table[GPU_SURED]             	 = UOP_GPU_FSURED;
	m_fp_uop_table[GPU_SURED64]            	 = UOP_GPU_FSURED64;
	m_fp_uop_table[GPU_SUST]             		 = UOP_GPU_FSUST;
	m_fp_uop_table[GPU_SUST64]             	 = UOP_GPU_FSUST64;
	m_fp_uop_table[GPU_SUQ]             		 = UOP_GPU_FSUQ;
  m_fp_uop_table[GPU_TESTP]             	 = UOP_GPU_FTESTP;
  m_fp_uop_table[GPU_TESTP64]            	 = UOP_GPU_FTESTP64;
  m_fp_uop_table[GPU_TEX]             		 = UOP_GPU_FTEX;
  m_fp_uop_table[GPU_TLD4]             		 = UOP_GPU_FTLD4;
  m_fp_uop_table[GPU_TXQ]             		 = UOP_GPU_FTXQ;
  m_fp_uop_table[GPU_TRAP]             		 = UOP_GPU_FTRAP;
  m_fp_uop_table[GPU_VABSDIFF]             = UOP_GPU_FVABSDIFF;
  m_fp_uop_table[GPU_VADD]             		 = UOP_GPU_FVADD;
  m_fp_uop_table[GPU_VMAD]             		 = UOP_GPU_FVMAD;
  m_fp_uop_table[GPU_VMAX]             		 = UOP_GPU_FVMAX;
  m_fp_uop_table[GPU_VMIN]             		 = UOP_GPU_FVMIN;
  m_fp_uop_table[GPU_VSET]             		 = UOP_GPU_FVSET;
  m_fp_uop_table[GPU_VSHL]             		 = UOP_GPU_FVSHL;
  m_fp_uop_table[GPU_VSHR]             		 = UOP_GPU_FVSHR;
  m_fp_uop_table[GPU_VSUB]             		 = UOP_GPU_FVSUB;
  m_fp_uop_table[GPU_VOTE]             		 = UOP_GPU_FVOTE;
  m_fp_uop_table[GPU_XOR]             		 = UOP_GPU_FXOR;
  m_fp_uop_table[GPU_XOR64]             	 = UOP_GPU_FXOR64;
  m_fp_uop_table[GPU_RECONVERGE]           = UOP_GPU_FRECONVERGE;
  m_fp_uop_table[GPU_PHI]             		 = UOP_GPU_FPHI;
  m_fp_uop_table[GPU_MEM_LD_GM]            = UOP_FADD;
  m_fp_uop_table[GPU_MEM_LD_LM]            = UOP_FADD;
  m_fp_uop_table[GPU_MEM_LD_SM]            = UOP_FADD;
  m_fp_uop_table[GPU_MEM_LD_CM]            = UOP_FADD;
  m_fp_uop_table[GPU_MEM_LD_TM]            = UOP_FADD;
  m_fp_uop_table[GPU_MEM_LD_PM]            = UOP_FADD;
  m_fp_uop_table[GPU_MEM_ST_GM]            = UOP_FADD;
  m_fp_uop_table[GPU_MEM_ST_LM]            = UOP_FADD;
  m_fp_uop_table[GPU_MEM_ST_SM]            = UOP_FADD;
  m_fp_uop_table[GPU_DATA_XFER_GM]         = UOP_FADD;
  m_fp_uop_table[GPU_DATA_XFER_LM]         = UOP_FADD;
  m_fp_uop_table[GPU_DATA_XFER_SM]         = UOP_FADD;
}


const char* gpu_decoder_c::g_tr_reg_names[MAX_TR_REG] = {
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
  "x87",
  "r_status_flags",
  "rdf",
};


const char* gpu_decoder_c::g_tr_opcode_names[MAX_TR_OPCODE_NAME] = {
  "GPU_INVALID",
  "GPU_ABS",
  "GPU_ABS64",
  "GPU_ADD", 
  "GPU_ADD64", 
	"GPU_ADDC",
	"GPU_AND",
	"GPU_AND64",
	"GPU_ATOM",
	"GPU_ATOM64",
	"GPU_BAR",
	"GPU_BFE",
	"GPU_BFE64",
	"GPU_BFI",
	"GPU_BFI64",
	"GPU_BFIND",
	"GPU_BFIND64",
	"GPU_BRA",
	"GPU_BREV",
	"GPU_BREV64",
	"GPU_BRKPT",
	"GPU_CALL",
	"GPU_CLZ",
	"GPU_CLZ64",
	"GPU_CNOT",
	"GPU_CNOT64",
	"GPU_COPYSIGN",
	"GPU_COPYSIGN64",
	"GPU_COS",
	"GPU_CVT",
	"GPU_CVT64",
	"GPU_CVTA",
	"GPU_CVTA64",
	"GPU_DIV",
	"GPU_DIV64",
	"GPU_EX2",
	"GPU_EXIT",
	"GPU_FMA",
	"GPU_FMA64",
	"GPU_ISSPACEP",
	"GPU_LD",
	"GPU_LD64",
	"GPU_LDU",
	"GPU_LDU64",
	"GPU_LG2",
	"GPU_MAD24",
	"GPU_MAD",
	"GPU_MAD64",
	"GPU_MAX",
	"GPU_MAX64",
	"GPU_MEMBAR",
	"GPU_MIN",
	"GPU_MIN64",
	"GPU_MOV",
	"GPU_MOV64",
	"GPU_MUL24",
	"GPU_MUL",
	"GPU_MUL64",
	"GPU_NEG",
	"GPU_NEG64",
	"GPU_NOT",
	"GPU_NOT64",
	"GPU_OR",
	"GPU_OR64",
	"GPU_PMEVENT",
	"GPU_POPC",
	"GPU_POPC64",
	"GPU_PREFETCH",
	"GPU_PREFETCHU",
	"GPU_PRMT",
	"GPU_RCP",
	"GPU_RCP64",
	"GPU_RED",
	"GPU_RED64",
	"GPU_REM",
	"GPU_REM64",
	"GPU_RET",
	"GPU_RSQRT",
	"GPU_RSQRT64",
	"GPU_SAD",
	"GPU_SAD64",
	"GPU_SELP",
	"GPU_SELP64",
	"GPU_SET",
	"GPU_SET64",
	"GPU_SETP",
	"GPU_SETP64",
	"GPU_SHL",
	"GPU_SHL64",
	"GPU_SHR",
	"GPU_SHR64",
	"GPU_SIN",
	"GPU_SLCT",
	"GPU_SLCT64",
	"GPU_SQRT",
	"GPU_SQRT64",
	"GPU_ST",
	"GPU_ST64",
	"GPU_SUB",
	"GPU_SUB64",
	"GPU_SUBC",
	"GPU_SULD",
	"GPU_SULD64",
	"GPU_SURED",
	"GPU_SURED64",
	"GPU_SUST",
	"GPU_SUST64",
	"GPU_SUQ",
  "GPU_TESTP",
  "GPU_TESTP64",
  "GPU_TEX",
  "GPU_TLD4",
  "GPU_TXQ",
  "GPU_TRAP",
  "GPU_VABSDIFF",
  "GPU_VADD",
  "GPU_VMAD",
  "GPU_VMAX",
  "GPU_VMIN",
  "GPU_VSET",
  "GPU_VSHL",
  "GPU_VSHR",
  "GPU_VSUB",
  "GPU_VOTE",
  "GPU_XOR",
  "GPU_XOR64",
  "GPU_RECONVERGE",
  "GPU_PHI",
  "GPU_MEM_LD_GM",
  "GPU_MEM_LD_LM",
  "GPU_MEM_LD_SM",
  "GPU_MEM_LD_CM",
  "GPU_MEM_LD_TM",
  "GPU_MEM_LD_PM",
  "GPU_MEM_LDU_GM",
  "GPU_MEM_ST_GM",
  "GPU_MEM_ST_LM",
  "GPU_MEM_ST_SM",
  "GPU_DATA_XFER_GM",
  "GPU_DATA_XFER_LM",
  "GPU_DATA_XFER_SM",
};



const char* gpu_decoder_c::g_tr_cf_names[10] = {
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


const char *gpu_decoder_c::g_optype_names[37] = {
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


const char *gpu_decoder_c::g_mem_type_names[20] = {
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


