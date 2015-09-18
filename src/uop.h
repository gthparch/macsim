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
 * File         : uop.h
 * Author       : Hyesoon Kim
 * Date         : 12/16/2007
 * SVN          : $Id: uop.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : uop structure
 *********************************************************************************************/

#ifndef UOP_H_INCLUDED
#define UOP_H_INCLUDED


#include <string>

#include "global_types.h"
#include "global_defs.h"

#include "hmc_types.h"


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief allocation queue types
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum Iaq_Type_enum{
  gen_ALLOCQ = 0,
  mem_ALLOCQ,
  fp_ALLOCQ,
  max_ALLOCQ
} ALLOCQ_Type;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief uop types
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum Uop_Type_enum {
  UOP_INV,                      //!< invalid opcode
  UOP_SPEC,                     //!< something weird (rpcc)

  //UOP_SSMT0,   // something added for ssmt uarch interaction
  //UOP_SSMT1,   // something added for ssmt uarch interaction
  //UOP_SSMT2,   // something added for ssmt uarch interaction
  //UOP_SSMT3,   // something added for ssmt uarch interaction

  UOP_NOP,                      //!< is a decoded nop

  // these instructions use all integer regs
  UOP_CF,                       //!< change of flow
  UOP_CMOV,                     //!< conditional move
  UOP_LDA,                      //!< load address
  UOP_IMEM,                     //!< int memory instruction
  UOP_IADD,                     //!< integer add
  UOP_IMUL,                     //!< integer multiply
  UOP_ICMP,                     //!< integer compare
  UOP_LOGIC,                    //!< logical
  UOP_SHIFT,                    //!< shift
  UOP_BYTE,                     //!< byte manipulation
  UOP_MM,                       //!< multimedia instructions

  // fence instruction
  UOP_LFENCE,                   //!< load fence, TODO
  UOP_FULL_FENCE,               //!< Full memory fence
  UOP_ACQ_FENCE,                //!< acquire fence
  UOP_REL_FENCE,                //!< release fence

  // fmem reads one int reg and writes a fp reg
  UOP_FMEM,                     //!< fp memory instruction

  // everything below here is floating point regs only
  UOP_FCF,
  UOP_FCVT,                     //!< floating point convert
  UOP_FADD,                     //!< floating point add
  UOP_FMUL,                     //!< floating point multiply
  UOP_FDIV,                     //!< floating point divide
  UOP_FCMP,                     //!< floating point compare
  UOP_FBIT,                     //!< floating point bit
  UOP_FCMOV,                    //!< floating point cond move

  UOP_LD,                       //!< load memory instruction
  UOP_ST,                       //!< store memory instruction

  // MMX instructions
  UOP_SSE,

  // added on may-10-2012 for GPU instructions
  UOP_GPU_ABS,
  UOP_GPU_ABS64,
  UOP_GPU_ADD, 
  UOP_GPU_ADD64, 
	UOP_GPU_ADDC,
	UOP_GPU_AND,
	UOP_GPU_AND64,
	UOP_GPU_ATOM_GM,
	UOP_GPU_ATOM_SM,
	UOP_GPU_ATOM64_GM,
	UOP_GPU_ATOM64_SM,
	UOP_GPU_BAR_ARRIVE,
	UOP_GPU_BAR_SYNC,
	UOP_GPU_BAR_RED,
	UOP_GPU_BFE,
	UOP_GPU_BFE64,
	UOP_GPU_BFI,
	UOP_GPU_BFI64,
	UOP_GPU_BFIND,
	UOP_GPU_BFIND64,
	UOP_GPU_BRA,
	UOP_GPU_BREV,
	UOP_GPU_BREV64,
	UOP_GPU_BRKPT,
	UOP_GPU_CALL,
	UOP_GPU_CLZ,
	UOP_GPU_CLZ64,
	UOP_GPU_CNOT,
	UOP_GPU_CNOT64,
	UOP_GPU_COPYSIGN,
	UOP_GPU_COPYSIGN64,
	UOP_GPU_COS,
	UOP_GPU_CVT,
	UOP_GPU_CVT64,
	UOP_GPU_CVTA,
	UOP_GPU_CVTA64,
	UOP_GPU_DIV,
	UOP_GPU_DIV64,
	UOP_GPU_EX2,
	UOP_GPU_EXIT,
	UOP_GPU_FMA,
	UOP_GPU_FMA64,
	UOP_GPU_ISSPACEP,
	UOP_GPU_LD,
	UOP_GPU_LD64,
	UOP_GPU_LDU,
	UOP_GPU_LDU64,
	UOP_GPU_LG2,
	UOP_GPU_MAD24,
	UOP_GPU_MAD,
	UOP_GPU_MAD64,
	UOP_GPU_MADC,
	UOP_GPU_MADC64,
	UOP_GPU_MAX,
	UOP_GPU_MAX64,
	UOP_GPU_MEMBAR_CTA,
	UOP_GPU_MEMBAR_GL,
	UOP_GPU_MEMBAR_SYS,
	UOP_GPU_MIN,
	UOP_GPU_MIN64,
	UOP_GPU_MOV,
	UOP_GPU_MOV64,
	UOP_GPU_MUL24,
	UOP_GPU_MUL,
	UOP_GPU_MUL64,
	UOP_GPU_NEG,
	UOP_GPU_NEG64,
	UOP_GPU_NOT,
	UOP_GPU_NOT64,
	UOP_GPU_OR,
	UOP_GPU_OR64,
	UOP_GPU_PMEVENT,
	UOP_GPU_POPC,
	UOP_GPU_POPC64,
	UOP_GPU_PREFETCH,
	UOP_GPU_PREFETCHU,
	UOP_GPU_PRMT,
	UOP_GPU_RCP,
	UOP_GPU_RCP64,
	UOP_GPU_RED_GM,
	UOP_GPU_RED_SM,
	UOP_GPU_RED64_GM,
	UOP_GPU_RED64_SM,
	UOP_GPU_REM,
	UOP_GPU_REM64,
	UOP_GPU_RET,
	UOP_GPU_RSQRT,
	UOP_GPU_RSQRT64,
	UOP_GPU_SAD,
	UOP_GPU_SAD64,
	UOP_GPU_SELP,
	UOP_GPU_SELP64,
	UOP_GPU_SET,
	UOP_GPU_SET64,
	UOP_GPU_SETP,
	UOP_GPU_SETP64,
	UOP_GPU_SHFL,
	UOP_GPU_SHFL64,
	UOP_GPU_SHL,
	UOP_GPU_SHL64,
	UOP_GPU_SHR,
	UOP_GPU_SHR64,
	UOP_GPU_SIN,
	UOP_GPU_SLCT,
	UOP_GPU_SLCT64,
	UOP_GPU_SQRT,
	UOP_GPU_SQRT64,
	UOP_GPU_ST,
	UOP_GPU_ST64,
	UOP_GPU_SUB,
	UOP_GPU_SUB64,
	UOP_GPU_SUBC,
	UOP_GPU_SULD,
	UOP_GPU_SULD64,
	UOP_GPU_SURED,
	UOP_GPU_SURED64,
	UOP_GPU_SUST,
	UOP_GPU_SUST64,
	UOP_GPU_SUQ,
  UOP_GPU_TESTP,
  UOP_GPU_TESTP64,
  UOP_GPU_TEX,
  UOP_GPU_TLD4,
  UOP_GPU_TXQ,
  UOP_GPU_TRAP,
  UOP_GPU_VABSDIFF,
  UOP_GPU_VADD,
  UOP_GPU_VMAD,
  UOP_GPU_VMAX,
  UOP_GPU_VMIN,
  UOP_GPU_VSET,
  UOP_GPU_VSHL,
  UOP_GPU_VSHR,
  UOP_GPU_VSUB,
  UOP_GPU_VOTE,
  UOP_GPU_XOR,
  UOP_GPU_XOR64,
  UOP_GPU_RECONVERGE,
  UOP_GPU_PHI,

  UOP_GPU_FABS,
  UOP_GPU_FABS64,
  UOP_GPU_FADD, 
  UOP_GPU_FADD64, 
	UOP_GPU_FADDC,
	UOP_GPU_FAND,
	UOP_GPU_FAND64,
	UOP_GPU_FATOM_GM,
	UOP_GPU_FATOM_SM,
	UOP_GPU_FATOM64_GM,
	UOP_GPU_FATOM64_SM,
	UOP_GPU_FBAR_ARRIVE,
	UOP_GPU_FBAR_SYNC,
	UOP_GPU_FBAR_RED,
	UOP_GPU_FBFE,
	UOP_GPU_FBFE64,
	UOP_GPU_FBFI,
	UOP_GPU_FBFI64,
	UOP_GPU_FBFIND,
	UOP_GPU_FBFIND64,
	UOP_GPU_FBRA,
	UOP_GPU_FBREV,
	UOP_GPU_FBREV64,
	UOP_GPU_FBRKPT,
	UOP_GPU_FCALL,
	UOP_GPU_FCLZ,
	UOP_GPU_FCLZ64,
	UOP_GPU_FCNOT,
	UOP_GPU_FCNOT64,
	UOP_GPU_FCOPYSIGN,
	UOP_GPU_FCOPYSIGN64,
	UOP_GPU_FCOS,
	UOP_GPU_FCVT,
	UOP_GPU_FCVT64,
	UOP_GPU_FCVTA,
	UOP_GPU_FCVTA64,
	UOP_GPU_FDIV,
	UOP_GPU_FDIV64,
	UOP_GPU_FEX2,
	UOP_GPU_FEXIT,
	UOP_GPU_FFMA,
	UOP_GPU_FFMA64,
	UOP_GPU_FISSPACEP,
	UOP_GPU_FLD,
	UOP_GPU_FLD64,
	UOP_GPU_FLDU,
	UOP_GPU_FLDU64,
	UOP_GPU_FLG2,
	UOP_GPU_FMAD24,
	UOP_GPU_FMAD,
	UOP_GPU_FMAD64,
	UOP_GPU_FMADC,
	UOP_GPU_FMADC64,
	UOP_GPU_FMAX,
	UOP_GPU_FMAX64,
	UOP_GPU_FMEMBAR_CTA,
	UOP_GPU_FMEMBAR_GL,
	UOP_GPU_FMEMBAR_SYS,
	UOP_GPU_FMIN,
	UOP_GPU_FMIN64,
	UOP_GPU_FMOV,
	UOP_GPU_FMOV64,
	UOP_GPU_FMUL24,
	UOP_GPU_FMUL,
	UOP_GPU_FMUL64,
	UOP_GPU_FNEG,
	UOP_GPU_FNEG64,
	UOP_GPU_FNOT,
	UOP_GPU_FNOT64,
	UOP_GPU_FOR,
	UOP_GPU_FOR64,
	UOP_GPU_FPMEVENT,
	UOP_GPU_FPOPC,
	UOP_GPU_FPOPC64,
	UOP_GPU_FPREFETCH,
	UOP_GPU_FPREFETCHU,
	UOP_GPU_FPRMT,
	UOP_GPU_FRCP,
	UOP_GPU_FRCP64,
	UOP_GPU_FRED_GM,
	UOP_GPU_FRED_SM,
	UOP_GPU_FRED64_GM,
	UOP_GPU_FRED64_SM,
	UOP_GPU_FREM,
	UOP_GPU_FREM64,
	UOP_GPU_FRET,
	UOP_GPU_FRSQRT,
	UOP_GPU_FRSQRT64,
	UOP_GPU_FSAD,
	UOP_GPU_FSAD64,
	UOP_GPU_FSELP,
	UOP_GPU_FSELP64,
	UOP_GPU_FSET,
	UOP_GPU_FSET64,
	UOP_GPU_FSETP,
	UOP_GPU_FSETP64,
	UOP_GPU_FSHFL,
	UOP_GPU_FSHFL64,
	UOP_GPU_FSHL,
	UOP_GPU_FSHL64,
	UOP_GPU_FSHR,
	UOP_GPU_FSHR64,
	UOP_GPU_FSIN,
	UOP_GPU_FSLCT,
	UOP_GPU_FSLCT64,
	UOP_GPU_FSQRT,
	UOP_GPU_FSQRT64,
	UOP_GPU_FST,
	UOP_GPU_FST64,
	UOP_GPU_FSUB,
	UOP_GPU_FSUB64,
	UOP_GPU_FSUBC,
	UOP_GPU_FSULD,
	UOP_GPU_FSULD64,
	UOP_GPU_FSURED,
	UOP_GPU_FSURED64,
	UOP_GPU_FSUST,
	UOP_GPU_FSUST64,
	UOP_GPU_FSUQ,
  UOP_GPU_FTESTP,
  UOP_GPU_FTESTP64,
  UOP_GPU_FTEX,
  UOP_GPU_FTLD4,
  UOP_GPU_FTXQ,
  UOP_GPU_FTRAP,
  UOP_GPU_FVABSDIFF,
  UOP_GPU_FVADD,
  UOP_GPU_FVMAD,
  UOP_GPU_FVMAX,
  UOP_GPU_FVMIN,
  UOP_GPU_FVSET,
  UOP_GPU_FVSHL,
  UOP_GPU_FVSHR,
  UOP_GPU_FVSUB,
  UOP_GPU_FVOTE,
  UOP_GPU_FXOR,
  UOP_GPU_FXOR64,
  UOP_GPU_FRECONVERGE,
  UOP_GPU_FPHI,
  // end of additions for GPU instructions

  NUM_UOP_TYPES,
} Uop_Type;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief  Mem_Type breaks down the different types of memory operations into
/// loads, stores, and prefetches.
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum Mem_Type_enum {
  NOT_MEM,                      //!< not a memory instruction
  MEM_LD,                       //!< a load instruction
  MEM_ST,                       //!< a store instruction
  MEM_PF,                       //!< a prefetch
  MEM_WH,                       //!< a write hint
  MEM_EVICT,                    //!< a cache block eviction hint
  MEM_SWPREF_NTA,
  MEM_SWPREF_T0,
  MEM_SWPREF_T1,
  MEM_SWPREF_T2,
  MEM_LD_LM,                    //!< local memory access in PTX 
  MEM_LD_SM,                    //!< shared memory access in PTX 
  MEM_LD_GM,                    //!< global memory access in PTX 
  MEM_ST_LM,                    //!< local memory access in PTX 
  MEM_ST_SM,                    //!< shared memory access in PTX 
  MEM_ST_GM,                    //!< global memory access in PTX 
  MEM_LD_CM,                    //!< load from constant memory in PTX 
  MEM_LD_TM,                    //!< load from texture memory in PTX
  MEM_LD_PM,                    //!< load from parameter memory in PTX
  NUM_MEM_TYPES,
} Mem_Type;


