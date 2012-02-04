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
 * File         : trace_read.h 
 * Author       : Hyesoon Kim
 * Date         : 
 * SVN          : $Id: dram.h 912 2009-11-20 19:09:21Z kacear $
 * Description  : Trace handling class
 *********************************************************************************************/


#ifndef TRACE_H_INCLUDED
#define TRACE_H_INCLUDED


#include "uop.h"
#include "inst_info.h"


///////////////////////////////////////////////////////////////////////////////////////////////


#define MAX_TR_REG 300 // REG_LAST = 298
#define MAX_TR_OPCODE_NAME 70
#define REP_MOV_MEM_SIZE_MAX 4
#define REP_MOV_MEM_SIZE_MAX_NEW MAX2(REP_MOV_MEM_SIZE_MAX, (*m_simBase->m_knobs->KNOB_MEM_SIZE_AMP*4))
#define MAX_SRC_NUM 9
#define MAX_DST_NUM 6
#define TRACE_SIZE (sizeof(trace_info_s) - sizeof(uint32_t))


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Trace information structure
///
/// Note that actual raw trace format does not have m_instruction_next_addr field. However,
/// instead of keeping two separate structures, just using TRACE_SIZE, which is defined above,
/// read/write required size only.
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct trace_info_s {
  uint8_t  m_num_read_regs;     /**< num read registers */
  uint8_t  m_num_dest_regs;     /**< num dest registers */
  uint8_t  m_src[MAX_SRC_NUM];  /**< src register id */
  uint8_t  m_dst[MAX_DST_NUM];  /**< dest register id */
  uint8_t  m_cf_type;           /**< branch type */
  bool     m_has_immediate;     /**< has immediate field */
  uint8_t  m_opcode;            /**< opcode */
  bool     m_has_st;            /**< has store operation */ 
  bool     m_is_fp;             /**< fp operation */
  bool     m_write_flg;         /**< write flag */
  uint8_t  m_num_ld;            /**< number of load operations */
  uint8_t  m_size;              /**< instruction size */
  // dynamic information
  uint32_t m_ld_vaddr1;         /**< load address 1 */
  uint32_t m_ld_vaddr2;         /**< load address 2 */
  uint32_t m_st_vaddr;          /**< store address */
  uint32_t m_instruction_addr;  /**< pc address */
  uint32_t m_branch_target;     /**< branch target address */
  uint8_t  m_mem_read_size;     /**< memory read size */
  uint8_t  m_mem_write_size;    /**< memory write size */
  bool     m_rep_dir;           /**< repetition direction */
  bool     m_actually_taken;    /**< branch actually taken */
  uint32_t m_instruction_next_addr; /**< next pc address, not in raw trace format */
} trace_info_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief structure to hold decoded uop information
///
/// an instruction from the trace can be decoded into one or more uops; 
/// an instance of a trace_uop_s represents one such uop
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct trace_uop_s {
  /**
   * Constructor
   */
  trace_uop_s();

  uint8_t      m_opcode;        /**< opcode */
  Uop_Type     m_op_type;       /**< type of operation */
  Mem_Type     m_mem_type;      /**< type of memory instruction */
  Cf_Type      m_cf_type;       /**< type of control flow instruction */ 
  Bar_Type     m_bar_type;      /**< type of barrier caused by instruction */
  uns          m_num_dest_regs; /**< number of destination registers written */
  uns          m_num_src_regs;  /**< number of source registers read */
  uns          m_mem_size;      /**< number of bytes read/written by a memory instruction */
  uns          m_inst_size;     /**< instruction size */
  Addr         m_addr;          /**< pc address */ 
  reg_info_s   m_srcs[MAX_SRCS]; /**< source register information */
  reg_info_s   m_dests[MAX_DESTS]; /**< destination register information */
  Addr         m_va;            /**< virtual address */
  bool         m_actual_taken;  /**< branch actually taken */
  Addr         m_target;        /**< branch target address */
  Addr         m_npc;           /**< next pc address */ 
  bool         m_pin_2nd_mem;   /**< has second memory operation */
  inst_info_s *m_info;          /**< pointer to the instruction hash table */ 
  int          m_rep_uop_num;   /**< repeated uop number */
  bool         m_eom;           /**< end of macro */
  bool         m_alu_uop;       /**< alu uop */ 
  // GPU simulation
  uint32_t     m_active_mask;   /**< active mask */
  uint32_t     m_taken_mask;    /**< branch taken mask */
  Addr         m_reconverge_addr; /**< address of reconvergence */
  bool         m_mul_mem_uops;  /**< multiple memory transactions */
} trace_uop_s; 


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Enumerator ID for temp register
///
/// Currently, we have 166 registers, so temp register will be 167.
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum TR_TEMP_TEMP_ENUM_ {
  TR_REG_TMP0 = 167
} TR_TEMP_TEMP_ENUM;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief enumerator for supported opcodes
///
/// IMPORTANT : please modify inst.stat.def if you modify this enumerator
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum TR_OPCODE_ENUM_ {
  XED_CATEGORY_INVALID,
  XED_CATEGORY_3DNOW,
  XED_CATEGORY_AES,
  XED_CATEGORY_AVX,
  XED_CATEGORY_BINARY,
  XED_CATEGORY_BITBYTE,
  XED_CATEGORY_BROADCAST,
  XED_CATEGORY_CALL,
  XED_CATEGORY_CMOV,
  XED_CATEGORY_COND_BR,
  XED_CATEGORY_CONVERT,
  XED_CATEGORY_DATAXFER,
  XED_CATEGORY_DECIMAL,
  XED_CATEGORY_FCMOV,
  XED_CATEGORY_FLAGOP,
  XED_CATEGORY_INTERRUPT,
  XED_CATEGORY_IO,
  XED_CATEGORY_IOSTRINGOP,
  XED_CATEGORY_LOGICAL,
  XED_CATEGORY_MISC,
  XED_CATEGORY_MMX,
  XED_CATEGORY_NOP,
  XED_CATEGORY_PCLMULQDQ,
  XED_CATEGORY_POP,
  XED_CATEGORY_PREFETCH,
  XED_CATEGORY_PUSH,
  XED_CATEGORY_RET,
  XED_CATEGORY_ROTATE,
  XED_CATEGORY_SEGOP,
  XED_CATEGORY_SEMAPHORE,
  XED_CATEGORY_SHIFT,
  XED_CATEGORY_SSE,
  XED_CATEGORY_STRINGOP,
  XED_CATEGORY_STTNI,
  XED_CATEGORY_SYSCALL,
  XED_CATEGORY_SYSRET,
  XED_CATEGORY_SYSTEM,
  XED_CATEGORY_UNCOND_BR,
  XED_CATEGORY_VTX,
  XED_CATEGORY_WIDENOP,
  XED_CATEGORY_X87_ALU,
  XED_CATEGORY_XSAVE,
  XED_CATEGORY_XSAVEOPT,
  TR_MUL,
  TR_DIV,
  TR_FMUL,
  TR_FDIV,
  TR_NOP,
  PREFETCH_NTA,
  PREFETCH_T0,
  PREFETCH_T1,
  PREFETCH_T2,
  TR_MEM_LD_LM,
  TR_MEM_LD_SM,
  TR_MEM_LD_GM,
  TR_MEM_ST_LM,
  TR_MEM_ST_SM,
  TR_MEM_ST_GM,
  TR_DATA_XFER_LM,
  TR_DATA_XFER_SM,
  TR_DATA_XFER_GM,
  TR_MEM_LD_CM,
  TR_MEM_LD_TM,
  TR_MEM_LD_PM,
  TR_OPCODE_LAST,
} TR_OPCODE_ENUM;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief enumerator for control flow type of an instruction
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum CF_TYPE_enum {
  PIN_NOT_CF,                   /**< not a control flow instruction */
  PIN_CF_BR,                    /**< an unconditional branch */
  PIN_CF_CBR,                   /**< a conditional branch */
  PIN_CF_CALL,                  /**< a call */
  // Indirect CFs
  PIN_CF_IBR,                   /**< an indirect branch */
  PIN_CF_ICALL,                 /**< an indirect call */
  PIN_CF_ICO,                   /**< an indirect jump to co-routine */
  PIN_CF_RET,                   /**< a return */
  PIN_CF_SYS,
  PIN_CF_ICBR
} CF_TYPE;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Trace reader class
///
/// This class handles all trace related operations. Read instructions from the file,
/// decode, split to micro ops, uop setups ...
///////////////////////////////////////////////////////////////////////////////////////////////
class trace_read_c
{
  public:
    /**
     * Constructor
     */
    trace_read_c(macsim_c* simBase);

