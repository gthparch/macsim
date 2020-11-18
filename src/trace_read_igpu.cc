#include "trace_read_igpu.h"
#include "process_manager.h"
#include "frontend.h"
#include "memory.h"
#include "assert_macros.h"
#include "debug_macros.h"
#include "utils.h"
#include "all_knobs.h"
#include "statistics.h"
#include "statsEnums.h"

#define DEBUG(args...) _DEBUG(*KNOB(KNOB_DEBUG_TRACE_READ), ##args)
#define DEBUG_CORE(m_core_id, args...)                          \
  if (m_core_id == *m_simBase->m_knobs->KNOB_DEBUG_CORE_ID) {   \
    _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_TRACE_READ, ##args); \
  }
#define READ_1st_GEN_IGPU_TRACES // this should be moved to make file 
#ifdef READ_1st_GEN_IGPU_TRACES 

void igpu_decoder_c::init_pin_convert()
{
  m_int_uop_table[GED_OPCODE_ILLEGAL] = UOP_INV;
  m_int_uop_table[GED_OPCODE_MOV] = UOP_IMEM;
  m_int_uop_table[GED_OPCODE_SEL] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_MOVI] = UOP_IMEM;
  m_int_uop_table[GED_OPCODE_NOT] = UOP_LOGIC;
  m_int_uop_table[GED_OPCODE_AND] = UOP_LOGIC;
  m_int_uop_table[GED_OPCODE_OR] = UOP_LOGIC;
  m_int_uop_table[GED_OPCODE_XOR] = UOP_LOGIC;
  m_int_uop_table[GED_OPCODE_SHR] = UOP_SHIFT;
  m_int_uop_table[GED_OPCODE_SHL] = UOP_SHIFT;
  m_int_uop_table[GED_OPCODE_ASR] = UOP_SHIFT;
  m_int_uop_table[GED_OPCODE_CMP] = UOP_ICMP;
  m_int_uop_table[GED_OPCODE_CMPN] = UOP_ICMP;
  m_int_uop_table[GED_OPCODE_CSEL] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_F32TO16] = UOP_FCVT;
  m_int_uop_table[GED_OPCODE_F16TO32] = UOP_FCVT;
  m_int_uop_table[GED_OPCODE_BFREV] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_BFE] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_BFI1] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_BFI2] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_JMPI] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_BRD] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_IF] = UOP_CF;
  m_int_uop_table[GED_OPCODE_BRC] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_ELSE] = UOP_CF;
  m_int_uop_table[GED_OPCODE_ENDIF] = UOP_CF;
  m_int_uop_table[GED_OPCODE_WHILE] = UOP_CF;
  m_int_uop_table[GED_OPCODE_BREAK] = UOP_CF;
  m_int_uop_table[GED_OPCODE_CONT] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_HALT] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_CALL] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_RET] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_WAIT] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_SEND] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_SENDC] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_MATH] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_ADD] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_MUL] = UOP_IMUL;
  m_int_uop_table[GED_OPCODE_AVG] = UOP_IMUL;
  m_int_uop_table[GED_OPCODE_FRC] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_RNDU] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_RNDD] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_RNDE] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_RNDZ] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_MAC] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_MACH] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_LZD] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_FBH] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_FBL] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_CBIT] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_ADDC] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_SUBB] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_SAD2] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_SADA2] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_DP4] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_DPH] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_DP3] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_DP2] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_LINE] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_PLN] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_MAD] = UOP_IMUL;
  m_int_uop_table[GED_OPCODE_LRP] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_NOP] = UOP_NOP;
  m_int_uop_table[GED_OPCODE_DIM] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_CALLA] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_SMOV] = UOP_IMEM;
  m_int_uop_table[GED_OPCODE_GOTO] = UOP_CF;
  m_int_uop_table[GED_OPCODE_JOIN] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_MADM] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_SENDS] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_SENDSC] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_INVALID] = UOP_INV;

  m_fp_uop_table[GED_OPCODE_ILLEGAL] = UOP_INV;
  m_fp_uop_table[GED_OPCODE_MOV] = UOP_IMEM;
  m_fp_uop_table[GED_OPCODE_SEL] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_MOVI] = UOP_IMEM;
  m_fp_uop_table[GED_OPCODE_NOT] = UOP_LOGIC;
  m_fp_uop_table[GED_OPCODE_AND] = UOP_LOGIC;
  m_fp_uop_table[GED_OPCODE_OR] = UOP_LOGIC;
  m_fp_uop_table[GED_OPCODE_XOR] = UOP_LOGIC;
  m_fp_uop_table[GED_OPCODE_SHR] = UOP_SHIFT;
  m_fp_uop_table[GED_OPCODE_SHL] = UOP_SHIFT;
  m_fp_uop_table[GED_OPCODE_ASR] = UOP_SHIFT;
  m_fp_uop_table[GED_OPCODE_CMP] = UOP_ICMP;
  m_fp_uop_table[GED_OPCODE_CMPN] = UOP_ICMP;
  m_fp_uop_table[GED_OPCODE_CSEL] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_F32TO16] = UOP_FCVT;
  m_fp_uop_table[GED_OPCODE_F16TO32] = UOP_FCVT;
  m_fp_uop_table[GED_OPCODE_BFREV] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_BFE] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_BFI1] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_BFI2] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_JMPI] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_BRD] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_IF] = UOP_CF;
  m_fp_uop_table[GED_OPCODE_BRC] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_ELSE] = UOP_CF;
  m_fp_uop_table[GED_OPCODE_ENDIF] = UOP_CF;
  m_fp_uop_table[GED_OPCODE_WHILE] = UOP_CF;
  m_fp_uop_table[GED_OPCODE_BREAK] = UOP_CF;
  m_fp_uop_table[GED_OPCODE_CONT] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_HALT] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_CALL] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_RET] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_WAIT] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_SEND] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_SENDC] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_MATH] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_ADD] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_MUL] = UOP_IMUL;
  m_fp_uop_table[GED_OPCODE_AVG] = UOP_IMUL;
  m_fp_uop_table[GED_OPCODE_FRC] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_RNDU] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_RNDD] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_RNDE] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_RNDZ] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_MAC] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_MACH] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_LZD] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_FBH] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_FBL] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_CBIT] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_ADDC] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_SUBB] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_SAD2] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_SADA2] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_DP4] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_DPH] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_DP3] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_DP2] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_LINE] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_PLN] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_MAD] = UOP_IMUL;
  m_fp_uop_table[GED_OPCODE_LRP] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_NOP] = UOP_NOP;
  m_fp_uop_table[GED_OPCODE_DIM] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_CALLA] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_SMOV] = UOP_IMEM;
  m_fp_uop_table[GED_OPCODE_GOTO] = UOP_CF;
  m_fp_uop_table[GED_OPCODE_JOIN] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_MADM] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_SENDS] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_SENDSC] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_INVALID] = UOP_INV;
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

  ASSERT(pi->m_opcode != GED_OPCODE_INVALID);
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
      //switch (pi->m_opcode) {
        //case IGPU_INS_PREFETCH:
          //trace_uop[0]->m_mem_type = MEM_PF;
          //break;
        //default:
          //trace_uop[0]->m_mem_type = MEM_LD;
          //break;
      //}
      
      // prefetch instruction
      //if (pi->m_opcode == IGPU_INS_PREFETCH 
          //|| (pi->m_opcode >= IGPU_INS_PREFETCH_NTA && pi->m_opcode <= IGPU_INS_PREFETCH_T2)) {
        //switch (pi->m_opcode) {
          //case IGPU_INS_PREFETCH_NTA:
            //trace_uop[0]->m_mem_type = MEM_SWPREF_NTA;
            //break;

          //case IGPU_INS_PREFETCH_T0:
            //trace_uop[0]->m_mem_type = MEM_SWPREF_T0;
            //break;

          //case IGPU_INS_PREFETCH_T1:
            //trace_uop[0]->m_mem_type = MEM_SWPREF_T1;
            //break;

          //case IGPU_INS_PREFETCH_T2:
            //trace_uop[0]->m_mem_type = MEM_SWPREF_T2;
            //break;
        //}
      //}

      trace_uop[0]->m_mem_size = 4;
      trace_uop[0]->m_cf_type = NOT_CF;
      trace_uop[0]->m_op_type = (pi->m_is_fp) ? UOP_FMEM : UOP_IMEM;
      trace_uop[0]->m_bar_type = NOT_BAR;
      trace_uop[0]->m_num_src_regs = pi->m_num_read_regs;
      
      //if (pi->m_opcode == XED_CATEGORY_DATAXFER)
        //trace_uop[0]->m_num_dest_regs = pi->m_num_dest_regs;
      //else
        //trace_uop[0]->m_num_dest_regs = 0;
      
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
    //if (num_uop == 0 && pi->m_opcode == IGPU_INS_MISC && pi->m_ld_vaddr2 == 1) {
      //trace_uop[0]->m_opcode        = pi->m_opcode;
      //trace_uop[0]->m_mem_type      = NOT_MEM;
      //trace_uop[0]->m_cf_type       = NOT_CF;
      //trace_uop[0]->m_op_type       = UOP_FULL_FENCE;
      //trace_uop[0]->m_bar_type      = NOT_BAR;
      //trace_uop[0]->m_num_dest_regs = 0;
      //trace_uop[0]->m_num_src_regs  = 0;
      //trace_uop[0]->m_pin_2nd_mem   = 0;
      //trace_uop[0]->m_eom           = 1;
      //trace_uop[0]->m_inst_size     = pi->m_size;
      //++num_uop;
    //}

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