typedef enum Bar_Type_enum {
  NOT_BAR       = 0x0,          //!< not a barrier-causing instruction
  BAR_FETCH     = 0x1,          //!< causes fetch to halt until a redirect occurs
  BAR_ISSUE     = 0x2,          //!< causes issue to serialize around the instruction
  PTX_BLOCK_BAR = 0x3,          //!< synchronizations with a block 
  ACQ_BAR       = 0x4,          //!< Acquire barrier, not used
  REL_BAR       = 0x5,          //!< Release barrier
} Bar_Type;


/*!
 * Upp_State is the state of the op in the datapath --- if you modify
 * this make sure you update the op_state_names array in debug_print.c
 */
typedef enum Uop_State_enum {
  OS_FETCHED,                   //!< uop has been   fetched, awaiting issue
  OS_ISSUED,                    //!< uop has been    issued, waiting for its sources
  OS_SCHEDULED,                 //!< uop has been scheduled, awaiting complection
  OS_MISS,                      //!< uop has missed in the dcache
  OS_WAIT_MEM,                  //!< uop is waiting for a port or a miss_buffer entry
  OS_DONE,                      //!< uop is finished   executing, awaiting retirement
  OS_ALLOCATE,
  OS_EXEC,
  OS_MERGED,
  OS_SCHEDULE,
  OS_SCHEDULE_DONE,
  OS_DCACHE_BEGIN,
  OS_DCACHE_HIT,
  OS_DCACHE_ACCESS,
  OS_DCACHE_MEM_ACCESS_DENIED,
  OS_EXEC_BEGIN,
  NUM_OP_STATES,
} Uop_State;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Branch instruction type
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum Cf_Type_enum {
  NOT_CF,                       //!< not a control flow instruction
  CF_BR,                        //!< an unconditional branch
  CF_CBR,                       //!< a conditional branch
  CF_CALL,                      //!< a call
  // below this point are indirect cfs
  CF_IBR,                       //!< an indirect branch // non conditional
  CF_ICALL,                     //!< an indirect call
  CF_ICO,                       //!< an indirect jump to co-routine
  CF_RET,                       //!< a return
  CF_MITE,                      //!< alpha PAL, micro-instruction assited instructions
  NUM_CF_TYPES,
} Cf_Type;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Dependence type
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum Dep_Type_enum {
  REG_DATA_DEP,
  MEM_ADDR_DEP,
  MEM_DATA_DEP,
  PREV_UOP_DEP,
  NUM_DEP_TYPES,
} Dep_Type;

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Source uop information class
///////////////////////////////////////////////////////////////////////////////////////////////
class src_info_c {
  public:
    Dep_Type  m_type; /**< dependence type */
    uop_c    *m_uop; /**< uop pointer */
    Counter   m_uop_num; /**< uop number */
    Counter   m_unique_num; /**< uop unique number */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief uop information clas
///////////////////////////////////////////////////////////////////////////////////////////////
class  uop_info_c
{
  public:
    Addr  m_pred_npc;           /**< predicted next pc */
    Addr  m_pred_addr;          /**< address used to predict branch */
    uns8  m_pred;               /**< branch direction */
    bool  m_misfetch;           /**< target address is the ONLY thing that was wrong */
    bool  m_mispred;            /**< direction of the branch was mispredicted */
    bool  m_originally_mispred; /**< branch was originally mispredicted */
    bool  m_originally_misfetch; /**< branch was originally misfetched */
    bool  m_btb_miss;           /**< target is not known at prediction time */
    bool  m_btb_miss_resolved;  /**< btb miss is handled */ 
    bool  m_no_target;          /**< no target for this branch at prediction time */
    bool  m_ibp_miss;           /**< indirect branch miss */
    bool  m_icmiss;             /**< instruction cache miss */
    bool  m_dcmiss;             /**< data cache miss */
    bool  m_l2_miss;            /**< l2 miss */
    uns32 m_pred_global_hist;   /**< global branch history 32-bit */
    uns64 m_pred_global_hist_64; /**< global branch history 64-bit */
    int32 m_perceptron_output;  /**< perceptron bp output */
    int   m_btb_set;            /**< btb set address */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief branch misprediction recovery information
///////////////////////////////////////////////////////////////////////////////////////////////
class recovery_info_c
{
  public:
    /**
     * Constructor 
     */
    recovery_info_c() {}

