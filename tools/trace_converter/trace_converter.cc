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


#include "trace_converter.h"
#include <iostream>
#include "all_knobs.h"

trace_reader_c trace_reader_c::Singleton;


trace_reader_c::trace_reader_c()
{
}

void trace_reader_c::init()
{
  if (g_knobs->KNOB_ENABLE_REUSE_DIST->getValue()) {
    trace_reader_c* reuse_distance = new reuse_distance_c;
    m_tracer.push_back(reuse_distance);
  }

  if (g_knobs->KNOB_ENABLE_COUNT_STATIC_PC->getValue()) {
    trace_reader_c* static_pc = new static_pc_c;
    m_tracer.push_back(static_pc);
  }
}

void trace_reader_c::reset()
{
  for (int ii = 0; ii < m_tracer.size(); ++ii) 
    m_tracer[ii]->reset();
}


trace_reader_c::~trace_reader_c()
{
  for (int ii = 0; ii < m_tracer.size(); ++ii) {
    delete m_tracer[ii];
  }

  m_tracer.clear();
}

#if defined(GPU_TRACE)
void trace_reader_c::inst_event(trace_info_gpu_small_s* inst)
#elif defined(ARM64_TRACE)
void trace_reader_c::inst_event(trace_info_a64_s* inst)
#else 
void trace_reader_c::inst_event(trace_info_cpu_s* inst)
#endif 

{
  for (int ii = 0; ii < m_tracer.size(); ++ii) {
    m_tracer[ii]->inst_event(inst);
  }
}


void trace_reader_c::print()
{
}