#else 

void igpu_decoder_c::init_pin_convert() {
  m_int_uop_table[GED_OPCODE_ILLEGAL] = UOP_INV;
  m_int_uop_table[GED_OPCODE_MOV] = UOP_IMEM;
  m_int_uop_table[GED_OPCODE_SEL] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_MOVI] = UOP_IMEM;
  m_int_uop_table[GED_OPCODE_NOT] = UOP_LOGIC;
  m_int_uop_table[GED_OPCODE_AND] = UOP_LOGIC;
  m_int_uop_table[GED_OPCODE_OR] = UOP_LOGIC;
  m_int_uop_table[GED_OPCODE_XOR] = UOP_LOGIC;
  m_int_uop_table[GED_OPCODE_SHR] = UOP_SHIFT;
  m_int_uop_table[GED_OPCODE_SHL] = UOP_SHIFT;
  m_int_uop_table[GED_OPCODE_SMOV] = UOP_IMEM;
  m_int_uop_table[GED_OPCODE_ASR] = UOP_SHIFT;
  m_int_uop_table[GED_OPCODE_CMP] = UOP_ICMP;
  m_int_uop_table[GED_OPCODE_CMPN] = UOP_ICMP;
  m_int_uop_table[GED_OPCODE_CSEL] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_BFREV] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_BFE] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_BFI1] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_BFI2] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_JMPI] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_BRD] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_IF] = UOP_CF;
  m_int_uop_table[GED_OPCODE_BRC] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_ELSE] = UOP_CF;
  m_int_uop_table[GED_OPCODE_ENDIF] = UOP_CF;
  m_int_uop_table[GED_OPCODE_WHILE] = UOP_CF;
  m_int_uop_table[GED_OPCODE_BREAK] = UOP_CF;
  m_int_uop_table[GED_OPCODE_CONT] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_HALT] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_CALLA] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_CALL] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_RET] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_GOTO] = UOP_CF;
  m_int_uop_table[GED_OPCODE_JOIN] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_WAIT] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_SEND] = UOP_IMEM;
  m_int_uop_table[GED_OPCODE_SENDC] = UOP_IMEM;
  m_int_uop_table[GED_OPCODE_SENDS] = UOP_IMEM;
  m_int_uop_table[GED_OPCODE_SENDSC] = UOP_IMEM;
  m_int_uop_table[GED_OPCODE_MATH] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_ADD] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_MUL] = UOP_IMUL;
  m_int_uop_table[GED_OPCODE_AVG] = UOP_IMUL;
  m_int_uop_table[GED_OPCODE_FRC] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_RNDU] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_RNDD] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_RNDE] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_RNDZ] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_MAC] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_MACH] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_LZD] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_FBH] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_FBL] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_CBIT] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_ADDC] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_SUBB] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_SAD2] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_SADA2] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_DP4] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_DPH] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_DP3] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_DP2] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_LINE] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_PLN] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_MAD] = UOP_SIMD;
  m_int_uop_table[GED_OPCODE_LRP] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_MADM] = UOP_SIMD;
  m_int_uop_table[GED_OPCODE_NOP] = UOP_NOP;
  m_int_uop_table[GED_OPCODE_ROR] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_ROL] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_DP4A] = UOP_IMUL;
  m_int_uop_table[GED_OPCODE_F32TO16] = UOP_FCVT;
  m_int_uop_table[GED_OPCODE_F16TO32] = UOP_FCVT;
  m_int_uop_table[GED_OPCODE_DIM] = UOP_IADD;
  m_int_uop_table[GED_OPCODE_INVALID] = UOP_INV;

  m_fp_uop_table[GED_OPCODE_ILLEGAL] = UOP_INV;
  m_fp_uop_table[GED_OPCODE_MOV] = UOP_IMEM;
  m_fp_uop_table[GED_OPCODE_SEL] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_MOVI] = UOP_IMEM;
  m_fp_uop_table[GED_OPCODE_NOT] = UOP_LOGIC;
  m_fp_uop_table[GED_OPCODE_AND] = UOP_LOGIC;
  m_fp_uop_table[GED_OPCODE_OR] = UOP_LOGIC;
  m_fp_uop_table[GED_OPCODE_XOR] = UOP_LOGIC;
  m_fp_uop_table[GED_OPCODE_SHR] = UOP_SHIFT;
  m_fp_uop_table[GED_OPCODE_SHL] = UOP_SHIFT;
  m_fp_uop_table[GED_OPCODE_SMOV] = UOP_IMEM;
  m_fp_uop_table[GED_OPCODE_ASR] = UOP_SHIFT;
  m_fp_uop_table[GED_OPCODE_CMP] = UOP_ICMP;
  m_fp_uop_table[GED_OPCODE_CMPN] = UOP_ICMP;
  m_fp_uop_table[GED_OPCODE_CSEL] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_BFREV] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_BFE] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_BFI1] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_BFI2] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_JMPI] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_BRD] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_IF] = UOP_CF;
  m_fp_uop_table[GED_OPCODE_BRC] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_ELSE] = UOP_CF;
  m_fp_uop_table[GED_OPCODE_ENDIF] = UOP_CF;
  m_fp_uop_table[GED_OPCODE_WHILE] = UOP_CF;
  m_fp_uop_table[GED_OPCODE_BREAK] = UOP_CF;
  m_fp_uop_table[GED_OPCODE_CONT] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_HALT] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_CALLA] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_CALL] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_RET] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_GOTO] = UOP_CF;
  m_fp_uop_table[GED_OPCODE_JOIN] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_WAIT] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_SEND] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_SENDC] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_SENDS] = UOP_IMEM;
  m_fp_uop_table[GED_OPCODE_SENDSC] = UOP_IMEM;
  m_fp_uop_table[GED_OPCODE_MATH] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_ADD] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_MUL] = UOP_IMUL;
  m_fp_uop_table[GED_OPCODE_AVG] = UOP_IMUL;
  m_fp_uop_table[GED_OPCODE_FRC] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_RNDU] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_RNDD] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_RNDE] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_RNDZ] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_MAC] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_MACH] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_LZD] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_FBH] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_FBL] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_CBIT] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_ADDC] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_SUBB] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_SAD2] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_SADA2] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_DP4] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_DPH] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_DP3] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_DP2] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_LINE] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_PLN] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_MAD] = UOP_SIMD;
  m_fp_uop_table[GED_OPCODE_LRP] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_MADM] = UOP_SIMD;
  m_fp_uop_table[GED_OPCODE_NOP] = UOP_NOP;
  m_fp_uop_table[GED_OPCODE_ROR] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_ROL] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_DP4A] = UOP_IMUL;
  m_fp_uop_table[GED_OPCODE_F32TO16] = UOP_FCVT;
  m_fp_uop_table[GED_OPCODE_F16TO32] = UOP_FCVT;
  m_fp_uop_table[GED_OPCODE_DIM] = UOP_IADD;
  m_fp_uop_table[GED_OPCODE_INVALID] = UOP_INV;
}

