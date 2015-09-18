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
 * Author       : HPArch Research Group
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
//#define MAX_TR_OPCODE_NAME GPU_OPCODE_LAST
#define REP_MOV_MEM_SIZE_MAX 4
#define REP_MOV_MEM_SIZE_MAX_NEW MAX2(REP_MOV_MEM_SIZE_MAX, (*KNOB(KNOB_MEM_SIZE_AMP)*4))
#define MAX_SRC_NUM 9
#define MAX_DST_NUM 6
#define MAX_GPU_SRC_NUM 5
#define MAX_GPU_DST_NUM 4
#define CPU_TRACE_SIZE (sizeof(trace_info_cpu_s) - sizeof(uint64_t))
#define GPU_TRACE_SIZE (sizeof(trace_info_gpu_small_s))
#define MAX_TR_OPCODE 452 // ARM_INS_ENDING


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Trace information structure
///
/// Note that actual raw trace format does not have m_instruction_next_addr field. However,
/// instead of keeping two separate structures, just using TRACE_SIZE, which is defined above,
/// read/write required size only.
///////////////////////////////////////////////////////////////////////////////////////////////

typedef struct trace_info_s {
  trace_info_s();
  virtual ~trace_info_s();
} trace_info_s;

typedef struct trace_info_cpu_s {
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
  uint64_t m_ld_vaddr1;         /**< load address 1 */
  uint64_t m_ld_vaddr2;         /**< load address 2 */
  uint64_t m_st_vaddr;          /**< store address */
  uint64_t m_instruction_addr;  /**< pc address */
  uint64_t m_branch_target;     /**< branch target address */
  uint8_t  m_mem_read_size;     /**< memory read size */
  uint8_t  m_mem_write_size;    /**< memory write size */
  bool     m_rep_dir;           /**< repetition direction */
  bool     m_actually_taken;    /**< branch actually taken */
  uint64_t m_instruction_next_addr; /**< next pc address, not in raw trace format */
} trace_info_cpu_s;

typedef struct trace_info_a64_s {
  uint8_t  m_num_read_regs;     /**< num read registers */
  uint8_t  m_num_dest_regs;     /**< num dest registers */
  uint8_t  m_src[MAX_SRC_NUM];  /**< src register id */
  uint8_t  m_dst[MAX_DST_NUM];  /**< dest register id */
  uint8_t  m_cf_type;           /**< branch type */
  bool     m_has_immediate;     /**< has immediate field */
  uint16_t m_opcode;            /**< opcode */
  bool     m_has_st;            /**< has store operation */
  bool     m_is_fp;             /**< fp operation */
  bool     m_write_flg;         /**< write flag */
  uint8_t  m_num_ld;            /**< number of load operations */
  uint8_t  m_size;              /**< instruction size */
  // dynamic information
  uint64_t m_ld_vaddr1;         /**< load address 1 */
  uint64_t m_ld_vaddr2;         /**< load address 2 */
  uint64_t m_st_vaddr;          /**< store address */
  uint64_t m_instruction_addr;  /**< pc address */
  uint64_t m_branch_target;     /**< branch target address */
  uint8_t  m_mem_read_size;     /**< memory read size */
  uint8_t  m_mem_write_size;    /**< memory write size */
  bool     m_rep_dir;           /**< repetition direction */
  bool     m_actually_taken;    /**< branch actually taken */
  uint64_t m_instruction_next_addr; /**< next pc address, not in raw trace format */
} trace_info_a64_s;

// identical to structure in trace generator
typedef struct trace_info_gpu_small_s {
  uint8_t m_opcode;
  bool m_is_fp;
  bool m_is_load;
  uint8_t m_cf_type;
  uint8_t m_num_read_regs;
  uint8_t m_num_dest_regs;
  uint16_t m_src[MAX_GPU_SRC_NUM];
  uint16_t m_dst[MAX_GPU_DST_NUM];
  uint8_t m_size;

  uint32_t m_active_mask;
  uint32_t m_br_taken_mask;
  uint64_t m_inst_addr;
  uint64_t m_br_target_addr;
  union {
    uint64_t m_reconv_inst_addr;
    uint64_t m_mem_addr;
  };
  union {
    uint8_t m_mem_access_size;
    uint8_t m_barrier_id;
  };
  uint16_t m_num_barrier_threads;
  union {
    uint8_t m_addr_space; //for loads, stores, atomic, prefetch(?)
    uint8_t m_level; //for membar
  };
  uint8_t m_cache_level; //for prefetch?
  uint8_t m_cache_operator; //for loads, stores, atomic, prefetch(?)
} trace_info_gpu_small_s;


