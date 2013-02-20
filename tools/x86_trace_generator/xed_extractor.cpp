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


// ============================================================================
// Author: Jaekyu Lee (kacear@gmail.com)
// Date: November 3, 2011
// NOTE: Extract PIN::XED information
// ============================================================================

#include "pin.H"
#include <iostream>
#include <map>


INT32 usage()
{
  cerr << KNOB_BASE::StringKnobSummary() << endl;
  return -1;
}
  

void extract_register(void)
{
  cout << "const char *trace_read_c::g_tr_reg_names[MAX_TR_REG] = {\n";
  map<LEVEL_BASE::REG, bool> full_set;
  for (int ii = 0; ii < REG_PIN_BASE; ++ii) {
    full_set[REG_FullRegName(static_cast<LEVEL_BASE::REG>(ii))] = true;
    cout << "   \"" << REG_StringShort(REG_FullRegName(static_cast<LEVEL_BASE::REG>(ii))) << "\",\n";
  }
  cout << "};\n";

  cout << full_set.size() << "\n";
}

void extract_category(void)
{
  int num_opcodes = XED_CATEGORY_LAST + 10;
  cout << "\n";
  cout << "string tr_opcode_names[" << num_opcodes << "] = {\n";
  for (int ii = 0; ii < XED_CATEGORY_LAST; ++ii) {
    cout << "  \"" << CATEGORY_StringShort(ii) << "\",\n";
  }
  cout << "  \"TR_MUL\",\n"; 
  cout << "  \"TR_DIV\",\n";
  cout << "  \"TR_FMUL\",\n";
  cout << "  \"TR_FDIV\",\n";
  cout << "  \"TR_NOP\",\n";
  cout << "  \"PREFETCH_NTA\",\n";
  cout << "  \"PREFETCH_T0\",\n";
  cout << "  \"PREFETCH_T1\",\n";
  cout << "  \"PREFETCH_T2\",\n";
  cout << "  \"GPU_EN\",\n";
  cout << "};\n";
}


void extract_opcode(void)
{
  for (int ii = 0; ii < XED_ICLASS_LAST; ++ii) {
//    if (OPCODE_StringShort(ii).find("NOP") != string::npos)
    cout << OPCODE_StringShort(ii) << "\n";
  }
}


void macsim_opcode_enum(void)
{
  cout << "typedef enum CPU_OPCODE_ENUM_ {\n";
  for (int ii = 0; ii < XED_CATEGORY_LAST; ++ii) {
    cout << "  XED_CATEGORY_" << CATEGORY_StringShort(ii) << ",\n";
  }
  cout << "  TR_MUL,\n"; 
  cout << "  TR_DIV,\n";
  cout << "  TR_FMUL,\n";
  cout << "  TR_FDIV,\n";
  cout << "  TR_NOP,\n";
  cout << "  PREFETCH_NTA,\n";
  cout << "  PREFETCH_T0,\n";
  cout << "  PREFETCH_T1,\n";
  cout << "  PREFETCH_T2,\n";
  cout << "  GPU_EN,\n";
  cout << "  CPU_OPCODE_LAST,\n";
  cout << "} CPU_OPCODE_ENUM;\n";
}


void macsim_opcode(void)
{
  cout << "const char* cpu_decoder_c::g_tr_opcode_names[MAX_TR_OPCODE_NAME] = {\n";
  for (int ii = 0; ii < XED_CATEGORY_LAST; ++ii) {
    cout << "  \"" << CATEGORY_StringShort(ii) << "\",\n";
  }
  cout << "  \"TR_MUL\",\n"; 
  cout << "  \"TR_DIV\",\n";
  cout << "  \"TR_FMUL\",\n";
  cout << "  \"TR_FDIV\",\n";
  cout << "  \"TR_NOP\",\n";
  cout << "  \"PREFETCH_NTA\",\n";
  cout << "  \"PREFETCH_T0\",\n";
  cout << "  \"PREFETCH_T1\",\n";
  cout << "  \"PREFETCH_T2\",\n";
  cout << "  \"GPU_EN\",\n";
  cout << "  \"CPU_OPCODE_LAST\",\n";
  cout << "};\n";
}