    /**
     * Destructor
     */
    ~trace_read_c();

    /**
     * Get an uop from trace
     * Called by frontend.cc
     * @param core_id - core id
     * @param uop - uop object to hold instruction information
     * @param sim_thread_id thread id
     */
    bool get_uops_from_traces(int core_id, uop_c *uop, int sim_thread_id);

    /**
     * GPU simulation : Read trace ahead to read synchronization information
     * @param trace_info - trace information
     * @see process_manager_c::sim_thread_schedule
     */
    void pre_read_trace(thread_s* trace_info);

    /**
     * This function is called once for each thread/warp when the thread/warp is started.
     * The simulator does read-ahead of the trace file to get next pc address
     * @param core_id - core id
     * @param sim_thread_id - thread id
     * @see frontend_c::process_ifetch
     */
    void setup_trace(int core_id, int sim_thread_id);

    static const char *g_tr_reg_names[MAX_TR_REG]; /**< register name string */
    static const char* g_tr_opcode_names[MAX_TR_OPCODE_NAME]; /**< opcode name string */
    static const char* g_tr_cf_names[10]; /**< cf type string */
    static const char *g_optype_names[37]; /**< opcode type string */
    static const char *g_mem_type_names[20]; /**< memeory request type string */

  private:
      /**
       * Initialize the mapping between trace opcode and uop type
       */
    void init_pin_convert();