// trace_info_gpu_small_s + next_inst_addr
typedef struct trace_info_gpu_s {
  uint8_t m_opcode;
  bool m_is_fp;
  bool m_is_load;
  uint8_t m_cf_type;
  uint8_t m_num_read_regs;
  uint8_t m_num_dest_regs;
  uint16_t m_src[MAX_GPU_SRC_NUM];
  uint16_t m_dst[MAX_GPU_DST_NUM];
  uint8_t m_size;

  uint32_t m_active_mask;
  uint32_t m_br_taken_mask;
  uint64_t m_inst_addr;
  uint64_t m_br_target_addr;
  union {
    uint64_t m_reconv_inst_addr;
    uint64_t m_mem_addr;
  };
  union {
    uint8_t m_mem_access_size;
    uint8_t m_barrier_id;
  };
  uint16_t m_num_barrier_threads;
  union {
    uint8_t m_addr_space; //for loads, stores, atomic, prefetch(?)
    uint8_t m_level; //for membar
  };
  uint8_t m_cache_level; //for prefetch?
  uint8_t m_cache_operator; //for loads, stores, atomic, prefetch(?)
  uint64_t m_next_inst_addr; // next pc address, not present in raw trace format
} trace_info_gpu_s;


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

  uint16_t     m_opcode;        /**< opcode */
  Uop_Type     m_op_type;       /**< type of operation */
  Mem_Type     m_mem_type;      /**< type of memory instruction */
  Cf_Type      m_cf_type;       /**< type of control flow instruction */ 
  Bar_Type     m_bar_type;      /**< type of barrier caused by instruction */
  int          m_num_dest_regs; /**< number of destination registers written */
  int          m_num_src_regs;  /**< number of source registers read */
  int          m_mem_size;      /**< number of bytes read/written by a memory instruction */
  int          m_inst_size;     /**< instruction size */
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

  // HMC simulation
  // changed by Lifeng
  HMC_Type m_hmc_inst;  /**<  hmc type of cur uop */
} trace_uop_s; 


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Enumerator ID for temp register
///
/// Currently, we have 166 registers, so temp register will be 167.
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum TR_TEMP_TEMP_ENUM_ {
  TR_REG_TMP0 = 167
} TR_TEMP_TEMP_ENUM;


typedef enum CPU_OPCODE_ENUM_ {
  XED_CATEGORY_INVALID,
  XED_CATEGORY_3DNOW,
  XED_CATEGORY_AES,
  XED_CATEGORY_AVX,
  XED_CATEGORY_AVX2, // new
  XED_CATEGORY_AVX2GATHER, // new
  XED_CATEGORY_BDW, // new
  XED_CATEGORY_BINARY,
  XED_CATEGORY_BITBYTE,
  XED_CATEGORY_BMI1, // new
  XED_CATEGORY_BMI2, // new
  XED_CATEGORY_BROADCAST,
  XED_CATEGORY_CALL,
  XED_CATEGORY_CMOV,
  XED_CATEGORY_COND_BR,
  XED_CATEGORY_CONVERT,
  XED_CATEGORY_DATAXFER,
  XED_CATEGORY_DECIMAL,
  XED_CATEGORY_FCMOV,
  XED_CATEGORY_FLAGOP,
  XED_CATEGORY_FMA4, // new
  XED_CATEGORY_INTERRUPT,
  XED_CATEGORY_IO,
  XED_CATEGORY_IOSTRINGOP,
  XED_CATEGORY_LOGICAL,
  XED_CATEGORY_LZCNT, // new
  XED_CATEGORY_MISC,
  XED_CATEGORY_MMX,
  XED_CATEGORY_NOP,
  XED_CATEGORY_PCLMULQDQ,
  XED_CATEGORY_POP,
  XED_CATEGORY_PREFETCH,
  XED_CATEGORY_PUSH,
  XED_CATEGORY_RDRAND, // new
  XED_CATEGORY_RDSEED, // new
  XED_CATEGORY_RDWRFSGS, // new
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
  XED_CATEGORY_TBM, // new
  XED_CATEGORY_UNCOND_BR,
  XED_CATEGORY_VFMA, // new
  XED_CATEGORY_VTX,
  XED_CATEGORY_WIDENOP,
  XED_CATEGORY_X87_ALU,
  XED_CATEGORY_XOP,
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
  GPU_EN,
  CPU_OPCODE_LAST,
} CPU_OPCODE_ENUM;