void trace_reader_c::inst_convert(trace_info_cpu_s *inst)
{
// call converstion here 
// convert opcode 

// PIN212_CPU_OPCDE_ENUM 
switch(inst->m_opcode){
  case  PIN212_XED_CATEGORY_INVALID:
      inst->m_opcode = XED_CATEGORY_INVALID;
      break;
 case  PIN212_XED_CATEGORY_3DNOW: 
      inst->m_opcode = XED_CATEGORY_3DNOW;
      break; 
 case  PIN212_XED_CATEGORY_AES:
      inst->m_opcode = XED_CATEGORY_AES;
      break; 
 case  PIN212_XED_CATEGORY_AVX:
      inst->m_opcode = XED_CATEGORY_AVX;
      break; 
  case  PIN212_XED_CATEGORY_AVX2: 
      inst->m_opcode = XED_CATEGORY_AVX2; 
      break; 
  case  PIN212_XED_CATEGORY_AVX2GATHER:
       inst->m_opcode = XED_CATEGORY_AVX2GATHER;
       break; 
  case  PIN212_XED_CATEGORY_BDW: 
        inst->m_opcode = XED_CATEGORY_BINARY; //  no category // need to check 
        break; 
  case  PIN212_XED_CATEGORY_BINARY:
        inst->m_opcode = XED_CATEGORY_BINARY;
      break; 
  case  PIN212_XED_CATEGORY_BITBYTE:
        inst->m_opcode = XED_CATEGORY_BITBYTE;
        break; 
  case  PIN212_XED_CATEGORY_BMI1:
         inst->m_opcode = XED_CATEGORY_BMI1;
        break; 
  case  PIN212_XED_CATEGORY_BMI2:
        inst->m_opcode = XED_CATEGORY_BMI2;
        break; 
  case  PIN212_XED_CATEGORY_BROADCAST:
        inst->m_opcode = XED_CATEGORY_BROADCAST;
        break; 
  case  PIN212_XED_CATEGORY_CALL:
        inst->m_opcode = XED_CATEGORY_CALL;
        break; 
  case  PIN212_XED_CATEGORY_CMOV:
        inst->m_opcode = XED_CATEGORY_CMOV;
        break; 
  case  PIN212_XED_CATEGORY_COND_BR:
        inst->m_opcode = XED_CATEGORY_COND_BR;
        break; 
  case  PIN212_XED_CATEGORY_CONVERT:
        inst->m_opcode = XED_CATEGORY_CONVERT;
        break; 
  case  PIN212_XED_CATEGORY_DATAXFER:
        inst->m_opcode = XED_CATEGORY_DATAXFER;
        break; 
  case  PIN212_XED_CATEGORY_DECIMAL:
        inst->m_opcode = XED_CATEGORY_DECIMAL;
        break; 
  case  PIN212_XED_CATEGORY_FCMOV:
        inst->m_opcode = XED_CATEGORY_FCMOV;
        break; 
  case  PIN212_XED_CATEGORY_FLAGOP:
        inst->m_opcode = XED_CATEGORY_FLAGOP;
        break; 
  case  PIN212_XED_CATEGORY_FMA4:
        inst->m_opcode = XED_CATEGORY_FMA4;
        break; 
  case  PIN212_XED_CATEGORY_INTERRUPT:
        inst->m_opcode = XED_CATEGORY_INTERRUPT;
        break; 
  case  PIN212_XED_CATEGORY_IO:
        inst->m_opcode = XED_CATEGORY_IO;
        break; 
  case  PIN212_XED_CATEGORY_IOSTRINGOP:
        inst->m_opcode = XED_CATEGORY_IOSTRINGOP;
        break; 
  case  PIN212_XED_CATEGORY_LOGICAL:
        inst->m_opcode = XED_CATEGORY_LOGICAL;
        break; 
  case  PIN212_XED_CATEGORY_LZCNT:
        inst->m_opcode = XED_CATEGORY_LZCNT;
        break; 
  case  PIN212_XED_CATEGORY_MISC:
        inst->m_opcode = XED_CATEGORY_MISC;
        break; 
  case  PIN212_XED_CATEGORY_MMX:
        inst->m_opcode = XED_CATEGORY_MMX;
        break; 
  case  PIN212_XED_CATEGORY_NOP:
        inst->m_opcode = XED_CATEGORY_NOP;
        break; 
  case  PIN212_XED_CATEGORY_PCLMULQDQ:
        inst->m_opcode = XED_CATEGORY_PCLMULQDQ;
        break; 
  case  PIN212_XED_CATEGORY_POP:
        inst->m_opcode = XED_CATEGORY_POP;
        break; 
  case  PIN212_XED_CATEGORY_PREFETCH:
        inst->m_opcode = XED_CATEGORY_PREFETCH;
        break; 
  case  PIN212_XED_CATEGORY_PUSH:
        inst->m_opcode = XED_CATEGORY_PUSH;
        break; 
  case  PIN212_XED_CATEGORY_RDRAND:
        inst->m_opcode = XED_CATEGORY_RDRAND;
        break; 
  case  PIN212_XED_CATEGORY_RDSEED:
        inst->m_opcode = XED_CATEGORY_RDSEED;
        break; 
  case  PIN212_XED_CATEGORY_RDWRFSGS:
        inst->m_opcode = XED_CATEGORY_RDWRFSGS;
        break; 
  case  PIN212_XED_CATEGORY_RET:
        inst->m_opcode = XED_CATEGORY_RET;
        break; 
  case  PIN212_XED_CATEGORY_ROTATE:
        inst->m_opcode = XED_CATEGORY_ROTATE;
        break; 
  case  PIN212_XED_CATEGORY_SEGOP:
        inst->m_opcode = XED_CATEGORY_SEGOP;
        break; 
  case  PIN212_XED_CATEGORY_SEMAPHORE:
        inst->m_opcode = XED_CATEGORY_SEMAPHORE;
        break; 
  case  PIN212_XED_CATEGORY_SHIFT:
        inst->m_opcode = XED_CATEGORY_SHIFT;
        break; 
  case  PIN212_XED_CATEGORY_SSE:
        inst->m_opcode = XED_CATEGORY_SSE;
        break; 
  case  PIN212_XED_CATEGORY_STRINGOP:
        inst->m_opcode = XED_CATEGORY_STRINGOP;
        break; 
  case  PIN212_XED_CATEGORY_STTNI:
        inst->m_opcode = XED_CATEGORY_STTNI;
        break; 
  case  PIN212_XED_CATEGORY_SYSCALL:
        inst->m_opcode = XED_CATEGORY_SYSCALL;
        break; 
  case  PIN212_XED_CATEGORY_SYSRET:
        inst->m_opcode = XED_CATEGORY_SYSRET;
        break; 
  case  PIN212_XED_CATEGORY_SYSTEM:
        inst->m_opcode = XED_CATEGORY_SYSTEM;
        break; 
  case  PIN212_XED_CATEGORY_TBM:
        inst->m_opcode = XED_CATEGORY_TBM;
        break; 
  case  PIN212_XED_CATEGORY_UNCOND_BR:
        inst->m_opcode = XED_CATEGORY_UNCOND_BR;
        break; 
  case  PIN212_XED_CATEGORY_VFMA:
        inst->m_opcode = XED_CATEGORY_VFMA;
        break; 
  case  PIN212_XED_CATEGORY_VTX:
        inst->m_opcode = XED_CATEGORY_VTX;
        break; 
  case  PIN212_XED_CATEGORY_WIDENOP:
        inst->m_opcode = XED_CATEGORY_WIDENOP;
        break; 
  case  PIN212_XED_CATEGORY_X87_ALU:
        inst->m_opcode = XED_CATEGORY_X87_ALU;
        break; 
  case  PIN212_XED_CATEGORY_XOP:
        inst->m_opcode = XED_CATEGORY_XOP;
        break; 
  case  PIN212_XED_CATEGORY_XSAVE:
        inst->m_opcode = XED_CATEGORY_XSAVE;
        break; 
  case  PIN212_XED_CATEGORY_XSAVEOPT:
        inst->m_opcode = XED_CATEGORY_XSAVEOPT;
        break; 
  case  PIN212_TR_MUL:
        inst->m_opcode = TR_MUL;
        break; 
  case  PIN212_TR_DIV:
        inst->m_opcode = TR_DIV;
        break; 
  case  PIN212_TR_FMUL:
        inst->m_opcode = TR_FMUL;
        break; 
  case  PIN212_TR_FDIV:
        inst->m_opcode = TR_FDIV;
        break; 
  case  PIN212_TR_NOP:
        inst->m_opcode = TR_NOP;
        break; 
  case  PIN212_PREFETCH_NTA:
        inst->m_opcode = PREFETCH_NTA;
        break; 
  case  PIN212_PREFETCH_T0:
        inst->m_opcode = PREFETCH_T0;
        break; 
  case  PIN212_PREFETCH_T1:
        inst->m_opcode = PREFETCH_T1;
        break; 
  case  PIN212_PREFETCH_T2:
        inst->m_opcode = PREFETCH_T2;
        break; 
  case  PIN212_GPU_EN:
        inst->m_opcode = XED_CATEGORY_INVALID;
        break; 
  case  PIN212_CPU_OPCODE_LAST:
        inst->m_opcode = CPU_OPCODE_LAST;
        break;
  default: 
    std::cerr<< "wrong opcode!!";


}


}