    /**
     * Function to decode an instruction from the trace file into a sequence of uops
     * @param pi - raw trace format
     * @param trace_uop - micro uops storage for this instruction
     * @param core_id - core id
     * @param sim_thread_id - thread id  
     */
    inst_info_s* convert_pinuop_to_t_uop(trace_info_s *pi, trace_uop_s **trace_uop, 
        int core_id, int sim_thread_id);

    /**
     * Convert MacSim trace to instruction information (hash table)
     * @param t_uop - MacSim trace
     * @param info - instruction information in hash table
     */
    void convert_t_uop_to_info(trace_uop_s *t_uop, inst_info_s *info);

    /**
     * Convert instruction information (from hash table) to trace uop
     * @param info - instruction information from the hash table
     * @param trace_uop - MacSim trace format
     */
    void convert_info_uop(inst_info_s *info, trace_uop_s *trace_uop);

    /**
     * From statis instruction, add dynamic information such as load address, branch target, ...
     * @param info - instruction information from the hash table
     * @param pi - raw trace information
     * @param trace_uop - MacSim uop type
     * @param rep_offset - repetition offet
     * @param core_id - core id
     */
    void convert_dyn_uop(inst_info_s *info, trace_info_s *pi, trace_uop_s *trace_uop, 
        Addr rep_offset, int core_id);

    /**
     * Dump out instruction information to the file. At most 50000 instructions will be printed
     * @param t_info - trace information
     * @param core_id - core id
     * @param thread_id - thread id
     */
    void dprint_inst(trace_info_s *t_info, int core_id, int thread_id);

    /**
     * @param core_id - core id
     * @param trace_info - trace information
     * @param sim_thread_id - thread id
     * @param inst_read - set true if instruction read successful
     * @see get_uops_from_traces
     */
    bool read_trace(int core_id, trace_info_s *trace_info, int sim_thread_id, bool *inst_read);

    /**
     * After peeking trace, in case of failture, we need to rewind trace file.
     * @param core_id - core id
     * @param sim_thread_id - thread id
     * @param num_inst - number of instructions to rewind
     * @see peek_trace
     */
    bool ungetch_trace(int core_id, int sim_thread_id, int num_inst);

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
    bool peek_trace(int core_id, trace_info_s *trace_info, int sim_thread_id, bool *inst_read);

  private:
    static const int k_trace_buffer_size = 1000; /**< maximum buffer size */
    int m_int_uop_table[TR_OPCODE_LAST]; /**< opcode to uop type mapping table (int) */
    int m_fp_uop_table[TR_OPCODE_LAST]; /**< opcode to uop type mapping tabpe (fp) */

    uint32_t  dprint_count; /**< dumped instruction count */
    ofstream* dprint_output; /**< dump output file stream */

    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */

};

#endif // TRACE_READ_H_INCLUDED
