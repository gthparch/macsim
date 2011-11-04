//Nagesh

#ifndef INDI_H
#define INDI_H

#ifdef _MSC_VER
typedef unsigned __int8 uint8_t;
typedef unsigned __int32 uint32_t;
#else
#include <inttypes.h>
#endif
#include <stdio.h>
/* added on jan-28 by nagesh */
#include <zlib.h>
#include <ostream>
/* added on jan-28 by nagesh */
#include "xed-category-enum.h"

#define MAX_SRC_NUM 9
#define MAX_DST_NUM 6
/* added on jan-28 by nagesh */

#define USE_MAP 0

#define MAX_THREADS 1000


/**
 * Instrumentation Option
 */
enum InstrumentationOption {
  NORMAL,         /**< Full, PinPoint*/
  FINPOINT_1 = 3, /**< FuntionPoint*/
  FINPOINT_2 = 4, /**< FuntionPoint*/
};




void init_reg_compress(void) ;


/**
 * Thread Information
 */
struct Thread_info
{
  uint32_t thread_id;  /**< Thread ID */ 
  uint64_t inst_count; /**< Instruction count */
};


/**
 * Instruction Information
 */
struct Inst_info {
  uint8_t num_read_regs;      // 3-bits
  uint8_t num_dest_regs;      // 3-bits
  uint8_t src[MAX_SRC_NUM]; // 6-bits * 4
  uint8_t dst[MAX_DST_NUM]; // 6-bits * 4
  uint8_t cf_type;  // 4 bits
  bool has_immediate;  // 1bits
  uint8_t opcode; // 6 bits
  bool  has_st;  // 1 bit
  bool is_fp;    // 1bit
  bool write_flg;   // 1bit
  uint8_t num_ld;  // 2bit
  uint8_t size; // 5 bit
  // **** dynamic ****
  uint32_t ld_vaddr1; // 4 bytes
  uint32_t ld_vaddr2; // 4 bytes
  uint32_t st_vaddr;   // 4 bytes
  uint32_t instruction_addr; // 4 bytes
  uint32_t branch_target; // not the dynamic info. static info  // 4 bytes
  uint8_t mem_read_size; // 8 bit
  uint8_t mem_write_size;  // 8 bit
  bool rep_dir;  // 1 bit
  bool actually_taken; // 1 ibt
};


#define BUF_SIZE (10 * sizeof(struct Inst_info))
struct Trace_info {
  gzFile trace_stream;
  char trace_buf[BUF_SIZE];
  int bytes_accumulated;
  Inst_info inst_info;
  uint64_t inst_count;
  uint32_t vaddr1;
  uint32_t vaddr2;
  uint32_t st_vaddr;
  uint32_t target;
  uint32_t actually_taken;
  uint32_t mem_read_size;
  uint32_t mem_write_size;
  uint32_t eflags;
  ofstream* debug_stream;
};


VOID write_inst_to_file(ofstream* file, Inst_info *t_info);

enum TR_OPCODE_enum {
  TR_MUL = XED_CATEGORY_LAST ,
  TR_DIV,
  TR_FMUL,
  TR_FDIV,
  TR_NOP,
  PREFETCH_NTA,
  PREFETCH_T0,
  PREFETCH_T1,
  PREFETCH_T2,
}TR_OPCODE;


enum CF_TYPE_enum {
  NOT_CF,			// not a control flow instruction
  CF_BR,			// an unconditional branch
  CF_CBR,			// a conditional branch
  CF_CALL,			// a call
  // below this point are indirect cfs
  CF_IBR,			// an indirect branch
  CF_ICALL,			// an indirect call
  CF_ICO,			// an indirect jump to co-routine
  CF_RET,			// a return
  CF_SYS,
  CF_ICBR                     // an indirect conditional branch
}CF_TYPE;

string tr_cf_names[15] = {
  "NOT_CF",			// not a control flow instruction
  "CF_BR",			// an unconditional branch
  "CF_CBR",			// a conditional branch
  "CF_CALL",			// a call
  "CF_IBR",			// an indirect branch
  "CF_ICALL",			// an indirect call
  "CF_ICO",			// an indirect jump to co-routine
  "CF_RET",			// a return
  "CF_SYS",
  "CF_ICBR"
};


string tr_opcode_names[60] = {
  "INVALID",
  "3DNOW",
  "AES",
  "AVX",
  "BINARY",
  "BITBYTE",
  "BROADCAST",
  "CALL",
  "CMOV",
  "COND_BR",
  "CONVERT",
  "DATAXFER",
  "DECIMAL",
  "FCMOV",
  "FLAGOP",
  "INTERRUPT",
  "IO",
  "IOSTRINGOP",
  "LOGICAL",
  "MISC",
  "MMX",
  "NOP",
  "PCLMULQDQ",
  "POP",
  "PREFETCH",
  "PUSH",
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
  "UNCOND_BR",
  "VTX",
  "WIDENOP",
  "X87_ALU",
  "XSAVE",
  "XSAVEOPT",
};


/////////////////////////////////////////////////////////////////////////////////////////
/// Function forward declaration
void write_inst_to_file(ofstream*, Inst_info*);
void dprint_inst(ADDRINT, string*, THREADID);
void finish(void);
void thread_end(void);
void thread_end(THREADID threadid);

#endif //INDI_H