bool igpu_decoder_c::get_uops_from_traces(int core_id, uop_c *uop,
                                          int sim_thread_id) {
  ASSERT(uop);

  trace_uop_s *trace_uop;
  int num_uop = 0;
  core_c *core = m_simBase->m_core_pointers[core_id];
  inst_info_s *info;

  // fetch ended : no uop to fetch
  if (core->m_fetch_ended[sim_thread_id]) return false;

  bool read_success = true;
  thread_s *thread_trace_info = core->get_trace_info(sim_thread_id);

  if (thread_trace_info->m_thread_init) {
    thread_trace_info->m_thread_init = false;
  }

  ///
  /// BOM (beginning of macro) : need to get a next instruction
  ///
  if (thread_trace_info->m_bom) {
    bool inst_read;  // indicate new instruction has been read from a trace file

    if (core->m_inst_fetched[sim_thread_id] < *KNOB(KNOB_MAX_INSTS)) {
      // read next instruction
      read_success = read_trace(core_id, thread_trace_info->m_next_trace_info,
                                sim_thread_id, &inst_read);
    } else {
      inst_read = false;
      if (!core->get_trace_info(sim_thread_id)->m_trace_ended) {
        core->get_trace_info(sim_thread_id)->m_trace_ended = true;
      }
    }

    ///
    /// Trace read failed
    ///
    if (!read_success) return false;

    info = get_inst_info(thread_trace_info, core_id, sim_thread_id);

    STAT_CORE_EVENT(core_id, PARENT_UOP);
    // STAT_CORE_EVENT(core_id, TRACE_READ_COUNT);

    // read a new instruction, so update stats
    if (inst_read) {
      ++core->m_inst_fetched[sim_thread_id];
      DEBUG_CORE(core_id, "core_id:%d thread_id:%d inst_num:%llu\n", core_id,
                 sim_thread_id,
                 (Counter)(thread_trace_info->m_temp_inst_count + 1));

      if (core->m_inst_fetched[sim_thread_id] > core->m_max_inst_fetched)
        core->m_max_inst_fetched = core->m_inst_fetched[sim_thread_id];
    }

    trace_uop = thread_trace_info->m_trace_uop_array[0];
    num_uop = info->m_trace_info.m_num_uop;
    ASSERT(info->m_trace_info.m_num_uop > 0);

    thread_trace_info->m_num_sending_uop = 1;
    thread_trace_info->m_eom = thread_trace_info->m_trace_uop_array[0]->m_eom;
    thread_trace_info->m_bom = false;

    uop->m_isitBOM = true;
    POWER_CORE_EVENT(core_id, POWER_INST_DECODER_R);
    POWER_CORE_EVENT(core_id, POWER_OPERAND_DECODER_R);
  }  // END EOM
  // read remaining uops from the same instruction
  else {
    trace_uop = thread_trace_info
                  ->m_trace_uop_array[thread_trace_info->m_num_sending_uop];
    info = trace_uop->m_info;
    thread_trace_info->m_eom = trace_uop->m_eom;
    info->m_trace_info.m_bom = 0;  // because of repeat instructions ....
    uop->m_isitBOM = false;
    ++thread_trace_info->m_num_sending_uop;
  }

  // uop number is specific to the core
  uop->m_unique_num = core->inc_and_get_unique_uop_num();

  // set end of macro flag
  if (thread_trace_info->m_eom) {
    uop->m_isitEOM = true;  // mark for current uop
    thread_trace_info->m_bom = true;  // mark for next instruction
  } else {
    uop->m_isitEOM = false;
    thread_trace_info->m_bom = false;
  }

  if (core->get_trace_info(sim_thread_id)->m_trace_ended && uop->m_isitEOM) {
    --core->m_fetching_thread_num;
    core->m_fetch_ended[sim_thread_id] = true;
    uop->m_last_uop = true;
    DEBUG_CORE(core_id,
               "core_id:%d thread_id:%d inst_num:%lld uop_num:%lld "
               "fetched:%lld last uop\n",
               core_id, sim_thread_id, uop->m_inst_num, uop->m_uop_num,
               core->m_inst_fetched[sim_thread_id]);
  }

  ///
  /// Set up actual uop data structure
  ///
  uop->m_opcode = trace_uop->m_opcode;
  uop->m_uop_type = info->m_table_info->m_op_type;
  uop->m_cf_type = info->m_table_info->m_cf_type;
  uop->m_mem_type = info->m_table_info->m_mem_type;
  ASSERT(uop->m_mem_type >= 0 && uop->m_mem_type < NUM_MEM_TYPES);
  uop->m_bar_type = trace_uop->m_bar_type;
  uop->m_npc = trace_uop->m_npc;
  uop->m_active_mask = trace_uop->m_active_mask;

  if (uop->m_cf_type) {
    uop->m_taken_mask = trace_uop->m_taken_mask;
    uop->m_reconverge_addr = trace_uop->m_reconverge_addr;
    uop->m_target_addr = trace_uop->m_target;
  }

  if (uop->m_opcode == GPU_EN) {
    m_simBase->m_gpu_paused = false;
  }

  // address translation
  if (trace_uop->m_va == 0) {
    uop->m_vaddr = 0;
  } else {
#ifndef USING_SST
    // since we can have 64-bit address space and each trace has 32-bit address,
    // using extra bits to differentiate address space of each application
    uop->m_vaddr =
      trace_uop->m_va +
      m_simBase->m_memory->base_addr(
        core_id,
        (unsigned long)UINT_MAX *
          (core->get_trace_info(sim_thread_id)->m_process->m_process_id) *
          10ul);
#else
    Addr addr = (unsigned long)UINT_MAX *
                (core->get_trace_info(sim_thread_id)->m_process->m_process_id) *
                10ul;
    Addr line_size = 64;
    uop->m_vaddr = trace_uop->m_va + (addr & -line_size);
#endif
  }

  uop->m_mem_size = trace_uop->m_mem_size;
  uop->m_dir = trace_uop->m_actual_taken;
  uop->m_pc = info->m_addr;
  uop->m_core_id = core_id;

  if (uop->m_mem_type != NOT_MEM) {
    int temp_num_req =
      (uop->m_mem_size + *KNOB(KNOB_MAX_TRANSACTION_SIZE) - 1) /
      *KNOB(KNOB_MAX_TRANSACTION_SIZE);

    ASSERTM(
      temp_num_req > 0,
      "pc:%llx vaddr:%llx opcode:%d size:%d max:%d num:%d type:%d num:%d\n",
      uop->m_pc, uop->m_vaddr, uop->m_opcode, uop->m_mem_size,
      (int)*KNOB(KNOB_MAX_TRANSACTION_SIZE), temp_num_req, uop->m_mem_type,
      trace_uop->m_info->m_trace_info.m_num_uop);
  }

  // we found first uop of an instruction, so add instruction count
  if (uop->m_isitBOM) ++thread_trace_info->m_temp_inst_count;

  uop->m_inst_num = thread_trace_info->m_temp_inst_count;
  uop->m_num_srcs = trace_uop->m_num_src_regs;
  uop->m_num_dests = trace_uop->m_num_dest_regs;

  ASSERTM(uop->m_num_dests < MAX_DST_NUM, "uop->num_dests=%d MAX_DST_NUM=%d\n",
          uop->m_num_dests, MAX_DST_NUM);

  DEBUG_CORE(uop->m_core_id,
             "thread_id:%d uop_num:%llu num_srcs:%d  "
             "trace_uop->num_src_regs:%d  num_dsts:%d num_sending_uop:%d "
             "pc:0x%llx dir:%d BOM:%d \n",
             sim_thread_id, uop->m_uop_num, uop->m_num_srcs,
             trace_uop->m_num_src_regs, uop->m_num_dests,
             thread_trace_info->m_num_sending_uop, uop->m_pc, uop->m_dir,
             uop->m_isitBOM);

  // filling the src_info, dest_info
  if (uop->m_num_srcs < MAX_SRCS) {
    for (int index = 0; index < uop->m_num_srcs; ++index) {
      uop->m_src_info[index] = trace_uop->m_srcs[index].m_id;
      // DEBUG("uop_num:%lld src_info[%d]:%d\n", uop->uop_num, index, uop->src_info[index]);
    }
  } else {
    ASSERTM(uop->m_num_srcs < MAX_SRCS, "src_num:%d MAX_SRC:%d",
            uop->m_num_srcs, MAX_SRCS);
  }

  for (int index = 0; index < uop->m_num_dests; ++index) {
    uop->m_dest_info[index] = trace_uop->m_dests[index].m_id;
    ASSERT(trace_uop->m_dests[index].m_reg < NUM_REG_IDS);
  }

  uop->m_uop_num = (thread_trace_info->m_temp_uop_count++);
  uop->m_thread_id = sim_thread_id;
  uop->m_block_id = ((core)->get_trace_info(sim_thread_id))->m_block_id;
  uop->m_orig_block_id =
    ((core)->get_trace_info(sim_thread_id))->m_orig_block_id;
  uop->m_unique_thread_id =
    ((core)->get_trace_info(sim_thread_id))->m_unique_thread_id;
  uop->m_orig_thread_id =
    ((core)->get_trace_info(sim_thread_id))->m_orig_thread_id;

  DEBUG_CORE(
    uop->m_core_id,
    "new uop: uop_num:%lld inst_num:%lld thread_id:%d unique_num:%lld \n",
    uop->m_uop_num, uop->m_inst_num, uop->m_thread_id, uop->m_unique_num);

  // for a parent memory uop, read child uops from the trace
  if (uop->m_mem_type != NOT_MEM) {
    if (trace_uop->m_is_parent && trace_uop->m_num_children > 0) {
      DEBUG_CORE(uop->m_core_id,
                 "new parent_uop: uop_num:%lld inst_num:%lld thread_id:%d "
                 "unique_num:%lld \n",
                 uop->m_uop_num, uop->m_inst_num, uop->m_thread_id,
                 uop->m_unique_num);
      uop->m_child_uops = new uop_c *[trace_uop->m_num_children];
      uop->m_num_child_uops = trace_uop->m_num_children;
      uop->m_num_child_uops_done = 0;
      if (uop->m_num_child_uops != 64)
        uop->m_pending_child_uops = N_BIT_MASK(uop->m_num_child_uops);
      else
        uop->m_pending_child_uops = N_BIT_MASK_64;

      uop->m_vaddr = 0;
      uop->m_mem_size = 0;

      // read child uops from the trace
      uop_c *child_mem_uop = NULL;
      for (int i = 0; i < trace_uop->m_num_children; ++i) {
        bool dummy;
        read_success = read_trace(core_id, thread_trace_info->m_next_trace_info,
                                  sim_thread_id, &dummy);
        if (!read_success) return false;
        STAT_CORE_EVENT(core_id, CHILD_UOP_READ);

        trace_info_igpu_s *ti = static_cast<trace_info_igpu_s *>(
          thread_trace_info->m_prev_trace_info);

        child_mem_uop =
          core->get_frontend()->get_uop_pool()->acquire_entry(m_simBase);
        child_mem_uop->allocate();
        ASSERT(child_mem_uop);

        memcpy(child_mem_uop, uop, sizeof(uop_c));
        child_mem_uop->m_parent_uop = uop;
        if (trace_uop->m_mem_type == MEM_LD) {
          child_mem_uop->m_vaddr = ti->m_ld_vaddr1;
          child_mem_uop->m_mem_size = ti->m_mem_read_size;
        } else {
          child_mem_uop->m_vaddr = ti->m_st_vaddr;
          child_mem_uop->m_mem_size = ti->m_mem_write_size;
        }
        child_mem_uop->m_uop_num = (thread_trace_info->m_temp_uop_count++);
        child_mem_uop->m_unique_num = core->inc_and_get_unique_uop_num();

        uop->m_child_uops[i] = child_mem_uop;

        DEBUG_CORE(uop->m_core_id,
                   "new %dth-child_uop: uop_num:%lld inst_num:%lld "
                   "thread_id:%d unique_num:%lld \n",
                   i, child_mem_uop->m_uop_num, child_mem_uop->m_inst_num,
                   child_mem_uop->m_thread_id, child_mem_uop->m_unique_num);
        // Copy next instruction to current instruction field
        memcpy(thread_trace_info->m_prev_trace_info,
               thread_trace_info->m_next_trace_info, sizeof(trace_info_igpu_s));
      }
    }
  }

  return read_success;
}

