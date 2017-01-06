#include "trace_read_igpu.h"
#include "process_manager.h"
#include "assert_macros.h"
#include "debug_macros.h"
#include "utils.h"
#include "all_knobs.h"

#define DEBUG(args...)   _DEBUG(*KNOB(KNOB_DEBUG_TRACE_READ), ## args)
#define DEBUG_CORE(m_core_id, args...)                            \
  if (m_core_id == *m_simBase->m_knobs->KNOB_DEBUG_CORE_ID) {     \
    _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_TRACE_READ, ## args);  \
  }

void igpu_decoder_c::init_pin_convert()
{
  m_int_uop_table[IGPU_INS_INVALID]        = UOP_INV;
  m_int_uop_table[IGPU_INS_3DNOW]          = UOP_IADD;
  m_int_uop_table[IGPU_INS_AES]            = UOP_IMUL;
  m_int_uop_table[IGPU_INS_AVX]            = UOP_FADD;
  m_int_uop_table[IGPU_INS_AVX2]           = UOP_FADD;
  m_int_uop_table[IGPU_INS_AVX2GATHER]     = UOP_FADD;
  m_int_uop_table[IGPU_INS_BDW]            = UOP_FADD;
  m_int_uop_table[IGPU_INS_BINARY]         = UOP_IADD;
  m_int_uop_table[IGPU_INS_BITBYTE]        = UOP_BYTE;
  m_int_uop_table[IGPU_INS_BMI1]           = UOP_BYTE;
  m_int_uop_table[IGPU_INS_BMI2]           = UOP_BYTE;
  m_int_uop_table[IGPU_INS_BROADCAST]      = UOP_IADD;
  m_int_uop_table[IGPU_INS_CALL]           = UOP_IADD;
  m_int_uop_table[IGPU_INS_CMOV]           = UOP_CMOV;
  m_int_uop_table[IGPU_INS_COND_BR]        = UOP_IADD;
  m_int_uop_table[IGPU_INS_CONVERT]        = UOP_IADD;
  m_int_uop_table[IGPU_INS_DATAXFER]       = UOP_IADD;
  m_int_uop_table[IGPU_INS_DECIMAL]        = UOP_IADD;
  m_int_uop_table[IGPU_INS_FCMOV]          = UOP_FADD;
  m_int_uop_table[IGPU_INS_FLAGOP]         = UOP_IADD;
  m_int_uop_table[IGPU_INS_FMA4]           = UOP_IADD;
  m_int_uop_table[IGPU_INS_INTERRUPT]      = UOP_IADD;
  m_int_uop_table[IGPU_INS_IO]             = UOP_IADD;
  m_int_uop_table[IGPU_INS_IOSTRINGOP]     = UOP_IMUL;
  m_int_uop_table[IGPU_INS_LOGICAL]        = UOP_LOGIC;
  m_int_uop_table[IGPU_INS_LZCNT]          = UOP_LOGIC;
  m_int_uop_table[IGPU_INS_MISC]           = UOP_IADD;
  m_int_uop_table[IGPU_INS_MMX]            = UOP_FADD;
  m_int_uop_table[IGPU_INS_NOP]            = UOP_NOP;
  m_int_uop_table[IGPU_INS_PCLMULQDQ]      = UOP_IMUL;
  m_int_uop_table[IGPU_INS_POP]            = UOP_IADD;
  m_int_uop_table[IGPU_INS_PREFETCH]       = UOP_IADD;
  m_int_uop_table[IGPU_INS_PUSH]           = UOP_IADD;
  m_int_uop_table[IGPU_INS_RDRAND]         = UOP_IADD;
  m_int_uop_table[IGPU_INS_RDSEED]         = UOP_IADD;
  m_int_uop_table[IGPU_INS_RDWRFSGS]       = UOP_IADD;
  m_int_uop_table[IGPU_INS_RET]            = UOP_IADD;
  m_int_uop_table[IGPU_INS_ROTATE]         = UOP_SHIFT;
  m_int_uop_table[IGPU_INS_SEGOP]          = UOP_IADD;
  m_int_uop_table[IGPU_INS_SEMAPHORE]      = UOP_IADD;
  m_int_uop_table[IGPU_INS_SHIFT]          = UOP_SHIFT;
  m_int_uop_table[IGPU_INS_SSE]            = UOP_FADD;
  m_int_uop_table[IGPU_INS_STRINGOP]       = UOP_IADD;
  m_int_uop_table[IGPU_INS_STTNI]          = UOP_IADD;
  m_int_uop_table[IGPU_INS_SYSCALL]        = UOP_IADD;
  m_int_uop_table[IGPU_INS_SYSRET]         = UOP_IADD;
  m_int_uop_table[IGPU_INS_SYSTEM]         = UOP_IADD;
  m_int_uop_table[IGPU_INS_TBM]            = UOP_IADD;
  m_int_uop_table[IGPU_INS_UNCOND_BR]      = UOP_IADD;
  m_int_uop_table[IGPU_INS_VFMA]           = UOP_IADD;
  m_int_uop_table[IGPU_INS_VTX]            = UOP_IADD;
  m_int_uop_table[IGPU_INS_WIDENOP]        = UOP_NOP;
  m_int_uop_table[IGPU_INS_X87_ALU]        = UOP_FADD;
  m_int_uop_table[IGPU_INS_XOP]            = UOP_IADD; // new
  m_int_uop_table[IGPU_INS_XSAVE]          = UOP_IMUL;
  m_int_uop_table[IGPU_INS_XSAVEOPT]       = UOP_IMUL;
	m_int_uop_table[IGPU_INS_TR_MUL]         = UOP_IMUL;            
  m_int_uop_table[IGPU_INS_TR_DIV]         = UOP_IMUL;
  m_int_uop_table[IGPU_INS_TR_FMUL]        = UOP_FMUL;
  m_int_uop_table[IGPU_INS_TR_FDIV]        = UOP_FDIV;
  m_int_uop_table[IGPU_INS_TR_NOP]         = UOP_NOP;
  m_int_uop_table[IGPU_INS_PREFETCH_NTA]   = UOP_IADD;
  m_int_uop_table[IGPU_INS_PREFETCH_T0]    = UOP_IADD;
  m_int_uop_table[IGPU_INS_PREFETCH_T1]    = UOP_IADD;
  m_int_uop_table[IGPU_INS_PREFETCH_T2]    = UOP_IADD;
  m_int_uop_table[IGPU_INS_GPU_EN]         = UOP_IADD; // new
  m_int_uop_table[IGPU_INS_CPU_MEM_EXT_OP] = UOP_IADD; // new

  m_fp_uop_table[IGPU_INS_INVALID]        = UOP_INV;
  m_fp_uop_table[IGPU_INS_3DNOW]          = UOP_FADD;
  m_fp_uop_table[IGPU_INS_AES]            = UOP_FMUL;
  m_fp_uop_table[IGPU_INS_AVX]            = UOP_FADD;
  m_fp_uop_table[IGPU_INS_AVX2]           = UOP_FADD;
  m_fp_uop_table[IGPU_INS_AVX2GATHER]     = UOP_FADD;
  m_fp_uop_table[IGPU_INS_BDW]            = UOP_FADD;
  m_fp_uop_table[IGPU_INS_BINARY]         = UOP_FADD;
  m_fp_uop_table[IGPU_INS_BITBYTE]        = UOP_BYTE;
  m_fp_uop_table[IGPU_INS_BMI1]           = UOP_BYTE;
  m_fp_uop_table[IGPU_INS_BMI2]           = UOP_BYTE;
  m_fp_uop_table[IGPU_INS_BROADCAST]      = UOP_FADD;
  m_fp_uop_table[IGPU_INS_CALL]           = UOP_FADD;
  m_fp_uop_table[IGPU_INS_CMOV]           = UOP_CMOV;
  m_fp_uop_table[IGPU_INS_COND_BR]        = UOP_FADD;
  m_fp_uop_table[IGPU_INS_CONVERT]        = UOP_FADD;
  m_fp_uop_table[IGPU_INS_DATAXFER]       = UOP_FADD;
  m_fp_uop_table[IGPU_INS_DECIMAL]        = UOP_FADD;
  m_fp_uop_table[IGPU_INS_FCMOV]          = UOP_FADD;
  m_fp_uop_table[IGPU_INS_FLAGOP]         = UOP_FADD;
  m_fp_uop_table[IGPU_INS_FMA4]           = UOP_IADD;
  m_fp_uop_table[IGPU_INS_INTERRUPT]      = UOP_FADD;
  m_fp_uop_table[IGPU_INS_IO]             = UOP_FADD;
  m_fp_uop_table[IGPU_INS_IOSTRINGOP]     = UOP_FMUL;
  m_fp_uop_table[IGPU_INS_LOGICAL]        = UOP_LOGIC;
  m_fp_uop_table[IGPU_INS_LZCNT]          = UOP_LOGIC;
  m_fp_uop_table[IGPU_INS_MISC]           = UOP_FADD;
  m_fp_uop_table[IGPU_INS_MMX]            = UOP_FADD;
  m_fp_uop_table[IGPU_INS_NOP]            = UOP_NOP;
  m_fp_uop_table[IGPU_INS_PCLMULQDQ]      = UOP_FMUL;
  m_fp_uop_table[IGPU_INS_POP]            = UOP_FADD;
  m_fp_uop_table[IGPU_INS_PREFETCH]       = UOP_FADD;
  m_fp_uop_table[IGPU_INS_PUSH]           = UOP_FADD;
  m_fp_uop_table[IGPU_INS_RDRAND]         = UOP_IADD;
  m_fp_uop_table[IGPU_INS_RDSEED]         = UOP_IADD;
  m_fp_uop_table[IGPU_INS_RDWRFSGS]       = UOP_IADD;
  m_fp_uop_table[IGPU_INS_RET]            = UOP_FADD;
  m_fp_uop_table[IGPU_INS_ROTATE]         = UOP_SHIFT;
  m_fp_uop_table[IGPU_INS_SEGOP]          = UOP_FADD;
  m_fp_uop_table[IGPU_INS_SEMAPHORE]      = UOP_FADD;
  m_fp_uop_table[IGPU_INS_SHIFT]          = UOP_SHIFT;
  m_fp_uop_table[IGPU_INS_SSE]            = UOP_FADD;
  m_fp_uop_table[IGPU_INS_STRINGOP]       = UOP_FADD;
  m_fp_uop_table[IGPU_INS_STTNI]          = UOP_FADD;
  m_fp_uop_table[IGPU_INS_SYSCALL]        = UOP_FADD;
  m_fp_uop_table[IGPU_INS_SYSRET]         = UOP_FADD;
  m_fp_uop_table[IGPU_INS_SYSTEM]         = UOP_FADD;
  m_fp_uop_table[IGPU_INS_TBM]            = UOP_IADD;
  m_fp_uop_table[IGPU_INS_UNCOND_BR]      = UOP_FADD;
  m_fp_uop_table[IGPU_INS_VFMA]           = UOP_IADD;
  m_fp_uop_table[IGPU_INS_VTX]            = UOP_FADD;
  m_fp_uop_table[IGPU_INS_WIDENOP]        = UOP_NOP;
  m_fp_uop_table[IGPU_INS_X87_ALU]        = UOP_FADD;
  m_fp_uop_table[IGPU_INS_XOP]            = UOP_FADD; // new
  m_fp_uop_table[IGPU_INS_XSAVE]          = UOP_FMUL;
  m_fp_uop_table[IGPU_INS_XSAVEOPT]       = UOP_FMUL;
	m_fp_uop_table[IGPU_INS_TR_MUL]         = UOP_IMUL;            
  m_fp_uop_table[IGPU_INS_TR_DIV]         = UOP_IMUL;
  m_fp_uop_table[IGPU_INS_TR_FMUL]        = UOP_FMUL;
  m_fp_uop_table[IGPU_INS_TR_FDIV]        = UOP_FDIV;
  m_fp_uop_table[IGPU_INS_TR_NOP]         = UOP_NOP;
  m_fp_uop_table[IGPU_INS_PREFETCH_NTA]   = UOP_FADD;
  m_fp_uop_table[IGPU_INS_PREFETCH_T0]    = UOP_FADD;
  m_fp_uop_table[IGPU_INS_PREFETCH_T1]    = UOP_FADD;
  m_fp_uop_table[IGPU_INS_PREFETCH_T2]    = UOP_FADD;
  m_fp_uop_table[IGPU_INS_GPU_EN]         = UOP_FADD; // new
  m_fp_uop_table[IGPU_INS_CPU_MEM_EXT_OP] = UOP_FADD; // new
}

