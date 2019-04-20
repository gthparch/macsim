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


#ifndef TRACE_GENERATOR_H
#define TRACE_GENERATOR_H

#ifdef _MSC_VER
typedef unsigned __int8 uint8_t;
typedef unsigned __int32 uint32_t;
#else
#include <inttypes.h>
#endif


#include <stdio.h>
#include <zlib.h>
#include <ostream>
#include "xed-category-enum.h"


#define MAX_SRC_NUM 9
#define MAX_DST_NUM 6
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


/**
 * Thread Information
 */
struct Thread_info
{
  uint32_t thread_id;  /**< Thread ID */ 
  uint64_t inst_count; /**< Instruction count */
};

// new version april-2019
/**
 * Instruction Information
 */
struct Inst_info {
  uint8_t  num_read_regs;       // 3-bits
  uint8_t  num_dest_regs;       // 3-bits
  uint16_t  src[MAX_SRC_NUM];    // incresed in 2019 version // 6-bits * 4
  uint16_t  dst[MAX_DST_NUM];    // increased in 2019 version  6-bits * 4
  uint8_t  cf_type;             // 4 bits
  bool     has_immediate;       // 1bits
  uint8_t  opcode;              // 6 bits
  bool     has_st;              // 1 bit
  bool     is_fp;               // 1bit
  bool     write_flg;           // 1bit
  uint8_t  num_ld;              // 2bit
  uint8_t  size;                // 5 bit
  // **** dynamic ****
  uint64_t ld_vaddr1;           // 4 bytes
  uint64_t ld_vaddr2;           // 4 bytes
  uint64_t st_vaddr;            // 4 bytes
  uint64_t instruction_addr;    // 4 bytes
  uint64_t branch_target;       // not the dynamic info. static info  // 4 bytes
  uint8_t  mem_read_size;       // 8 bit
  uint8_t  mem_write_size;      // 8 bit
  bool     rep_dir;             // 1 bit
  bool     actually_taken;      // 1 ibt
};

#define BUF_SIZE (10 * sizeof(struct Inst_info))
struct Trace_info {
  gzFile    trace_stream;
  char      trace_buf[BUF_SIZE];
  int       bytes_accumulated;
  Inst_info inst_info;
  uint64_t  inst_count;
  uint64_t  vaddr1;
  uint64_t  vaddr2;
  uint64_t  st_vaddr;
  uint64_t  target;
  uint32_t  actually_taken;
  uint32_t  mem_read_size;
  uint32_t  mem_write_size;
  uint32_t  eflags;
  ofstream* debug_stream;
};


enum CPU_OPCODE_enum {
  TR_MUL = XED_CATEGORY_LAST ,
  TR_DIV,
  TR_FMUL,
  TR_FDIV,
  TR_NOP,
  PREFETCH_NTA,
  PREFETCH_T0,
  PREFETCH_T1,
  PREFETCH_T2,
  GPU_EN,
  CPU_OPCODE_LAST,
} CPU_OPCODE;


enum CF_TYPE_enum {
  NOT_CF,   // not a control flow instruction
  CF_BR,   // an unconditional branch
  CF_CBR,   // a conditional branch
  CF_CALL,  // a call
  // below this point are indirect cfs
  CF_IBR,   // an indirect branch
  CF_ICALL,   // an indirect call
  CF_ICO,   // an indirect jump to co-routine
  CF_RET,   // a return
  CF_SYS,
  CF_ICBR     // an indirect conditional branch
} CF_TYPE;


string tr_cf_names[15] = {
  "NOT_CF",   // not a control flow instruction
  "CF_BR",    // an unconditional branch
  "CF_CBR",   // a conditional branch
  "CF_CALL",  // a call
  "CF_IBR",   // an indirect branch
  "CF_ICALL", // an indirect call
  "CF_ICO",   // an indirect jump to co-routine
  "CF_RET",   // a return
  "CF_SYS",
  "CF_ICBR"
};