inst_info_s *igpu_decoder_c::get_inst_info(thread_s *thread_trace_info,
                                           int core_id, int sim_thread_id) {
  trace_info_igpu_s trace_info;
  inst_info_s *info;
  // Copy current instruction to data structure
  memcpy(&trace_info, thread_trace_info->m_prev_trace_info,
         sizeof(trace_info_igpu_s));

  // Set next pc address
  trace_info_igpu_s *next_trace_info =
    static_cast<trace_info_igpu_s *>(thread_trace_info->m_next_trace_info);
  trace_info.m_instruction_next_addr = next_trace_info->m_instruction_addr;

  // Copy next instruction to current instruction field
  memcpy(thread_trace_info->m_prev_trace_info,
         thread_trace_info->m_next_trace_info, sizeof(trace_info_igpu_s));

  DEBUG_CORE(
    core_id,
    "trace_read core_id:%d thread_id:%d pc:0x%llx opcode:%d inst_count:%llu\n",
    core_id, sim_thread_id, (Addr)(trace_info.m_instruction_addr),
    static_cast<int>(trace_info.m_opcode),
    (Counter)(thread_trace_info->m_temp_inst_count));

  // So far we have raw instruction format, so we need to MacSim specific trace format
  info = convert_pinuop_to_t_uop(
    &trace_info, thread_trace_info->m_trace_uop_array, core_id, sim_thread_id);

  return info;
}

