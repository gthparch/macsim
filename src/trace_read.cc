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
 * File         : trace_read.cc
 * Author       : HPArch Research Group
 * Date         : 
 * SVN          : $Id: trace_read.cc 912 2009-11-20 19:09:21Z kacear $
 * Description  : Trace handling class
 *********************************************************************************************/


#include <iostream>
#include <set>

#include "assert_macros.h"
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

#include "trace_read_cpu.h"
#include "trace_read_a64.h"
#include "trace_read_gpu.h"

#include "all_knobs.h"

///////////////////////////////////////////////////////////////////////////////////////////////


#define DEBUG(args...)   _DEBUG(*KNOB(KNOB_DEBUG_TRACE_READ), ## args)


///////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////


trace_info_s::trace_info_s() { }
trace_info_s::~trace_info_s() { }


///////////////////////////////////////////////////////////////////////////////////////////////


reg_info_s::reg_info_s()
{
  m_reg  = 0;
  m_type = INT_REG;
  m_id   = 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////


trace_uop_s::trace_uop_s()
{
  m_opcode          = 0;
  m_op_type         = UOP_INV;
  m_mem_type        = NOT_MEM;
  m_cf_type         = NOT_CF;
  m_bar_type        = NOT_BAR;
  m_num_dest_regs   = 0;
  m_num_src_regs    = 0;
  m_mem_size        = 0;
  m_inst_size       = 0;
  m_addr            = 0;
  m_va              = 0;
  m_actual_taken    = false;
  m_target          = 0;
  m_npc             = 0;
  m_pin_2nd_mem     = false;
  m_info            = NULL;
  m_rep_uop_num     = 0;
  m_eom             = false;
  m_alu_uop         = false;
  m_active_mask     = 0;
  m_taken_mask      = 0;
  m_reconverge_addr = 0;
  m_mul_mem_uops    = false;

  m_hmc_inst        =HMC_NONE;
}


///////////////////////////////////////////////////////////////////////////////////////////////


/**
 * Constructor
 */
trace_read_c::trace_read_c(macsim_c* simBase, ofstream* dprint_output)
{
  m_simBase = simBase;
  m_dprint_count = 0;
  m_dprint_output = dprint_output;
}


/**
 * Destructor
 */
trace_read_c::~trace_read_c()
{
}


/**
 * This function is called once for each thread/warp when the thread/warp is started.
 * The simulator does read-ahead of the trace file to get next pc address
 * @param core_id - core id
 * @param sim_thread_id - thread id
 */
void trace_read_c::setup_trace(int core_id, int sim_thread_id)
{
  core_c* core = m_simBase->m_core_pointers[core_id];
  thread_s* thread_trace_info = core->get_trace_info(sim_thread_id);

  // read one instruction from the trace file to get next instruction. Always one instruction
  // will be read ahead to get next pc address
  if (core->m_running_thread_num) {
    gzread(thread_trace_info->m_trace_file, thread_trace_info->m_prev_trace_info, 
        m_trace_size);

    if (*KNOB(KNOB_DEBUG_TRACE_READ)) {
      dprint_inst(thread_trace_info->m_prev_trace_info, core_id, sim_thread_id);
    }
  }
}

/**
 * @param core_id - core id
 * @param trace_info - trace information
 * @param sim_thread_id - thread id
 * @param inst_read - set true if instruction read successful
 * @see get_uops_from_traces
 */
bool trace_read_c::read_trace(int core_id, void *trace_info, int sim_thread_id, 
    bool *inst_read)
{
  core_c* core = m_simBase->m_core_pointers[core_id];
  int bytes_read;
  thread_s *thread_trace_info = core->get_trace_info(sim_thread_id);
  process_s* process;

  *inst_read = false;

  while (true) {
    if(core->m_running_thread_num && !thread_trace_info->m_trace_ended) {
      ///
      /// First read chunk of instructions, instead of reading one by one, then read from
      /// buffer using buffer_index. When all instruction have been read (by checking 
      /// buffer index), read another chunk, and so on.
      ///
      if (thread_trace_info->m_buffer_index == 0) {
        thread_trace_info->m_buffer_index_max  = gzread(thread_trace_info->m_trace_file, 
            thread_trace_info->m_buffer, m_trace_size*k_trace_buffer_size);
        thread_trace_info->m_buffer_index_max /= m_trace_size;
        thread_trace_info->m_buffer_exhausted  = false;

//        if (thread_trace_info->m_buffer_index_max < k_trace_buffer_size) {
//          gzclose(thread_trace_info->m_trace_file);
//        }
      }

      memcpy(trace_info, 
          &(thread_trace_info->m_buffer[m_trace_size * thread_trace_info->m_buffer_index]), 
          m_trace_size);
      thread_trace_info->m_buffer_index = (thread_trace_info->m_buffer_index + 1) % 
        k_trace_buffer_size; 


      if (thread_trace_info->m_buffer_index >= thread_trace_info->m_buffer_index_max) {
        bytes_read = 0;
      }
      else 
        bytes_read = m_trace_size;

      if (thread_trace_info->m_buffer_index == 0) {
        thread_trace_info->m_buffer_exhausted = true;
      }


      if (*KNOB(KNOB_DEBUG_TRACE_READ))
        dprint_inst(trace_info, core_id, sim_thread_id);


      ///
      /// When an application is created, only the main thread will be allocated to the
      /// trace scheduler. When the main thread is executed in trace_read, allocate
      /// all other threads. Each thread can start after certain instruction relative to
      /// the main thread (in case of spawned threads).
      ///
      if (thread_trace_info->m_main_thread) {
        ++thread_trace_info->m_inst_count;
        process = thread_trace_info->m_process;
        int no_created_thread = process->m_no_of_threads_created;

        // create other threads which are not main-thread.
        while((process->m_no_of_threads_created < process->m_no_of_threads &&
              thread_trace_info->m_inst_count == 
              process->m_thread_start_info[no_created_thread].m_inst_count) || 
            (process->m_no_of_threads_created < process->m_no_of_threads &&
             process->m_thread_start_info[no_created_thread].m_inst_count == 0)) {

          // create non-main threads here
          m_simBase->m_process_manager->create_thread_node(process, process->m_no_of_threads_created++, 
              false);
          m_simBase->m_process_manager->sim_thread_schedule(false);
        }
      }

      // normal trace read
      if (bytes_read == m_trace_size) {
        *inst_read = true;
        break;
      } 
      // trace reading error
      else {
        if (bytes_read == 0) {
          if (!core->m_thread_finished[sim_thread_id]) { 
            thread_trace_info->m_trace_ended = true;

            DEBUG("trace ended core_id:%d thread_id:%d\n", core_id, sim_thread_id);
          } 
          else {
            return false; 
          }
        } 
        else if (bytes_read == -1) {
          report("error reading trace " << thread_trace_info->m_thread_id);
          return false;
        } 
        else {
          report("core_id:" << core_id << " error reading trace");
          return false;
        }
      }
    } // end if( thread_info )
    else {
      break;
    }
  } // end while( true )

  return 1;
}

/**
 * Convert instruction information (from hash table) to trace uop
 * @param info - instruction information from the hash table
 * @param trace_uop - MacSim trace format
 */
void trace_read_c::convert_info_uop(inst_info_s *info, trace_uop_s *trace_uop)
{
  trace_uop->m_op_type       = info->m_table_info->m_op_type;
  trace_uop->m_mem_type      = info->m_table_info->m_mem_type;
  trace_uop->m_cf_type       = info->m_table_info->m_cf_type;
  trace_uop->m_bar_type      = info->m_table_info->m_bar_type;
  trace_uop->m_num_dest_regs = info->m_table_info->m_num_dest_regs;
  trace_uop->m_num_src_regs  = info->m_table_info->m_num_src_regs;
  trace_uop->m_mem_size      = info->m_table_info->m_mem_size;
  trace_uop->m_addr          = info->m_addr;
  trace_uop->m_inst_size     = info->m_trace_info.m_inst_size;
  trace_uop->m_num_src_regs  = info->m_table_info->m_num_src_regs;
  trace_uop->m_num_dest_regs = info->m_table_info->m_num_dest_regs;

  for (int ii = 0; ii < info->m_table_info->m_num_src_regs; ++ii) {
    trace_uop->m_srcs[ii].m_type = INT_REG;
    trace_uop->m_srcs[ii].m_id   = info->m_srcs[ii].m_id;
    trace_uop->m_srcs[ii].m_reg  = info->m_srcs[ii].m_reg;
  }

  for (int ii = 0; ii < info->m_table_info->m_num_dest_regs; ++ii) {
    trace_uop->m_dests[ii].m_id   = info->m_dests[ii].m_id ;
    trace_uop->m_dests[ii].m_reg  = info->m_dests[ii].m_reg;
    ASSERT(trace_uop->m_dests[ii].m_reg < NUM_REG_IDS);
    trace_uop->m_dests[ii].m_type = INT_REG;
  }

  trace_uop->m_pin_2nd_mem = info->m_trace_info.m_second_mem;
}


/**
 * Convert MacSim trace to instruction information (hash table)
 * @param t_uop - MacSim trace
 * @param info - instruction information in hash table
 */
void trace_read_c::convert_t_uop_to_info(trace_uop_s *t_uop, inst_info_s *info)
{
  info->m_table_info->m_op_type       = t_uop->m_op_type;
  info->m_table_info->m_mem_type      = t_uop->m_mem_type;
  info->m_table_info->m_cf_type       = t_uop->m_cf_type;
  info->m_table_info->m_bar_type      = t_uop->m_bar_type;
  info->m_table_info->m_num_dest_regs = t_uop->m_num_dest_regs;
  info->m_table_info->m_num_src_regs  = t_uop->m_num_src_regs;
  info->m_table_info->m_mem_size      = t_uop->m_mem_size;                              
  info->m_table_info->m_type          = 0;                              
  info->m_table_info->m_mask          = 0;                             
  info->m_addr                        = t_uop->m_addr;
  info->m_trace_info.m_inst_size      = (t_uop->m_inst_size); 
  info->m_table_info->m_num_src_regs  = t_uop->m_num_src_regs;
  info->m_table_info->m_num_dest_regs = t_uop->m_num_dest_regs;

  ASSERTM(t_uop->m_num_dest_regs <= MAX_DESTS, "num_dest_regs:%d ", t_uop->m_num_dest_regs);
  ASSERTM(t_uop->m_num_src_regs <= MAX_SRCS, "num_src_regs:%d  ", t_uop->m_num_src_regs);

  for (int ii = 0; ii < info->m_table_info->m_num_src_regs; ++ii) {
    info->m_srcs[ii].m_type = INT_REG;
    info->m_srcs[ii].m_id   = t_uop->m_srcs[ii].m_id;
    info->m_srcs[ii].m_reg  = t_uop->m_srcs[ii].m_reg;
  }

  // only one destination - temporary that is going to be read by the second t_uop
  for (int ii = 0; ii < info->m_table_info->m_num_dest_regs; ++ii) {
    info->m_table_info->m_num_dest_regs = 1;
    info->m_dests[ii].m_type = INT_REG;
    info->m_dests[ii].m_id   = t_uop->m_dests[ii].m_id;
    info->m_dests[ii].m_reg  = t_uop->m_dests[ii].m_reg;
  }

  info->m_trace_info.m_second_mem = t_uop->m_pin_2nd_mem;
}





const char* trace_read_c::g_tr_reg_names[MAX_TR_REG] = {
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


const char* trace_read_c::g_tr_opcode_names[MAX_TR_OPCODE_NAME] = {
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

const char* trace_read_c::g_tr_cf_names[10] = {
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


const char *trace_read_c::g_optype_names[37] = {
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


const char *trace_read_c::g_mem_type_names[20] = {
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


trace_reader_wrapper_c::trace_reader_wrapper_c(macsim_c* simBase)
{
  m_simBase = simBase;

  // initialization
  m_dprint_output = new ofstream(KNOB(KNOB_STATISTICS_OUT_DIRECTORY)->getValue() + "/trace_debug.out");
  
  if (KNOB(KNOB_LARGE_CORE_TYPE)->getValue() == "x86")
    m_cpu_decoder = new cpu_decoder_c(simBase, m_dprint_output);
  else if (KNOB(KNOB_LARGE_CORE_TYPE)->getValue() == "a64")
    m_cpu_decoder = new a64_decoder_c(simBase, m_dprint_output);
  else {
    ASSERTM(0, "Wrong core type %s\n", KNOB(KNOB_LARGE_CORE_TYPE)->getValue().c_str());
  }

  m_cpu_decoder->init_pin_convert();

  m_gpu_decoder = new gpu_decoder_c(simBase, m_dprint_output);
}


trace_reader_wrapper_c::trace_reader_wrapper_c()
{
  m_dprint_output->close();
  delete m_dprint_output;
}


trace_reader_wrapper_c::~trace_reader_wrapper_c()
{
  delete m_cpu_decoder;
  delete m_gpu_decoder;
}

void trace_reader_wrapper_c::setup_trace(int core_id, int sim_thread_id, bool gpu_sim)
{
  if (gpu_sim)
    m_gpu_decoder->setup_trace(core_id, sim_thread_id);
  else
    m_cpu_decoder->setup_trace(core_id, sim_thread_id);
}
    
bool trace_reader_wrapper_c::get_uops_from_traces(int core_id, uop_c *uop, int sim_thread_id, 
    bool gpu_sim)
{
  if (gpu_sim)
    return m_gpu_decoder->get_uops_from_traces(core_id, uop, sim_thread_id);
  else
    return m_cpu_decoder->get_uops_from_traces(core_id, uop, sim_thread_id);
}
    

void trace_reader_wrapper_c::pre_read_trace(thread_s* trace_info)
{
  m_gpu_decoder->pre_read_trace(trace_info);
}