// identical to enum in PTX trace generator
typedef enum GPU_OPCODE_ {
  GPU_INVALID = 0,
  GPU_ABS,
  GPU_ABS64,
  GPU_ADD, 
  GPU_ADD64, 
	GPU_ADDC,
	GPU_AND,
	GPU_AND64,
	GPU_ATOM_GM,
	GPU_ATOM_SM,
	GPU_ATOM64_GM,
	GPU_ATOM64_SM,
	GPU_BAR_ARRIVE,
  GPU_BAR_SYNC,
  GPU_BAR_RED,
	GPU_BFE,
	GPU_BFE64,
	GPU_BFI,
	GPU_BFI64,
	GPU_BFIND,
	GPU_BFIND64,
	GPU_BRA,
	GPU_BREV,
	GPU_BREV64,
	GPU_BRKPT,
	GPU_CALL,
	GPU_CLZ,
	GPU_CLZ64,
	GPU_CNOT,
	GPU_CNOT64,
	GPU_COPYSIGN,
	GPU_COPYSIGN64,
	GPU_COS,
	GPU_CVT,
	GPU_CVT64,
	GPU_CVTA,
	GPU_CVTA64,
	GPU_DIV,
	GPU_DIV64,
	GPU_EX2,
	GPU_EXIT,
	GPU_FMA,
	GPU_FMA64,
	GPU_ISSPACEP,
	GPU_LD,
	GPU_LD64,
	GPU_LDU,
	GPU_LDU64,
	GPU_LG2,
	GPU_MAD24,
	GPU_MAD,
	GPU_MAD64,
	GPU_MADC,
	GPU_MADC64,
	GPU_MAX,
	GPU_MAX64,
	GPU_MEMBAR_CTA,
	GPU_MEMBAR_GL,
	GPU_MEMBAR_SYS,
	GPU_MIN,
	GPU_MIN64,
	GPU_MOV,
	GPU_MOV64,
	GPU_MUL24,
	GPU_MUL,
	GPU_MUL64,
	GPU_NEG,
	GPU_NEG64,
	GPU_NOT,
	GPU_NOT64,
	GPU_OR,
	GPU_OR64,
	GPU_PMEVENT,
	GPU_POPC,
	GPU_POPC64,
	GPU_PREFETCH,
	GPU_PREFETCHU,
	GPU_PRMT,
	GPU_RCP,
	GPU_RCP64,
	GPU_RED_GM,
	GPU_RED_SM,
	GPU_RED64_GM,
	GPU_RED64_SM,
	GPU_REM,
	GPU_REM64,
	GPU_RET,
	GPU_RSQRT,
	GPU_RSQRT64,
	GPU_SAD,
	GPU_SAD64,
	GPU_SELP,
	GPU_SELP64,
	GPU_SET,
	GPU_SET64,
	GPU_SETP,
	GPU_SETP64,
	GPU_SHFL,
	GPU_SHFL64,
	GPU_SHL,
	GPU_SHL64,
	GPU_SHR,
	GPU_SHR64,
	GPU_SIN,
	GPU_SLCT,
	GPU_SLCT64,
	GPU_SQRT,
	GPU_SQRT64,
	GPU_ST,
	GPU_ST64,
	GPU_SUB,
	GPU_SUB64,
	GPU_SUBC,
	GPU_SULD,
	GPU_SULD64,
	GPU_SURED,
	GPU_SURED64,
	GPU_SUST,
	GPU_SUST64,
	GPU_SUQ,
  GPU_TESTP,
  GPU_TESTP64,
  GPU_TEX,
  GPU_TLD4,
  GPU_TXQ,
  GPU_TRAP,
  GPU_VABSDIFF,
  GPU_VADD,
  GPU_VMAD,
  GPU_VMAX,
  GPU_VMIN,
  GPU_VSET,
  GPU_VSHL,
  GPU_VSHR,
  GPU_VSUB,
  GPU_VOTE,
  GPU_XOR,
  GPU_XOR64,
  GPU_RECONVERGE,
  GPU_PHI,
  GPU_MEM_LD_GM,
  GPU_MEM_LD_LM,
  GPU_MEM_LD_SM,
  GPU_MEM_LD_CM,
  GPU_MEM_LD_TM,
  GPU_MEM_LD_PM,
  GPU_MEM_LDU_GM,
  GPU_MEM_ST_GM,
  GPU_MEM_ST_LM,
  GPU_MEM_ST_SM,
  GPU_DATA_XFER_GM,
  GPU_DATA_XFER_LM,
  GPU_DATA_XFER_SM,
  GPU_OPCODE_LAST
} GPU_OPCODE_ENUM;


