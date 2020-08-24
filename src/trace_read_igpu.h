#ifndef _TRACE_READ_IGPU_H
#define _TRACE_READ_IGPU_H

#include "trace_read_cpu.h"

typedef enum GED_OPCODE_ENUM_ {
  GED_OPCODE_ILLEGAL = 0,
  GED_OPCODE_MOV,
  GED_OPCODE_SEL,
  GED_OPCODE_MOVI,
  GED_OPCODE_NOT,
  GED_OPCODE_AND,
  GED_OPCODE_OR,
  GED_OPCODE_XOR,
  GED_OPCODE_SHR,
  GED_OPCODE_SHL,
  GED_OPCODE_SMOV,
  GED_OPCODE_ASR,
  GED_OPCODE_CMP,
  GED_OPCODE_CMPN,
  GED_OPCODE_CSEL,
  GED_OPCODE_BFREV,
  GED_OPCODE_BFE,
  GED_OPCODE_BFI1,
  GED_OPCODE_BFI2,
  GED_OPCODE_JMPI,
  GED_OPCODE_BRD,
  GED_OPCODE_IF,
  GED_OPCODE_BRC,
  GED_OPCODE_ELSE,
  GED_OPCODE_ENDIF,
  GED_OPCODE_WHILE,
  GED_OPCODE_BREAK,
  GED_OPCODE_CONT,
  GED_OPCODE_HALT,
  GED_OPCODE_CALLA,
  GED_OPCODE_CALL,
  GED_OPCODE_RET,
  GED_OPCODE_GOTO,
  GED_OPCODE_JOIN,
  GED_OPCODE_WAIT,
  GED_OPCODE_SEND,
  GED_OPCODE_SENDC,
  GED_OPCODE_SENDS,
  GED_OPCODE_SENDSC,
  GED_OPCODE_MATH,
  GED_OPCODE_ADD,
  GED_OPCODE_MUL,
  GED_OPCODE_AVG,
  GED_OPCODE_FRC,
  GED_OPCODE_RNDU,
  GED_OPCODE_RNDD,
  GED_OPCODE_RNDE,
  GED_OPCODE_RNDZ,
  GED_OPCODE_MAC,
  GED_OPCODE_MACH,
  GED_OPCODE_LZD,
  GED_OPCODE_FBH,
  GED_OPCODE_FBL,
  GED_OPCODE_CBIT,
  GED_OPCODE_ADDC,
  GED_OPCODE_SUBB,
  GED_OPCODE_SAD2,
  GED_OPCODE_SADA2,
  GED_OPCODE_DP4,
  GED_OPCODE_DPH,
  GED_OPCODE_DP3,
  GED_OPCODE_DP2,
  GED_OPCODE_LINE,
  GED_OPCODE_PLN,
  GED_OPCODE_MAD,
  GED_OPCODE_LRP,
  GED_OPCODE_MADM,
  GED_OPCODE_NOP,
  GED_OPCODE_ROR,
  GED_OPCODE_ROL,
  GED_OPCODE_DP4A,
  GED_OPCODE_F32TO16,
  GED_OPCODE_F16TO32,
  GED_OPCODE_DIM,
  GED_OPCODE_INVALID,
  GED_OPCODE_LAST  // To mark the end of the list of instructions
} GED_OPCODE;

class igpu_decoder_c : public cpu_decoder_c
{
public:
  igpu_decoder_c(macsim_c *simBase, ofstream *dprint_output)
    : cpu_decoder_c(simBase, dprint_output) {
    m_trace_size = sizeof(trace_info_igpu_s) - sizeof(uint64_t);
  }

  static const char
    *g_tr_opcode_names[GED_OPCODE_LAST]; /**< opcode name string */

private:
  void init_pin_convert();
  inst_info_s *convert_pinuop_to_t_uop(void *pi, trace_uop_s **trace_uop,
                                       int core_id, int sim_thread_id);
  void convert_dyn_uop(inst_info_s *info, void *pi, trace_uop_s *trace_uop,
                       Addr rep_offset, int core_id);
  bool get_uops_from_traces(int core_id, uop_c *uop, int sim_thread_id);
  inst_info_s *get_inst_info(thread_s *thread_trace_info, int core_id,
                             int sim_thread_id);
  void dprint_inst(void *t_info, int core_id, int thread_id);
};

#endif