void macsim_reg(void)
{
  cout << "const char* trace_read_c::g_tr_reg_names[MAX_TR_REG] = {\n";
  for (int ii = 0; ii < REG_PIN_BASE; ++ii) {
    cout << "   \"" << REG_StringShort(static_cast<LEVEL_BASE::REG>(ii)) << "\",\n";
  }
  cout << "};\n";
}


void macsim_pin_convert(void)
{
  for (int ii = 0; ii < XED_CATEGORY_LAST; ++ii) {
    cout << "  m_int_uop_table[XED_CATEGORY_" << CATEGORY_StringShort(ii) << "]";
    for (unsigned int jj = 0; jj < 12 - CATEGORY_StringShort(ii).size(); ++jj) {
      cout << " ";
    }
    cout << "= UOP_;\n";
  }
  cout << "  m_int_uop_table[TR_MUL]                   = UOP_IMUL;\n"; 
  cout << "  m_int_uop_table[TR_DIV]                   = UOP_IMUL;\n";
  cout << "  m_int_uop_table[TR_FMUL]                  = UOP_FMUL;\n";
  cout << "  m_int_uop_table[TR_FDIV]                  = UOP_FDIV;\n";
  cout << "  m_int_uop_table[TR_NOP]                   = UOP_NOP;\n";
  cout << "  m_int_uop_table[PREFETCH_NTA]             = UOP_IADD;\n";
  cout << "  m_int_uop_table[PREFETCH_T0]              = UOP_IADD;\n";
  cout << "  m_int_uop_table[PREFETCH_T1]              = UOP_IADD;\n";
  cout << "  m_int_uop_table[PREFETCH_T2]              = UOP_IADD;\n";
  cout << "  m_int_uop_table[TR_MEM_LD_LM]             = UOP_IADD;\n";
  cout << "  m_int_uop_table[TR_MEM_LD_SM]             = UOP_IADD;\n";
  cout << "  m_int_uop_table[TR_MEM_LD_GM]             = UOP_IADD;\n";
  cout << "  m_int_uop_table[TR_MEM_ST_LM]             = UOP_IADD;\n";
  cout << "  m_int_uop_table[TR_MEM_ST_SM]             = UOP_IADD;\n";
  cout << "  m_int_uop_table[TR_MEM_ST_GM]             = UOP_IADD;\n";
  cout << "  m_int_uop_table[TR_DATA_XFER_LM]          = UOP_IADD;\n";
  cout << "  m_int_uop_table[TR_DATA_XFER_SM]          = UOP_IADD;\n";
  cout << "  m_int_uop_table[TR_DATA_XFER_GM]          = UOP_IADD;\n";
  cout << "  m_int_uop_table[TR_MEM_LD_CM]             = UOP_IADD;\n";
  cout << "  m_int_uop_table[TR_MEM_LD_TM]             = UOP_IADD;\n";
  cout << "  m_int_uop_table[TR_MEM_LD_PM]             = UOP_IADD;\n";

  cout << "\n";

  
  for (int ii = 0; ii < XED_CATEGORY_LAST; ++ii) {
    cout << "  m_fp_uop_table[XED_CATEGORY_" << CATEGORY_StringShort(ii) << "]";
    for (unsigned int jj = 0; jj < 12 - CATEGORY_StringShort(ii).size(); ++jj) {
      cout << " ";
    }
    cout << "= UOP_;\n";
  }
  cout << "  m_fp_uop_table[TR_MUL]                   = UOP_IMUL;\n"; 
  cout << "  m_fp_uop_table[TR_DIV]                   = UOP_IDIV;\n";
  cout << "  m_fp_uop_table[TR_FMUL]                  = UOP_FMUL;\n";
  cout << "  m_fp_uop_table[TR_FDIV]                  = UOP_FDIV;\n";
  cout << "  m_fp_uop_table[TR_NOP]                   = UOP_NOP;\n";
  cout << "  m_fp_uop_table[PREFETCH_NTA]             = UOP_FADD;\n";
  cout << "  m_fp_uop_table[PREFETCH_T0]              = UOP_FADD;\n";
  cout << "  m_fp_uop_table[PREFETCH_T1]              = UOP_FADD;\n";
  cout << "  m_fp_uop_table[PREFETCH_T2]              = UOP_FADD;\n";
  cout << "  m_fp_uop_table[TR_MEM_LD_LM]             = UOP_FADD;\n";
  cout << "  m_fp_uop_table[TR_MEM_LD_SM]             = UOP_FADD;\n";
  cout << "  m_fp_uop_table[TR_MEM_LD_GM]             = UOP_FADD;\n";
  cout << "  m_fp_uop_table[TR_MEM_ST_LM]             = UOP_FADD;\n";
  cout << "  m_fp_uop_table[TR_MEM_ST_SM]             = UOP_FADD;\n";
  cout << "  m_fp_uop_table[TR_MEM_ST_GM]             = UOP_FADD;\n";
  cout << "  m_fp_uop_table[TR_DATA_XFER_LM]          = UOP_FADD;\n";
  cout << "  m_fp_uop_table[TR_DATA_XFER_SM]          = UOP_FADD;\n";
  cout << "  m_fp_uop_table[TR_DATA_XFER_GM]          = UOP_FADD;\n";
  cout << "  m_fp_uop_table[TR_MEM_LD_CM]             = UOP_FADD;\n";
  cout << "  m_fp_uop_table[TR_MEM_LD_TM]             = UOP_FADD;\n";
  cout << "  m_fp_uop_table[TR_MEM_LD_PM]             = UOP_FADD;\n";
}