inst_info_s *igpu_decoder_c::convert_pinuop_to_t_uop(void *trace_info,
                                                     trace_uop_s **trace_uop,
                                                     int core_id,
                                                     int sim_thread_id) {
  trace_info_igpu_s *pi = static_cast<trace_info_igpu_s *>(trace_info);
  core_c *core = m_simBase->m_core_pointers[core_id];

  // simulator maintains a cache of decoded instructions (uop) for each process,
  // this avoids decoding of instructions everytime an instruction is executed
  int process_id = core->get_trace_info(sim_thread_id)->m_process->m_process_id;
  hash_c<inst_info_s> *htable = m_simBase->m_inst_info_hash[process_id];

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
  int num_uop = 0;
  int dyn_uop_counter = 0;
  bool tmp_reg_needed = false;
  bool inst_has_ALU_uop = false;
  bool inst_has_ld_uop = false;
  int ii, jj, kk;

  ASSERT(pi->m_opcode != GED_OPCODE_INVALID);
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
      // switch (pi->m_opcode) {
      // case IGPU_INS_PREFETCH:
      // trace_uop[0]->m_mem_type = MEM_PF;
      // break;
      // default:
      // trace_uop[0]->m_mem_type = MEM_LD;
      // break;
      //}

      trace_uop[0]->m_mem_type = MEM_LD;

      // prefetch instruction
      // if (pi->m_opcode == IGPU_INS_PREFETCH
      //|| (pi->m_opcode >= IGPU_INS_PREFETCH_NTA && pi->m_opcode <= IGPU_INS_PREFETCH_T2)) {
      // switch (pi->m_opcode) {
      // case IGPU_INS_PREFETCH_NTA:
      // trace_uop[0]->m_mem_type = MEM_SWPREF_NTA;
      // break;

      // case IGPU_INS_PREFETCH_T0:
      // trace_uop[0]->m_mem_type = MEM_SWPREF_T0;
      // break;

      // case IGPU_INS_PREFETCH_T1:
      // trace_uop[0]->m_mem_type = MEM_SWPREF_T1;
      // break;

      // case IGPU_INS_PREFETCH_T2:
      // trace_uop[0]->m_mem_type = MEM_SWPREF_T2;
      // break;
      //}
      //}

      trace_uop[0]->m_mem_size = 4;
      trace_uop[0]->m_cf_type = NOT_CF;
      trace_uop[0]->m_op_type = (pi->m_is_fp) ? UOP_FMEM : UOP_IMEM;
      trace_uop[0]->m_bar_type = NOT_BAR;
      trace_uop[0]->m_num_src_regs = pi->m_num_read_regs;

      /*
      // m_is_fp = true is used to indicate a child uop
      // m_branch_target is used to indicate the number of child uops a parent uop has
      if (pi->m_is_fp == false) {
        trace_uop[0]->m_is_parent = true;
        trace_uop[0]->m_num_children = pi->m_branch_target;
        }*/

      // if (pi->m_opcode == XED_CATEGORY_DATAXFER)
      // trace_uop[0]->m_num_dest_regs = pi->m_num_dest_regs;
      // else
      // trace_uop[0]->m_num_dest_regs = 0;

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
        trace_uop[1]->m_pin_2nd_mem = 1;
        trace_uop[1]->m_eom = 0;
        trace_uop[1]->m_alu_uop = false;
        trace_uop[1]->m_inst_size = pi->m_size;
        trace_uop[1]->m_mul_mem_uops = 0;  // uncoalesced memory accesses
        num_uop = 2;
      }  // num_loads == 2

      // TODO
      // for igpu we do not generate multiple uops based on has_immediate
      // m_has_immediate is overloaded - in case of ptx simulations, for uncoalesced
      // accesses, multiple memory access are generated and for each access there is
      // an instruction in the trace. the m_has_immediate flag is used to mark the
      // first and last accesses of an uncoalesced memory instruction
      trace_uop[0]->m_mul_mem_uops = 0;  // pi->m_has_immediate;

      // we do not need temporary registers in IGPU for loads
      write_dest_reg = 1;

      if (trace_uop[0]->m_mem_type == MEM_LD) {
        inst_has_ld_uop = true;
      }
    }  // HAS_LOAD

    // Add one more uop when temporary register is required
    if (pi->m_num_dest_regs && !write_dest_reg) {
      trace_uop_s *cur_trace_uop = trace_uop[num_uop++];
      if (inst_has_ld_uop) {
        tmp_reg_needed = true;
      }

      cur_trace_uop->m_opcode = pi->m_opcode;
      cur_trace_uop->m_mem_type = NOT_MEM;
      cur_trace_uop->m_cf_type = NOT_CF;
      cur_trace_uop->m_op_type =
        (Uop_Type)((pi->m_is_fp) ? m_fp_uop_table[pi->m_opcode]
                                 : m_int_uop_table[pi->m_opcode]);
      cur_trace_uop->m_bar_type = NOT_BAR;
      cur_trace_uop->m_num_src_regs = pi->m_num_read_regs;
      cur_trace_uop->m_num_dest_regs = pi->m_num_dest_regs;
      cur_trace_uop->m_pin_2nd_mem = 0;
      cur_trace_uop->m_eom = 0;
      cur_trace_uop->m_alu_uop = true;

      inst_has_ALU_uop = true;
    }

    ///
    /// 2. Instruction has a memory store operation
    ///
    if (pi->m_has_st) {
      trace_uop_s *cur_trace_uop = trace_uop[num_uop++];
      if (inst_has_ld_uop) tmp_reg_needed = true;

      cur_trace_uop->m_mem_type = MEM_ST;
      cur_trace_uop->m_opcode = pi->m_opcode;
      cur_trace_uop->m_cf_type = NOT_CF;
      cur_trace_uop->m_op_type = (pi->m_is_fp) ? UOP_FMEM : UOP_IMEM;
      cur_trace_uop->m_bar_type = NOT_BAR;
      cur_trace_uop->m_num_src_regs = pi->m_num_read_regs;
      cur_trace_uop->m_num_dest_regs = 0;
      cur_trace_uop->m_pin_2nd_mem = 0;
      cur_trace_uop->m_eom = 0;
      cur_trace_uop->m_alu_uop = false;
      cur_trace_uop->m_inst_size = pi->m_size;
      cur_trace_uop->m_mul_mem_uops = 0;
    }

    ///
    /// 3. Instruction has a branch operation
    ///
    if (pi->m_cf_type) {
      trace_uop_s *cur_trace_uop = trace_uop[num_uop++];

      if (inst_has_ld_uop) tmp_reg_needed = true;

      cur_trace_uop->m_mem_type = NOT_MEM;
      cur_trace_uop->m_cf_type =
        (Cf_Type)((pi->m_cf_type >= PIN_CF_SYS) ? CF_ICO : pi->m_cf_type);
      cur_trace_uop->m_op_type = UOP_CF;
      cur_trace_uop->m_bar_type = NOT_BAR;
      cur_trace_uop->m_num_src_regs = pi->m_num_read_regs;
      cur_trace_uop->m_num_dest_regs = 0;
      cur_trace_uop->m_pin_2nd_mem = 0;
      cur_trace_uop->m_eom = 0;
      cur_trace_uop->m_alu_uop = false;
      cur_trace_uop->m_inst_size = pi->m_size;
    }

    // Fence instruction: m_opcode == MISC && actually_taken == 1
    // if (num_uop == 0 && pi->m_opcode == IGPU_INS_MISC && pi->m_ld_vaddr2 == 1) {
    // trace_uop[0]->m_opcode        = pi->m_opcode;
    // trace_uop[0]->m_mem_type      = NOT_MEM;
    // trace_uop[0]->m_cf_type       = NOT_CF;
    // trace_uop[0]->m_op_type       = UOP_FULL_FENCE;
    // trace_uop[0]->m_bar_type      = NOT_BAR;
    // trace_uop[0]->m_num_dest_regs = 0;
    // trace_uop[0]->m_num_src_regs  = 0;
    // trace_uop[0]->m_pin_2nd_mem   = 0;
    // trace_uop[0]->m_eom           = 1;
    // trace_uop[0]->m_inst_size     = pi->m_size;
    //++num_uop;
    //}

    ///
    /// Non-memory, non-branch instruction
    ///
    if (num_uop == 0) {
      trace_uop[0]->m_opcode = pi->m_opcode;
      trace_uop[0]->m_mem_type = NOT_MEM;
      trace_uop[0]->m_cf_type = NOT_CF;
      trace_uop[0]->m_op_type = UOP_NOP;
      trace_uop[0]->m_bar_type = NOT_BAR;
      trace_uop[0]->m_num_dest_regs = 0;
      trace_uop[0]->m_num_src_regs = 0;
      trace_uop[0]->m_pin_2nd_mem = 0;
      trace_uop[0]->m_eom = 1;
      trace_uop[0]->m_inst_size = pi->m_size;
      ++num_uop;
    }

    info->m_trace_info.m_bom = true;
    info->m_trace_info.m_eom = false;
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
      ASSERTM(new_entry, "Add new uops to hash_table for core id::%d\n",
              core_id);

      trace_uop[ii]->m_addr = pi->m_instruction_addr;

      DEBUG_CORE(core_id,
                 "pi->instruction_addr:0x%llx trace_uop[%d]->addr:0x%llx "
                 "num_src_regs:%d num_read_regs:%d "
                 "pi:num_dst_regs:%d uop:num_dst_regs:%d \n",
                 (Addr)(pi->m_instruction_addr), ii, trace_uop[ii]->m_addr,
                 trace_uop[ii]->m_num_src_regs, pi->m_num_read_regs,
                 pi->m_num_dest_regs, trace_uop[ii]->m_num_dest_regs);

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
        } else if (inst_has_ALU_uop) {
          for (kk = 0; kk < pi->m_num_dest_regs; ++kk) {
            (trace_uop[ii])->m_srcs[jj + kk].m_type = (Reg_Type)0;
            (trace_uop[ii])->m_srcs[jj + kk].m_id = pi->m_dst[kk];
            (trace_uop[ii])->m_srcs[jj + kk].m_reg = pi->m_dst[kk];
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

      DEBUG_CORE(core_id, "tuop: pc 0x%llx num_src_reg:%d num_dest_reg:%d \n",
                 trace_uop[ii]->m_addr, trace_uop[ii]->m_num_src_regs,
                 trace_uop[ii]->m_num_dest_regs);

      trace_uop[ii]->m_info = info;

      // Add dynamic information to the uop
      convert_dyn_uop(info, pi, trace_uop[ii], 0, core_id);
    }

    // set end of macro flag to the last uop
    trace_uop[num_uop - 1]->m_eom = 1;

    ASSERT(num_uop > 0);
  }  // NEW_ENTRY
  ///
  /// Hash table already has matching instruction, we can skip above decoding process
  ///
  else {
    ASSERT(info);

    num_uop = info->m_trace_info.m_num_uop;
    for (ii = 0; ii < num_uop; ++ii) {
      if (ii > 0) {
        key_addr = ((pi->m_instruction_addr << 3) + ii);
        info = htable->hash_table_access_create(key_addr, &new_entry);
      }
      ASSERTM(!new_entry, "Core id %d index %d\n", core_id, ii);

      // convert raw instruction trace to MacSim trace format
      convert_info_uop(info, trace_uop[ii]);

      // add dynamic information
      convert_dyn_uop(info, pi, trace_uop[ii], 0, core_id);

      trace_uop[ii]->m_info = info;
      trace_uop[ii]->m_eom = 0;
      trace_uop[ii]->m_addr = pi->m_instruction_addr;
      trace_uop[ii]->m_opcode = pi->m_opcode;
    }

    // set end of macro flag to the last uop
    trace_uop[num_uop - 1]->m_eom = 1;

    ASSERT(num_uop > 0);
  }

  if ((pi->m_is_fp == false) && ((pi->m_num_ld) > 0 || (pi->m_has_st == 1)) &&
      (pi->m_branch_target > 0)) {
    /* only the first uop can be a parent uop */
    trace_uop[0]->m_is_parent = true;
    trace_uop[0]->m_num_children = pi->m_branch_target;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////
  // end of instruction decoding
  /////////////////////////////////////////////////////////////////////////////////////////////

  dyn_uop_counter = num_uop;

  ASSERT(dyn_uop_counter);

  // set eom flag and next pc address for the last uop of this instruction
  trace_uop[dyn_uop_counter - 1]->m_eom = 1;
  trace_uop[dyn_uop_counter - 1]->m_npc = pi->m_instruction_next_addr;

  STAT_CORE_EVENT(core_id, OP_CAT_GED_ILLEGAL + (pi->m_opcode));

  ASSERT(num_uop > 0);
  first_info->m_trace_info.m_num_uop = num_uop;

  DEBUG("%s: read: %d write: %d\n", g_tr_opcode_names[pi->m_opcode],
        pi->m_num_read_regs, pi->m_num_dest_regs);

  return first_info;
}

