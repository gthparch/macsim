/* ****************************************************************************************** */ 
/* memory address trace generator */ 
/* For each version of trace generator, we need to change XED Opcdoe just like trace_read.cc  */ 
/* ****************************************************************************************** */  

#define REP_MOV_MEM_SIZE_MAX 4
#define REP_MOV_MEM_SIZE_MAX_NEW MAX2(REP_MOV_MEM_SIZE_MAX, (*KNOB(KNOB_MEM_SIZE_AMP)*4))
#define MAX_SRC_NUM 9
#define MAX_DST_NUM 6


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
} trace_info_cpu_s;




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