// ====================================
// Initialize the simulation
// ====================================
void initialize_sim(void)
{
//  extract_register();
//  extract_category();
//  extract_opcode(); // useless
//  macsim_opcode();
//  macsim_opcode_enum();
  macsim_reg();
//  macsim_pin_convert();
}


void finalize_sim(void)
{
}


// ====================================
// BBL instrumentation : inst. count
// ====================================
VOID PIN_FAST_ANALYSIS_CALL INST_count(UINT32 count)
{
}


// ====================================
// Trace instrumentation routine for faster instruction counting
// ====================================
VOID INST_trace(TRACE trace, VOID *v)
{
#if 0
  for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
    BBL_InsertCall(bbl, IPOINT_ANYWHERE, AFUNPTR(INST_count), IARG_FAST_ANALYSIS_CALL,
        IARG_UINT32, BBL_NumIns(bbl), IARG_END);
  }
#endif
}


// ====================================
// Instruction instrumentation routine
// ====================================
VOID INST_instruction(INS ins, VOID *v)
{
}


VOID INST_fini(INT32 code, VOID *v)
{
}


VOID INST_tstart(THREADID tid, CONTEXT* ctxt, INT32 flags, VOID* v)
{
}


VOID INST_tfini(THREADID tid, const CONTEXT* ctxt, INT32 code, VOID* v)
{
}


int main(int argc, char* argv[])
{
  if (PIN_Init(argc, argv))
    return usage();

  initialize_sim();

  PIN_AddThreadStartFunction(INST_tstart, 0);
  PIN_AddThreadFiniFunction(INST_tfini, 0);
  TRACE_AddInstrumentFunction(INST_trace, 0);
  INS_AddInstrumentFunction(INST_instruction, 0);
  PIN_AddFiniFunction(INST_fini, 0);
  PIN_StartProgram();

  return 0;
}