void igpu_decoder_c::convert_dyn_uop(inst_info_s *info, void *trace_info,
                                     trace_uop_s *trace_uop, Addr rep_offset,
                                     int core_id) {
  trace_info_igpu_s *pi = static_cast<trace_info_igpu_s *>(trace_info);
  core_c *core = m_simBase->m_core_pointers[core_id];
  trace_uop->m_va = 0;

  if (info->m_table_info->m_cf_type) {
    trace_uop->m_actual_taken = pi->m_actually_taken;
    trace_uop->m_target = pi->m_branch_target;
  } else if (info->m_table_info->m_mem_type) {
    if (info->m_table_info->m_mem_type == MEM_ST) {
      trace_uop->m_va = pi->m_st_vaddr;
      trace_uop->m_mem_size = pi->m_mem_write_size;
      // In rare cases, if a page fault occurs due to a write you can miss the
      // memory callback because of which m_mem_write_size will be 0.
      // Hack it up using dummy values
      if (pi->m_mem_write_size == 0) {
        trace_uop->m_va = 0xabadabad;
        trace_uop->m_mem_size = 4;
      }
    } else if ((info->m_table_info->m_mem_type == MEM_LD) ||
               (info->m_table_info->m_mem_type == MEM_PF) ||
               (info->m_table_info->m_mem_type >= MEM_SWPREF_NTA &&
                info->m_table_info->m_mem_type <= MEM_SWPREF_T2)) {
      if (info->m_trace_info.m_second_mem)
        trace_uop->m_va = pi->m_ld_vaddr2;
      else
        trace_uop->m_va = pi->m_ld_vaddr1;

      if (pi->m_mem_read_size) trace_uop->m_mem_size = pi->m_mem_read_size;
    }
  }

  // next pc
  trace_uop->m_npc = trace_uop->m_addr;
}