    /**
     * Destructor
     */
    ~recovery_info_c() {}

  public:
    uns32 m_global_hist; /**< global branch history 32-bit */
    uns64 m_global_hist_64; /**< global branch history 64-bit */
    int   m_thread_id; /**< thread id */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Micro-Op (uop) class
///////////////////////////////////////////////////////////////////////////////////////////////
class uop_c
{
  public:
    /**
     * Constructor
     */
    uop_c();

    /**
     * Constructor
     */
    uop_c(macsim_c* simBase);

    /**
     * Initilization
     */
    void init();

    /**
     * Reset
     */
    uop_c* free();

    /**
     * Allocate (set)
     */
    void allocate();
    
    static const char *g_mem_type_name[NUM_MEM_TYPES]; /**< uop memory type string */
    static const char *g_uop_state_name[NUM_OP_STATES]; /**< uop state string */
    static const char *g_cf_type_name[NUM_CF_TYPES]; /**< branch type string */
    static const char *g_dep_type_name[NUM_DEP_TYPES]; /**< uop dependence type string */
    static const char *g_uop_type_name[NUM_UOP_TYPES]; /**< uop type string */


    Counter           m_uop_num; /**< uop number */
    Counter           m_unique_num; /**< uop unique number */
    Counter           m_inst_num; /**< instruction number */
    int               m_thread_id; /**< thread id */
    int               m_unique_thread_id; /**< unique thread id */
    int               m_orig_thread_id; /**< original thread id */
    int               m_orig_block_id; /**< original GPU block id */
    uns               m_block_id; /**< GPU data structure */
    int               m_core_id; /**< core id */
    Addr              m_pc; /**< pc address */
    Addr              m_npc; /**< next pc */
    uint16_t          m_opcode; /**< opcode */
    Uop_Type          m_uop_type; /**< uop type */
    Cf_Type           m_cf_type; /**< branch type */
    Mem_Type          m_mem_type; /**< memory type */
    Bar_Type          m_bar_type; /**< barrier type */