inst_info_s* igpu_decoder_c::get_inst_info(thread_s *thread_trace_info, int core_id, int sim_thread_id)
{
  trace_info_igpu_s trace_info;
  inst_info_s *info;
  // Copy current instruction to data structure
  memcpy(&trace_info, thread_trace_info->m_prev_trace_info, sizeof(trace_info_igpu_s));

  // Set next pc address
  trace_info_igpu_s *next_trace_info = static_cast<trace_info_igpu_s *>(thread_trace_info->m_next_trace_info);
  trace_info.m_instruction_next_addr = next_trace_info->m_instruction_addr;

  // Copy next instruction to current instruction field
  memcpy(thread_trace_info->m_prev_trace_info, thread_trace_info->m_next_trace_info, 
         sizeof(trace_info_igpu_s));

  DEBUG_CORE(core_id, "trace_read core_id:%d thread_id:%d pc:0x%llx opcode:%d inst_count:%llu\n", core_id, sim_thread_id, 
             (Addr)(trace_info.m_instruction_addr), static_cast<int>(trace_info.m_opcode), (Counter)(thread_trace_info->m_temp_inst_count));

  // So far we have raw instruction format, so we need to MacSim specific trace format
  info = convert_pinuop_to_t_uop(&trace_info, thread_trace_info->m_trace_uop_array, 
                                 core_id, sim_thread_id);

  return info;
}