/////////////////////////////////////////////////////////////////////////////////////////


reuse_distance_c::reuse_distance_c()
{
  m_name = "reuse_distance";
  m_self_counter = 0;
}


reuse_distance_c::~reuse_distance_c()
{
}


#if defined(GPU_TRACE)
void reuse_distance_c::inst_event(trace_info_gpu_small_s* inst) { 
 if (inst->m_is_load) {
    ++m_self_counter;
    Addr addr;
    addr = inst->m_mem_addr; 
    
    addr = addr >> 6;

    if (m_reuse_map.find(addr) != m_reuse_map.end()) {
      bool same_pc = false;
      if (inst->m_inst_addr == m_reuse_pc_map[addr]) same_pc = true;
      cout << dec << m_self_counter << " dist:" << dec << m_self_counter - m_reuse_map[addr] << " addr:" 
           << hex << addr << " pc:" << hex << m_reuse_pc_map[addr] << " " << hex << inst->m_inst_addr << "\n";
    }
    else {
      cout << hex << inst->m_inst_addr << "\n";
    }
    m_reuse_map[addr] = m_self_counter;
    m_reuse_pc_map[addr] = inst->m_inst_addr;
  }

}
#elif defined(ARM64_TRACE)
void reuse_distance_c::inst_event(trace_info_a64_s* inst) 
{
 if (static_cast<int>(inst->m_num_ld) > 0 or inst->m_has_st == true) {
    ++m_self_counter;
    Addr addr;
    if (inst->m_has_st) {
      addr = inst->m_st_vaddr;
    } else {
      addr = inst->m_ld_vaddr1;
    }

    addr = addr >> 6;

    if (m_reuse_map.find(addr) != m_reuse_map.end()) {
      bool same_pc = false;
      if (inst->m_instruction_addr == m_reuse_pc_map[addr]) same_pc = true;
      cout << dec << m_self_counter << " dist:" << dec << m_self_counter - m_reuse_map[addr] << " addr:" 
           << hex << addr << " pc:" << hex << m_reuse_pc_map[addr] << " " << hex << inst->m_instruction_addr << "\n";
    }
    else {
      cout << hex << inst->m_instruction_addr << "\n";
    }
    m_reuse_map[addr] = m_self_counter;
    m_reuse_pc_map[addr] = inst->m_instruction_addr;
  }
}
#else 
void reuse_distance_c::inst_event(trace_info_cpu_s* inst)
{
  if (static_cast<int>(inst->m_num_ld) > 0 or inst->m_has_st == true) {
    ++m_self_counter;
    Addr addr;
    if (inst->m_has_st) {
      addr = inst->m_st_vaddr;
    } else {
      addr = inst->m_ld_vaddr1;
    }

    addr = addr >> 6;

    if (m_reuse_map.find(addr) != m_reuse_map.end()) {
      bool same_pc = false;
      if (inst->m_instruction_addr == m_reuse_pc_map[addr]) same_pc = true;
      cout << dec << m_self_counter << " dist:" << dec << m_self_counter - m_reuse_map[addr] << " addr:" 
           << hex << addr << " pc:" << hex << m_reuse_pc_map[addr] << " " << hex << inst->m_instruction_addr << "\n";
    }
    else {
      cout << hex << inst->m_instruction_addr << "\n";
    }
    m_reuse_map[addr] = m_self_counter;
    m_reuse_pc_map[addr] = inst->m_instruction_addr;
  }
}
#endif 