    Counter           m_fetched_cycle; /**< fetched cycle */
    Counter           m_bp_cycle; /**< branch predictor access cycle */
    Counter           m_alloc_cycle; /**< allocated cycle */
    Counter           m_sched_cycle; /**< scheduled cycle */
    Counter           m_exec_cycle; /**< execution cycle */
    Counter           m_done_cycle; /**< done cycle */

    int               m_num_srcs; /**< number of src registers */
    int               m_num_dests; /**< number of dest registers */
    Addr              m_vaddr; /**< memory address */
    int               m_mem_size; /**< memory access size */
    uns8              m_dir; /**< branch direction */
    uns16             m_src_info[MAX_SRCS]; /**< src uop info */
    src_info_c        m_map_src_info[MAX_UOP_SRC_DEPS]; /**< src map information */
    Counter           m_src_uop_num; /**< number of source uops */
    uns16             m_dest_info[MAX_DESTS]; /**< destination information */
    bool              m_valid; /**< valid uop */
    bool              m_bogus;  /**< mispredicted uops */
    bool              m_off_path; /**< uop in wrong-path */
    bool              m_mispredicted; /**< mispredicted branch */
    Addr              m_target_addr; /**< branch target address */
    uint32_t          m_active_mask; /**< GPU : active mask */
    uint32_t          m_taken_mask; /**< GPU : taken mask */
    Addr              m_reconverge_addr; /**< GPU : reconvergence address */
    Uop_State         m_state; /**< the state of the op in the datapath */
    int               m_rob_entry; /**< rob entry id */
    bool              m_in_scheduler; /**< in scheduler */
    ALLOCQ_Type       m_allocq_num; /**< alloc queue id */
    bool              m_in_iaq; /**< in allocation queue */
    Counter *         m_last_dep_exec; /**< last dependent execution cycle */
    bool              m_srcs_rdy; /**< source ready */
    uop_info_c        m_uop_info; /**< uop microarchitecture info */
    bool              m_isitBOM; /**< first uop of an instruction */
    bool              m_isitEOM; /**< last uop of an instruction */
    bool              m_last_uop; /**< last uop of a thread */
    recovery_info_c   m_recovery_info; /**< recovery information */
    int               m_srcs_not_rdy_vector; /**< src not ready bit vector */
    int               m_num_child_uops; /**< number of children uops */
    int               m_num_child_uops_done; /**< number of done children uops */
    uop_c           **m_child_uops; /**< children uops */
    uop_c            *m_parent_uop; /**< parent uop */
    uns64             m_pending_child_uops; /**< pending child uops vector */
    mem_req_s *       m_req; /**< pointer to memory request */ 
    bool              m_uncoalesced_flag; /**< uncoalesced flag */
    Counter           m_mem_start_cycle; /**< mem start cycle */
    bool              m_req_sb; /**< need store buffer */
    bool              m_req_lb; /**< need load buffer */
    bool              m_req_int_reg; /**< need integer register */
    bool              m_req_fp_reg; /**< need fp register */
    int               m_dcache_bank_id; /**< dcache bank id */
    bool              m_bypass_llc; /**< bypass last level cache */
    bool              m_skip_llc; /**< skip last level cache */

    // hmc info 
    // changed by Lifeng
    HMC_Type m_hmc_inst;  /**< hmc type of current uop*/
  private:
    macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */

};

#endif  // UOP_H_INCLUDED