inst_info_s* igpu_decoder_c::convert_pinuop_to_t_uop(void *trace_info, trace_uop_s **trace_uop, 
                                                    int core_id, int sim_thread_id)
{
  trace_info_igpu_s *pi = static_cast<trace_info_igpu_s *>(trace_info); 
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

  ASSERT(pi->m_opcode != IGPU_INS_INVALID);
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
        case IGPU_INS_PREFETCH:
          trace_uop[0]->m_mem_type = MEM_PF;
          break;
        default:
          trace_uop[0]->m_mem_type = MEM_LD;
          break;
      }
      
      // prefetch instruction
      if (pi->m_opcode == IGPU_INS_PREFETCH 
          || (pi->m_opcode >= IGPU_INS_PREFETCH_NTA && pi->m_opcode <= IGPU_INS_PREFETCH_T2)) {
        switch (pi->m_opcode) {
          case IGPU_INS_PREFETCH_NTA:
            trace_uop[0]->m_mem_type = MEM_SWPREF_NTA;
            break;

          case IGPU_INS_PREFETCH_T0:
            trace_uop[0]->m_mem_type = MEM_SWPREF_T0;
            break;

          case IGPU_INS_PREFETCH_T1:
            trace_uop[0]->m_mem_type = MEM_SWPREF_T1;
            break;

          case IGPU_INS_PREFETCH_T2:
            trace_uop[0]->m_mem_type = MEM_SWPREF_T2;
            break;
        }
      }

      trace_uop[0]->m_mem_size = 4;
      trace_uop[0]->m_cf_type = NOT_CF;
      trace_uop[0]->m_op_type = (pi->m_is_fp) ? UOP_FMEM : UOP_IMEM;
      trace_uop[0]->m_bar_type = NOT_BAR;
      trace_uop[0]->m_num_src_regs = pi->m_num_read_regs;
      
      if (pi->m_opcode == XED_CATEGORY_DATAXFER)
        trace_uop[0]->m_num_dest_regs = pi->m_num_dest_regs;
      else
        trace_uop[0]->m_num_dest_regs = 0;
      
      trace_uop[0]->m_pin_2nd_mem = 0;
      trace_uop[0]->m_eom = 0;
      trace_uop[0]->m_alu_uop = false;
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
        trace_uop[1]->m_mem_type = MEM_LD;
        trace_uop[1]->m_mem_size = 4;
        trace_uop[1]->m_cf_type = NOT_CF;
        trace_uop[1]->m_op_type = (pi->m_is_fp) ? UOP_FMEM : UOP_IMEM;
        trace_uop[1]->m_bar_type = NOT_BAR;
        trace_uop[1]->m_num_dest_regs = 0;
        trace_uop[1]->m_num_src_regs = pi->m_num_read_regs;
        trace_uop[1]->m_num_dest_regs = pi->m_num_dest_regs;
        trace_uop[1]->m_pin_2nd_mem   = 1;
        trace_uop[1]->m_eom = 0;
        trace_uop[1]->m_alu_uop = false;
        trace_uop[1]->m_inst_size = pi->m_size;
        trace_uop[1]->m_mul_mem_uops = 0; // uncoalesced memory accesses
        num_uop = 2;
      } // num_loads == 2

      // TODO
      // for igpu we do not generate multiple uops based on has_immediate
      // m_has_immediate is overloaded - in case of ptx simulations, for uncoalesced 
      // accesses, multiple memory access are generated and for each access there is
      // an instruction in the trace. the m_has_immediate flag is used to mark the
      // first and last accesses of an uncoalesced memory instruction
      trace_uop[0]->m_mul_mem_uops = 0; //pi->m_has_immediate;

      // we do not need temporary registers in IGPU for loads
      write_dest_reg = 1;

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
      cur_trace_uop->m_mul_mem_uops  = 0;
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
    if (num_uop == 0 && pi->m_opcode == IGPU_INS_MISC && pi->m_ld_vaddr2 == 1) {
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
        key_addr = ((pi->m_instruction_addr << 3) + ii);
        info = htable->hash_table_access_create(key_addr, &new_entry);
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
        (trace_uop[ii])->m_srcs[jj].m_id = pi->m_src[jj];
        (trace_uop[ii])->m_srcs[jj].m_reg = pi->m_src[jj];
      }

      // store or control flow has a dependency whoever the last one
      if ((trace_uop[ii]->m_mem_type == MEM_ST) || 
          (trace_uop[ii]->m_cf_type != NOT_CF)) {

        if (tmp_reg_needed && !inst_has_ALU_uop) {
          (trace_uop[ii])->m_srcs[jj].m_type = (Reg_Type)0;
          (trace_uop[ii])->m_srcs[jj].m_id = TR_REG_TMP0;
          (trace_uop[ii])->m_srcs[jj].m_reg = TR_REG_TMP0;
          trace_uop[ii]->m_num_src_regs += 1;
        } 
        else if (inst_has_ALU_uop) {
          for (kk = 0; kk < pi->m_num_dest_regs; ++kk) {
            (trace_uop[ii])->m_srcs[jj+kk].m_type = (Reg_Type)0;
            (trace_uop[ii])->m_srcs[jj+kk].m_id = pi->m_dst[kk];
            (trace_uop[ii])->m_srcs[jj+kk].m_reg = pi->m_dst[kk];
          }

          trace_uop[ii]->m_num_src_regs += pi->m_num_dest_regs;
        }
      }

      // alu uop only has a dependency with a temp register
      if (trace_uop[ii]->m_alu_uop) {
        if (tmp_reg_needed) {
          (trace_uop[ii])->m_srcs[jj].m_type = (Reg_Type)0;
          (trace_uop[ii])->m_srcs[jj].m_id = TR_REG_TMP0;
          (trace_uop[ii])->m_srcs[jj].m_reg = TR_REG_TMP0;
          trace_uop[ii]->m_num_src_regs += 1;
        }
      }

      for (jj = 0; jj < trace_uop[ii]->m_num_dest_regs; ++jj) {
        (trace_uop[ii])->m_dests[jj].m_type = (Reg_Type)0;
        (trace_uop[ii])->m_dests[jj].m_id = pi->m_dst[jj];
        (trace_uop[ii])->m_dests[jj].m_reg = pi->m_dst[jj];
      }

      // add tmp register as a destination register
      if (tmp_reg_needed && trace_uop[ii]->m_mem_type == MEM_LD) { 
        (trace_uop[ii])->m_dests[jj].m_type = (Reg_Type)0;
        (trace_uop[ii])->m_dests[jj].m_id = TR_REG_TMP0;
        (trace_uop[ii])->m_dests[jj].m_reg = TR_REG_TMP0;
        trace_uop[ii]->m_num_dest_regs += 1;
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

  ASSERT(dyn_uop_counter);

  // set eom flag and next pc address for the last uop of this instruction
  trace_uop[dyn_uop_counter-1]->m_eom = 1;
  trace_uop[dyn_uop_counter-1]->m_npc = pi->m_instruction_next_addr;

  ASSERT(num_uop > 0);
  first_info->m_trace_info.m_num_uop = num_uop;

  DEBUG("%s: read: %d write: %d\n", igpu_opcode_names[pi->m_opcode], pi->m_num_read_regs, pi->m_num_dest_regs);

  return first_info;
}