void reuse_distance_c::print()
{
  std::cout << m_name << "\n";
}


void reuse_distance_c::reset()
{
  m_reuse_map.clear();
}


/////////////////////////////////////////////////////////////////////////////////////////


static_pc_c::static_pc_c() 
{
  m_total_inst_count = 0;
  m_total_load_count = 0;
}

static_pc_c::~static_pc_c() 
{
  cout << "Total static pc:" << m_static_pc.size() << "\n";
  cout << "Total static memory pc:" << m_static_mem_pc.size() << "\n";

  cout << "Total instruction count:" << m_total_inst_count << "\n";
  cout << "Total load count:" << m_total_load_count << "\n";

  m_static_pc.clear();
  m_static_mem_pc.clear();
}



#if defined(GPU_TRACE)
void static_pc_c::inst_event(trace_info_gpu_small_s* inst)
{
  m_static_pc[inst->m_inst_addr] = true;
  if (inst->m_is_load) { 
    m_static_mem_pc[inst->m_inst_addr] = true;
    ++m_total_load_count; 
  }
  ++m_total_inst_count;
}


#elif defined(ARM64_TRACE)
void static_pc_c::inst_event(trace_info_a64_s* inst)
{
  m_static_pc[inst->m_instruction_addr] = true;
  if (static_cast<int>(inst->m_num_ld) > 0 or inst->m_has_st == true) {
    m_static_mem_pc[inst->m_instruction_addr] = true;
  }


  ++m_total_inst_count;
  if (inst->m_num_ld == true)
    ++m_total_load_count;
}

#else 
void static_pc_c::inst_event(trace_info_cpu_s* inst)
{

  m_static_pc[inst->m_instruction_addr] = true;
  if (static_cast<int>(inst->m_num_ld) > 0 or inst->m_has_st == true) {
    m_static_mem_pc[inst->m_instruction_addr] = true;
  }


  ++m_total_inst_count;
  if (inst->m_has_st == true)
    ++m_total_load_count;
}
#endif 




