typedef enum GPU_ADDRESS_SPACE_ENUM_ {
  GPU_ADDR_SP_INVALID = 0,
  GPU_ADDR_SP_CONST,
  GPU_ADDR_SP_GLOBAL,
  GPU_ADDR_SP_LOCAL,
  GPU_ADDR_SP_PARAM,
  GPU_ADDR_SP_SHARED,
  GPU_ADDR_SP_TEXTURE,
  GPU_ADDR_SP_GENERIC,
  GPU_ADDR_SP_LAST
} GPU_ADDRESS_SPACE_ENUM;


typedef enum GPU_CACHE_OP_ENUM_ {
  GPU_CACHE_OP_INVALID = 0,
  GPU_CACHE_OP_CA,
  GPU_CACHE_OP_CV,
  GPU_CACHE_OP_CG,
  GPU_CACHE_OP_CS,
  GPU_CACHE_OP_WB,
  GPU_CACHE_OP_WT,
  GPU_CACHE_OP_LAST
} GPU_CACHE_OP_ENUM;


typedef enum GPU_CACHE_LEVEL_ENUM_ {
  GPU_CACHE_INVALID = 0,
  GPU_CACHE_L1,
  GPU_CACHE_L2,
  GPU_CACHE_LAST
} GPU_CACHE_LEVEL_ENUM;

typedef enum GPU_FENCE_LEVEL_ENUM_ {
  GPU_FENCE_INVALID = 0,
  GPU_FENCE_CTA,
  GPU_FENCE_GL,
  GPU_FENCE_SYS,
  GPU_FENCE_LAST
} GPU_FENCE_LEVEL_ENUM;

