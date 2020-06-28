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
enum InstrumentationOption
{
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

/**
 * Instruction Information
 */
struct Inst_info
{
  uint8_t num_read_regs;    // 3-bits
  uint8_t num_dest_regs;    // 3-bits
  uint8_t src[MAX_SRC_NUM]; // increased in 2019 version // 6-bits * 4 // back to 8
  uint8_t dst[MAX_DST_NUM]; // increased in 2019 version  6-bits * 4 // back to 8
  uint8_t cf_type;          // 4 bits
  bool has_immediate;       // 1bits
  uint8_t opcode;           // 6 bits
  bool has_st;              // 1 bit
  bool is_fp;               // 1bit
  bool write_flg;           // 1bit
  uint8_t num_ld;           // 2bit
  uint8_t size;             // 5 bit
  // **** dynamic ****
  uint64_t ld_vaddr1;        // 4 bytes
  uint64_t ld_vaddr2;        // 4 bytes
  uint64_t st_vaddr;         // 4 bytes
  uint64_t instruction_addr; // 4 bytes
  uint64_t branch_target;    // not the dynamic info. static info  // 4 bytes
  uint8_t mem_read_size;     // 8 bit
  uint8_t mem_write_size;    // 8 bit
  bool rep_dir;              // 1 bit
  bool actually_taken;       // 1 ibt
};

#define BUF_SIZE (10 * sizeof(struct Inst_info))
struct Trace_info
{
  gzFile trace_stream;
  char trace_buf[BUF_SIZE];
  int bytes_accumulated;
  Inst_info inst_info;
  uint64_t inst_count;
  uint64_t vaddr1;
  uint64_t vaddr2;
  uint64_t st_vaddr;
  uint64_t target;
  uint32_t actually_taken;
  uint32_t mem_read_size;
  uint32_t mem_write_size;
  uint32_t eflags;
  ofstream *debug_stream;
};

enum CPU_OPCODE_enum
{
  TR_MUL = XED_CATEGORY_LAST,
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

enum CF_TYPE_enum
{
  NOT_CF,  // not a control flow instruction
  CF_BR,   // an unconditional branch
  CF_CBR,  // a conditional branch
  CF_CALL, // a call
  // below this point are indirect cfs
  CF_IBR,   // an indirect branch
  CF_ICALL, // an indirect call
  CF_ICO,   // an indirect jump to co-routine
  CF_RET,   // a return
  CF_SYS,
  CF_ICBR // an indirect conditional branch
} CF_TYPE;

string tr_cf_names[10] = {
    "NOT_CF",   // not a control flow instruction
    "CF_BR",    // an unconditional branch
    "CF_CBR",   // a conditional branch
    "CF_CALL",  // a call
    "CF_IBR",   // an indirect branch
    "CF_ICALL", // an indirect call
    "CF_ICO",   // an indirect jump to co-routine
    "CF_RET",   // a return
    "CF_SYS",
    "CF_ICBR"};

string tr_opcode_names[106] = {
    "INVALID",
    "3DNOW",
    "ADOX_ADCX",
    "AES",
    "AVX",
    "AVX2",
    "AVX2GATHER",
    "AVX512",
    "AVX512_4FMAPS",
    "AVX512_4VNNIW",
    "AVX512_BITALG",
    "AVX512_VBMI",
    "AVX512_VP2INTERSECT",
    "BINARY",
    "BITBYTE",
    "BLEND",
    "BMI1",
    "BMI2",
    "BROADCAST",
    "CALL",
    "CET",
    "CLDEMOTE",
    "CLFLUSHOPT",
    "CLWB",
    "CLZERO",
    "CMOV",
    "COMPRESS",
    "COND_BR",
    "CONFLICT",
    "CONVERT",
    "DATAXFER",
    "DECIMAL",
    "ENQCMD",
    "EXPAND",
    "FCMOV",
    "FLAGOP",
    "FMA4",
    "GATHER",
    "GFNI",
    "IFMA",
    "INTERRUPT",
    "IO",
    "IOSTRINGOP",
    "KMASK",
    "LOGICAL",
    "LOGICAL_FP",
    "LZCNT",
    "MISC",
    "MMX",
    "MOVDIR",
    "MPX",
    "NOP",
    "PCLMULQDQ",
    "PCONFIG",
    "PKU",
    "POP",
    "PREFETCH",
    "PREFETCHWT1",
    "PT",
    "PUSH",
    "RDPID",
    "RDPRU",
    "RDRAND",
    "RDSEED",
    "RDWRFSGS",
    "RET",
    "ROTATE",
    "SCATTER",
    "SEGOP",
    "SEMAPHORE",
    "SETCC",
    "SGX",
    "SHA",
    "SHIFT",
    "SMAP",
    "SSE",
    "STRINGOP",
    "STTNI",
    "SYSCALL",
    "SYSRET",
    "SYSTEM",
    "TBM",
    "UNCOND_BR",
    "VAES",
    "VBMI2",
    "VFMA",
    "VIA_PADLOCK",
    "VPCLMULQDQ",
    "VTX",
    "WAITPKG",
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
    "CPU_OPCODE_LAST",
};

/////////////////////////////////////////////////////////////////////////////////////////
/// Function forward declaration
void write_inst_to_file(ofstream *, Inst_info *);
void dprint_inst(ADDRINT, string *, THREADID);
void finish(void);
void thread_end(void);
void thread_end(THREADID threadid);
void init_reg_compress(void);

#endif // TRACE_GENERATOR_H