char* trace_reader_c::tr_opcode_names_pin311[MAX_TR_OPCODE] = {
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
  "BINARY",
  "BITBYTE",
  "BLEND",
  "BMI1",
  "BMI2",
  "BROADCAST",
  "CALL",
  "CET",
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
  "VPCLMULQDQ",
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
  "CPU_OPCODE_LAST",
};





























































































char* trace_reader_c::tr_opcode_names_pin212[MAX_TR_OPCODE] = {
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


char* trace_reader_c::tr_reg_names_pin311[MAX_TR_REG] = {
  "*invalid*",
   "*none*",
   "*UNKNOWN REG 2*",
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
   "xmm16",
   "xmm17",
   "xmm18",
   "xmm19",
   "xmm20",
   "xmm21",
   "xmm22",
   "xmm23",
   "xmm24",
   "xmm25",
   "xmm26",
   "xmm27",
   "xmm28",
   "xmm29",
   "xmm30",
   "xmm31",
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
   "ymm16",
   "ymm17",
   "ymm18",
   "ymm19",
   "ymm20",
   "ymm21",
   "ymm22",
   "ymm23",
   "ymm24",
   "ymm25",
   "ymm26",
   "ymm27",
   "ymm28",
   "ymm29",
   "ymm30",
   "ymm31",
   "zmm0",
   "zmm1",
   "zmm2",
   "zmm3",
   "zmm4",
   "zmm5",
   "zmm6",
   "zmm7",
   "zmm8",
   "zmm9",
   "zmm10",
   "zmm11",
   "zmm12",
   "zmm13",
   "zmm14",
   "zmm15",
   "zmm16",
   "zmm17",
   "zmm18",
   "zmm19",
   "zmm20",
   "zmm21",
   "zmm22",
   "zmm23",
   "zmm24",
   "zmm25",
   "zmm26",
   "zmm27",
   "zmm28",
   "zmm29",
   "zmm30",
   "zmm31",
   "k0",
   "k1",
   "k2",
   "k3",
   "k4",
   "k5",
   "k6",
   "k7",
   "mxcsr",
   "mxcsrmask",
   "orig_rax",
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
   "tr",
   "tr3",
   "tr4",
   "tr5",
   "tr6",
   "tr7",
   "r_status_flags",
   "rdf",
   "seg_gs_base",
   "seg_fs_base",
   "inst_g0",
   "inst_g1",
   "inst_g2",
   "inst_g3",
   "inst_g4",
   "inst_g5",
   "inst_g6",
   "inst_g7",
   "inst_g8",
   "inst_g9",
   "inst_g10",
   "inst_g11",
   "inst_g12",
   "inst_g13",
   "inst_g14",
   "inst_g15",
   "inst_g16",
   "inst_g17",
   "inst_g18",
   "inst_g19",
   "inst_g20",
   "inst_g21",
   "inst_g22",
   "inst_g23",
   "inst_g24",
   "inst_g25",
   "inst_g26",
   "inst_g27",
   "inst_g28",
   "inst_g29",
   "buf_base0",
   "buf_base1",
   "buf_base2",
   "buf_base3",
   "buf_base4",
   "buf_base5",
   "buf_base6",
   "buf_base7",
   "buf_base8",
   "buf_base9",
   "buf_end0",
   "buf_end1",
   "buf_end2",
   "buf_end3",
   "buf_end4",
   "buf_end5",
   "buf_end6",
   "buf_end7",
   "buf_end8",
   "buf_end9",
   "inst_g0d",
   "inst_g1d",
   "inst_g2d",
   "inst_g3d",
   "inst_g4d",
   "inst_g5d",
   "inst_g6d",
   "inst_g7d",
   "inst_g8d",
   "inst_g9d",
   "inst_g10d",
   "inst_g11d",
   "inst_g12d",
   "inst_g13d",
   "inst_g14d",
   "inst_g15d",
   "inst_g16d",
   "inst_g17d",
   "inst_g18d",
   "inst_g19d",
   "inst_g20d",
   "inst_g21d",
   "inst_g22d",
   "inst_g23d",
   "inst_g24d",
   "inst_g25d",
   "inst_g26d",
   "inst_g27d",
   "inst_g28d",
   "inst_g29d",
};
char* trace_reader_c::tr_reg_names_pin212[MAX_TR_REG] = {
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