string tr_opcode_names[97] = {
  "XED_CATEGORY_INVALID",
  "XED_CATEGORY_3DNOW",
  "XED_CATEGORY_ADOX_ADCX",
  "XED_CATEGORY_AES",
  "XED_CATEGORY_AVX",
  "XED_CATEGORY_AVX2",
  "XED_CATEGORY_AVX2GATHER",
  "XED_CATEGORY_AVX512",
  "XED_CATEGORY_AVX512_4FMAPS",
  "XED_CATEGORY_AVX512_4VNNIW",
  "XED_CATEGORY_AVX512_BITALG",
  "XED_CATEGORY_AVX512_VBMI",
  "XED_CATEGORY_BINARY",
  "XED_CATEGORY_BITBYTE",
  "XED_CATEGORY_BLEND",
  "XED_CATEGORY_BMI1",
  "XED_CATEGORY_BMI2",
  "XED_CATEGORY_BROADCAST",
  "XED_CATEGORY_CALL",
  "XED_CATEGORY_CET",
  "XED_CATEGORY_CLFLUSHOPT",
  "XED_CATEGORY_CLWB",
  "XED_CATEGORY_CLZERO",
  "XED_CATEGORY_CMOV",
  "XED_CATEGORY_COMPRESS",
  "XED_CATEGORY_COND_BR",
  "XED_CATEGORY_CONFLICT",
  "XED_CATEGORY_CONVERT",
  "XED_CATEGORY_DATAXFER",
  "XED_CATEGORY_DECIMAL",
  "XED_CATEGORY_EXPAND",
  "XED_CATEGORY_FCMOV",
  "XED_CATEGORY_FLAGOP",
  "XED_CATEGORY_FMA4",
  "XED_CATEGORY_GATHER",
  "XED_CATEGORY_GFNI",
  "XED_CATEGORY_IFMA",
  "XED_CATEGORY_INTERRUPT",
  "XED_CATEGORY_IO",
  "XED_CATEGORY_IOSTRINGOP",
  "XED_CATEGORY_KMASK",
  "XED_CATEGORY_LOGICAL",
  "XED_CATEGORY_LOGICAL_FP",
  "XED_CATEGORY_LZCNT",
  "XED_CATEGORY_MISC",
  "XED_CATEGORY_MMX",
  "XED_CATEGORY_MPX",
  "XED_CATEGORY_NOP",
  "XED_CATEGORY_PCLMULQDQ",
  "XED_CATEGORY_PCONFIG",
  "XED_CATEGORY_PKU",
  "XED_CATEGORY_POP",
  "XED_CATEGORY_PREFETCH",
  "XED_CATEGORY_PREFETCHWT1",
  "XED_CATEGORY_PT",
  "XED_CATEGORY_PUSH",
  "XED_CATEGORY_RDPID",
  "XED_CATEGORY_RDRAND",
  "XED_CATEGORY_RDSEED",
  "XED_CATEGORY_RDWRFSGS",
  "XED_CATEGORY_RET",
  "XED_CATEGORY_ROTATE",
  "XED_CATEGORY_SCATTER",
  "XED_CATEGORY_SEGOP",
  "XED_CATEGORY_SEMAPHORE",
  "XED_CATEGORY_SETCC",
  "XED_CATEGORY_SGX",
  "XED_CATEGORY_SHA",
  "XED_CATEGORY_SHIFT",
  "XED_CATEGORY_SMAP",
  "XED_CATEGORY_SSE",
  "XED_CATEGORY_STRINGOP",
  "XED_CATEGORY_STTNI",
  "XED_CATEGORY_SYSCALL",
  "XED_CATEGORY_SYSRET",
  "XED_CATEGORY_SYSTEM",
  "XED_CATEGORY_TBM",
  "XED_CATEGORY_UNCOND_BR",
  "XED_CATEGORY_VAES",
  "XED_CATEGORY_VBMI2",
  "XED_CATEGORY_VFMA",
  "XED_CATEGORY_VPCLMULQDQ",
  "XED_CATEGORY_VTX",
  "XED_CATEGORY_WIDENOP",
  "XED_CATEGORY_X87_ALU",
  "XED_CATEGORY_XOP",
  "XED_CATEGORY_XSAVE",
  "XED_CATEGORY_XSAVEOPT",
  "TR_MUL",
  "TR_DIV",
  "TR_FMUL",
  "TR_FDIV",
  "TR_NOP",
  "PREFETCH_NTA",
  "PREFETCH_T0",
  "PREFETCH_T1",
  "PREFETCH_T2",
};

/*
string tr_opcode_names[66] = {
  "INVALID",
  "3DNOW",
  "AES",
  "AVX",
  "AVX2", // new
  "AVX2GATHER", // new
  "BDW", // new
  "BINARY",
  "BITBYTE",
  "BMI1", // new
  "BMI2", // new
  "BROADCAST",
  "CALL",
  "CMOV",
  "COND_BR",
  "CONVERT",
  "DATAXFER",
  "DECIMAL",
  "FCMOV",
  "FLAGOP",
  "FMA4", // new
  "INTERRUPT",
  "IO",
  "IOSTRINGOP",
  "LOGICAL",
  "LZCNT", // new
  "MISC",
  "MMX",
  "NOP",
  "PCLMULQDQ",
  "POP",
  "PREFETCH",
  "PUSH",
  "RDRAND", // new
  "RDSEED", // new
  "RDWRFSGS", // new
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
  "TBM", // new
  "UNCOND_BR",
  "VFMA", // new
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
*/

/////////////////////////////////////////////////////////////////////////////////////////
/// Function forward declaration
void write_inst_to_file(ofstream*, Inst_info*);
void dprint_inst(ADDRINT, string*, THREADID);
void finish(void);
void thread_end(void);
void thread_end(THREADID threadid);
void init_reg_compress(void) ;


#endif // TRACE_GENERATOR_H