void igpu_decoder_c::dprint_inst(void *trace_info, int core_id, int thread_id) {
  if (m_dprint_count++ >= 50000 || !*KNOB(KNOB_DEBUG_PRINT_TRACE)) return;

  trace_info_igpu_s *t_info = static_cast<trace_info_igpu_s *>(trace_info);

  *m_dprint_output << "*** begin of the data structure *** " << endl;
  *m_dprint_output << "core_id:" << core_id << " thread_id:" << thread_id
                   << endl;
  *m_dprint_output << "uop_opcode "
                   << g_tr_opcode_names[(uint32_t)t_info->m_opcode] << endl;
  *m_dprint_output << "num_read_regs: " << hex
                   << (uint32_t)t_info->m_num_read_regs << endl;
  *m_dprint_output << "num_dest_regs: " << hex
                   << (uint32_t)t_info->m_num_dest_regs << endl;
  *m_dprint_output << "has_immediate: " << hex
                   << (uint32_t)t_info->m_has_immediate << endl;
  *m_dprint_output << "r_dir:" << (uint32_t)t_info->m_rep_dir << endl;
  *m_dprint_output << "has_st: " << hex << (uint32_t)t_info->m_has_st << endl;
  *m_dprint_output << "num_ld: " << hex << (uint32_t)t_info->m_num_ld << endl;
  *m_dprint_output << "mem_read_size: " << hex
                   << (uint32_t)t_info->m_mem_read_size << endl;
  *m_dprint_output << "mem_write_size: " << hex
                   << (uint32_t)t_info->m_mem_write_size << endl;
  *m_dprint_output << "is_fp: " << (uint32_t)t_info->m_is_fp << endl;
  *m_dprint_output << "ld_vaddr1: " << hex << (uint64_t)t_info->m_ld_vaddr1
                   << endl;
  *m_dprint_output << "ld_vaddr2: " << hex << (uint64_t)t_info->m_ld_vaddr2
                   << endl;
  *m_dprint_output << "st_vaddr: " << hex << (uint64_t)t_info->m_st_vaddr
                   << endl;
  *m_dprint_output << "instruction_addr: " << hex
                   << (uint64_t)t_info->m_instruction_addr << endl;
  *m_dprint_output << "branch_target: " << hex
                   << (uint64_t)t_info->m_branch_target << endl;
  *m_dprint_output << "actually_taken: " << hex
                   << (uint32_t)t_info->m_actually_taken << endl;
  *m_dprint_output << "write_flg: " << hex << (uint32_t)t_info->m_write_flg
                   << endl;
  *m_dprint_output << "size: " << hex << (uint64_t)t_info->m_size << endl;
  *m_dprint_output << "*** end of the data structure *** " << endl << endl;
}