// in trace generator, special registers are assigned values starting from 200
// matches order in ocelot/ir/interface/PTXOperand.h
typedef enum GPU_SPECIAL_REGISTER_ENUM_ {
  GPU_SP_REG_INVALID = 0,
  GPU_SP_REG_TID = 200,
  GPU_SP_REG_NTID,
  GPU_SP_REG_LANEID,
  GPU_SP_REG_WARPID,
  GPU_SP_REG_NWARPID,
  GPU_SP_REG_WARPSIZE,
  GPU_SP_REG_CTAID,
  GPU_SP_REG_NCTAID,
  GPU_SP_REG_SMID,
  GPU_SP_REG_NSMID,
  GPU_SP_REG_GRIDID,
  GPU_SP_REG_CLOCK,
  GPU_SP_REG_CLOCK64,
  GPU_SP_REG_LANEMASK_EQ,
  GPU_SP_REG_LANEMASK_LE,
  GPU_SP_REG_LANEMASK_LT,
  GPU_SP_REG_LANEMASK_GE,
  GPU_SP_REG_LANEMASK_GT,
  GPU_SP_REG_PM0,
  GPU_SP_REG_PM1,
  GPU_SP_REG_PM2,
  GPU_SP_REG_PM3,
  GPU_SP_REG_ENVREG0,
  GPU_SP_REG_ENVREG1,
  GPU_SP_REG_ENVREG2,
  GPU_SP_REG_ENVREG3,
  GPU_SP_REG_ENVREG4,
  GPU_SP_REG_ENVREG5,
  GPU_SP_REG_ENVREG6,
  GPU_SP_REG_ENVREG7,
  GPU_SP_REG_ENVREG8,
  GPU_SP_REG_ENVREG9,
  GPU_SP_REG_ENVREG10,
  GPU_SP_REG_ENVREG11,
  GPU_SP_REG_ENVREG12,
  GPU_SP_REG_ENVREG13,
  GPU_SP_REG_ENVREG14,
  GPU_SP_REG_ENVREG15,
  GPU_SP_REG_ENVREG16,
  GPU_SP_REG_ENVREG17,
  GPU_SP_REG_ENVREG18,
  GPU_SP_REG_ENVREG19,
  GPU_SP_REG_ENVREG20,
  GPU_SP_REG_ENVREG21,
  GPU_SP_REG_ENVREG22,
  GPU_SP_REG_ENVREG23,
  GPU_SP_REG_ENVREG24,
  GPU_SP_REG_ENVREG25,
  GPU_SP_REG_ENVREG26,
  GPU_SP_REG_ENVREG27,
  GPU_SP_REG_ENVREG28,
  GPU_SP_REG_ENVREG29,
  GPU_SP_REG_ENVREG30,
  GPU_SP_REG_ENVREG31,
  SPECIALREGISTER_INVALID
} GPU_SPECIAL_REGISTER_ENUM;



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
    trace_read_c(macsim_c* simBase, ofstream* dprint_output);

    /**
     * Destructor
     */
    virtual ~trace_read_c();

    /**
     * Get an uop from trace
     * Called by frontend.cc
     * @param core_id - core id
     * @param uop - uop object to hold instruction information
     * @param sim_thread_id thread id
     */
    virtual bool get_uops_from_traces(int core_id, uop_c *uop, int sim_thread_id) = 0;

    /**
     * GPU simulation : Read trace ahead to read synchronization information
     * @param trace_info - trace information
     * @see process_manager_c::sim_thread_schedule
     */
    virtual void pre_read_trace(thread_s* trace_info) = 0;

    /**
     * This function is called once for each thread/warp when the thread/warp is started.
     * The simulator does read-ahead of the trace file to get next pc address
     * @param core_id - core id
     * @param sim_thread_id - thread id
     * @see frontend_c::process_ifetch
     */
    void setup_trace(int core_id, int sim_thread_id);

      /**
       * Initialize the mapping between trace opcode and uop type
       */
    virtual void init_pin_convert() = 0;

    static const char *g_tr_reg_names[MAX_TR_REG]; /**< register name string */
    static const char* g_tr_opcode_names[MAX_TR_OPCODE_NAME]; /**< opcode name string */
    static const char* g_tr_cf_names[10]; /**< cf type string */
    static const char *g_optype_names[37]; /**< opcode type string */
    static const char *g_mem_type_names[20]; /**< memeory request type string */

  protected:
    /**
     * Function to decode an instruction from the trace file into a sequence of uops
     * @param pi - raw trace format
     * @param trace_uop - micro uops storage for this instruction
     * @param core_id - core id
     * @param sim_thread_id - thread id  
     */
    virtual inst_info_s* convert_pinuop_to_t_uop(void *pi, trace_uop_s **trace_uop, 
        int core_id, int sim_thread_id) = 0;

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
    virtual void convert_dyn_uop(inst_info_s *info, void *pi, trace_uop_s *trace_uop, 
        Addr rep_offset, int core_id) = 0;

    /**
     * Dump out instruction information to the file. At most 50000 instructions will be printed
     * @param t_info - trace information
     * @param core_id - core id
     * @param thread_id - thread id
     */
    virtual void dprint_inst(void  *t_info, int core_id, int thread_id) = 0;

    /**
     * @param core_id - core id
     * @param trace_info - trace information
     * @param sim_thread_id - thread id
     * @param inst_read - set true if instruction read successful
     * @see get_uops_from_traces
     */
    bool read_trace(int core_id, void *trace_info, int sim_thread_id, bool *inst_read);

    /**
     * After peeking trace, in case of failture, we need to rewind trace file.
     * @param core_id - core id
     * @param sim_thread_id - thread id
     * @param num_inst - number of instructions to rewind
     * @see peek_trace
     */
    virtual bool ungetch_trace(int core_id, int sim_thread_id, int num_inst) = 0;

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
    virtual bool peek_trace(int core_id, void *trace_info, int sim_thread_id, bool *inst_read) = 0;

  protected:
    static const int k_trace_buffer_size = 1000; /**< maximum buffer size */
    int m_int_uop_table[MAX_TR_OPCODE]; /**< opcode to uop type mapping table (int) */
    int m_fp_uop_table[MAX_TR_OPCODE]; /**< opcode to uop type mapping tabpe (fp) */

    uint32_t  m_dprint_count; /**< dumped instruction count */
    ofstream* m_dprint_output; /**< dump output file stream */

    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */
    int m_trace_size;
};


class trace_reader_wrapper_c {
  public:
    trace_reader_wrapper_c(macsim_c* simBase);
    ~trace_reader_wrapper_c();
    
    void setup_trace(int core_id, int sim_thread_id, bool gpu_sim);
    bool get_uops_from_traces(int core_id, uop_c *uop, int sim_thread_id, bool gpu_sim);
    void pre_read_trace(thread_s* trace_info);

  private:
    trace_reader_wrapper_c();

  private:
    macsim_c* m_simBase;
    trace_read_c* m_cpu_decoder;
    trace_read_c* m_gpu_decoder;
    
    ofstream* m_dprint_output; /**< dump output file stream */

};






#endif // TRACE_READ_H_INCLUDED