void igpu_decoder_c::convert_dyn_uop(inst_info_s *info, void *trace_info, trace_uop_s *trace_uop, 
                                    Addr rep_offset, int core_id)
{
  trace_info_igpu_s *pi = static_cast<trace_info_igpu_s *>(trace_info);
  core_c* core    = m_simBase->m_core_pointers[core_id];
  trace_uop->m_va = 0;

  if (info->m_table_info->m_cf_type) {
    trace_uop->m_actual_taken = pi->m_actually_taken;
    trace_uop->m_target       = pi->m_branch_target;
  } else if (info->m_table_info->m_mem_type) {
    if (info->m_table_info->m_mem_type == MEM_ST) {
      trace_uop->m_va       = pi->m_st_vaddr;
      trace_uop->m_mem_size = pi->m_mem_write_size;
      // In rare cases, if a page fault occurs due to a write you can miss the
      // memory callback because of which m_mem_write_size will be 0.
      // Hack it up using dummy values
      if (pi->m_mem_write_size == 0) {
          trace_uop->m_va       = 0xabadabad;
          trace_uop->m_mem_size = 4;
      }
    } 
    else if ((info->m_table_info->m_mem_type == MEM_LD) || 
             (info->m_table_info->m_mem_type == MEM_PF) ||
             (info->m_table_info->m_mem_type >= MEM_SWPREF_NTA &&
              info->m_table_info->m_mem_type <= MEM_SWPREF_T2)) {
      if (info->m_trace_info.m_second_mem)
        trace_uop->m_va = pi->m_ld_vaddr2;
      else 
        trace_uop->m_va = pi->m_ld_vaddr1;

      if (pi->m_mem_read_size)
        trace_uop->m_mem_size = pi->m_mem_read_size;
    }
  }

  // next pc
  trace_uop->m_npc = trace_uop->m_addr;
}

void igpu_decoder_c::dprint_inst(void *trace_info, int core_id, int thread_id)
{
  if (m_dprint_count++ >= 50000 || !*KNOB(KNOB_DEBUG_PRINT_TRACE))
    return ;

  trace_info_igpu_s *t_info = static_cast<trace_info_igpu_s *>(trace_info);
 
  *m_dprint_output << "*** begin of the data strcture *** " << endl;
  *m_dprint_output << "core_id:" << core_id << " thread_id:" << thread_id << endl;
  *m_dprint_output << "uop_opcode " <<igpu_opcode_names[(uint32_t) t_info->m_opcode]  << endl;
  *m_dprint_output << "num_read_regs: " << hex <<  (uint32_t) t_info->m_num_read_regs << endl;
  *m_dprint_output << "num_dest_regs: " << hex << (uint32_t) t_info->m_num_dest_regs << endl;
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