const char *igpu_decoder_c::g_tr_opcode_names[GED_OPCODE_LAST] = {
  "GED_OPCODE_ILLEGAL", "GED_OPCODE_MOV",    "GED_OPCODE_SEL",
  "GED_OPCODE_MOVI",    "GED_OPCODE_NOT",    "GED_OPCODE_AND",
  "GED_OPCODE_OR",      "GED_OPCODE_XOR",    "GED_OPCODE_SHR",
  "GED_OPCODE_SHL",     "GED_OPCODE_ASR",    "GED_OPCODE_CMP",
  "GED_OPCODE_CMPN",    "GED_OPCODE_CSEL",   "GED_OPCODE_F32TO16",
  "GED_OPCODE_F16TO32", "GED_OPCODE_BFREV",  "GED_OPCODE_BFE",
  "GED_OPCODE_BFI1",    "GED_OPCODE_BFI2",   "GED_OPCODE_JMPI",
  "GED_OPCODE_BRD",     "GED_OPCODE_IF",     "GED_OPCODE_BRC",
  "GED_OPCODE_ELSE",    "GED_OPCODE_ENDIF",  "GED_OPCODE_WHILE",
  "GED_OPCODE_BREAK",   "GED_OPCODE_CONT",   "GED_OPCODE_HALT",
  "GED_OPCODE_CALL",    "GED_OPCODE_RET",    "GED_OPCODE_WAIT",
  "GED_OPCODE_SEND",    "GED_OPCODE_SENDC",  "GED_OPCODE_MATH",
  "GED_OPCODE_ADD",     "GED_OPCODE_MUL",    "GED_OPCODE_AVG",
  "GED_OPCODE_FRC",     "GED_OPCODE_RNDU",   "GED_OPCODE_RNDD",
  "GED_OPCODE_RNDE",    "GED_OPCODE_RNDZ",   "GED_OPCODE_MAC",
  "GED_OPCODE_MACH",    "GED_OPCODE_LZD",    "GED_OPCODE_FBH",
  "GED_OPCODE_FBL",     "GED_OPCODE_CBIT",   "GED_OPCODE_ADDC",
  "GED_OPCODE_SUBB",    "GED_OPCODE_SAD2",   "GED_OPCODE_SADA2",
  "GED_OPCODE_DP4",     "GED_OPCODE_DPH",    "GED_OPCODE_DP3",
  "GED_OPCODE_DP2",     "GED_OPCODE_LINE",   "GED_OPCODE_PLN",
  "GED_OPCODE_MAD",     "GED_OPCODE_LRP",    "GED_OPCODE_NOP",
  "GED_OPCODE_DIM",     "GED_OPCODE_CALLA",  "GED_OPCODE_SMOV",
  "GED_OPCODE_GOTO",    "GED_OPCODE_JOIN",   "GED_OPCODE_MADM",
  "GED_OPCODE_SENDS",   "GED_OPCODE_SENDSC", "GED_OPCODE_INVALID"};
#endif 