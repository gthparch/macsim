#include "trace_read_a64.h"
#include "process_manager.h"
#include "assert_macros.h"
#include "debug_macros.h"
#include "utils.h"
#include "all_knobs.h"

#define DEBUG(args...)   _DEBUG(*KNOB(KNOB_DEBUG_TRACE_READ), ## args)
#define DEBUG_CORE(m_core_id, args...)                            \
  if (m_core_id == *m_simBase->m_knobs->KNOB_DEBUG_CORE_ID) {     \
    _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_TRACE_READ, ## args);  \
  }

const char* a64_opcode_names[ARM64_INS_ENDING] =  {
  "ARM64_INS_INVALID",

  "ARM64_INS_ABS",
  "ARM64_INS_ADC",
  "ARM64_INS_ADDHN",
  "ARM64_INS_ADDHN2",
  "ARM64_INS_ADDP",
  "ARM64_INS_ADD",
  "ARM64_INS_ADDV",
  "ARM64_INS_ADR",
  "ARM64_INS_ADRP",
  "ARM64_INS_AESD",
  "ARM64_INS_AESE",
  "ARM64_INS_AESIMC",
  "ARM64_INS_AESMC",
  "ARM64_INS_AND",
  "ARM64_INS_ASR",
  "ARM64_INS_B",
  "ARM64_INS_BFM",
  "ARM64_INS_BIC",
  "ARM64_INS_BIF",
  "ARM64_INS_BIT",
  "ARM64_INS_BL",
  "ARM64_INS_BLR",
  "ARM64_INS_BR",
  "ARM64_INS_BRK",
  "ARM64_INS_BSL",
  "ARM64_INS_CBNZ",
  "ARM64_INS_CBZ",
  "ARM64_INS_CCMN",
  "ARM64_INS_CCMP",
  "ARM64_INS_CLREX",
  "ARM64_INS_CLS",
  "ARM64_INS_CLZ",
  "ARM64_INS_CMEQ",
  "ARM64_INS_CMGE",
  "ARM64_INS_CMGT",
  "ARM64_INS_CMHI",
  "ARM64_INS_CMHS",
  "ARM64_INS_CMLE",
  "ARM64_INS_CMLT",
  "ARM64_INS_CMTST",
  "ARM64_INS_CNT",
  "ARM64_INS_MOV",
  "ARM64_INS_CRC32B",
  "ARM64_INS_CRC32CB",
  "ARM64_INS_CRC32CH",
  "ARM64_INS_CRC32CW",
  "ARM64_INS_CRC32CX",
  "ARM64_INS_CRC32H",
  "ARM64_INS_CRC32W",
  "ARM64_INS_CRC32X",
  "ARM64_INS_CSEL",
  "ARM64_INS_CSINC",
  "ARM64_INS_CSINV",
  "ARM64_INS_CSNEG",
  "ARM64_INS_DCPS1",
  "ARM64_INS_DCPS2",
  "ARM64_INS_DCPS3",
  "ARM64_INS_DMB",
  "ARM64_INS_DRPS",
  "ARM64_INS_DSB",
  "ARM64_INS_DUP",
  "ARM64_INS_EON",
  "ARM64_INS_EOR",
  "ARM64_INS_ERET",
  "ARM64_INS_EXTR",
  "ARM64_INS_EXT",
  "ARM64_INS_FABD",
  "ARM64_INS_FABS",
  "ARM64_INS_FACGE",
  "ARM64_INS_FACGT",
  "ARM64_INS_FADD",
  "ARM64_INS_FADDP",
  "ARM64_INS_FCCMP",
  "ARM64_INS_FCCMPE",
  "ARM64_INS_FCMEQ",
  "ARM64_INS_FCMGE",
  "ARM64_INS_FCMGT",
  "ARM64_INS_FCMLE",
  "ARM64_INS_FCMLT",
  "ARM64_INS_FCMP",
  "ARM64_INS_FCMPE",
  "ARM64_INS_FCSEL",
  "ARM64_INS_FCVTAS",
  "ARM64_INS_FCVTAU",
  "ARM64_INS_FCVT",
  "ARM64_INS_FCVTL",
  "ARM64_INS_FCVTL2",
  "ARM64_INS_FCVTMS",
  "ARM64_INS_FCVTMU",
  "ARM64_INS_FCVTNS",
  "ARM64_INS_FCVTNU",
  "ARM64_INS_FCVTN",
  "ARM64_INS_FCVTN2",
  "ARM64_INS_FCVTPS",
  "ARM64_INS_FCVTPU",
  "ARM64_INS_FCVTXN",
  "ARM64_INS_FCVTXN2",
  "ARM64_INS_FCVTZS",
  "ARM64_INS_FCVTZU",
  "ARM64_INS_FDIV",
  "ARM64_INS_FMADD",
  "ARM64_INS_FMAX",
  "ARM64_INS_FMAXNM",
  "ARM64_INS_FMAXNMP",
  "ARM64_INS_FMAXNMV",
  "ARM64_INS_FMAXP",
  "ARM64_INS_FMAXV",
  "ARM64_INS_FMIN",
  "ARM64_INS_FMINNM",
  "ARM64_INS_FMINNMP",
  "ARM64_INS_FMINNMV",
  "ARM64_INS_FMINP",
  "ARM64_INS_FMINV",
  "ARM64_INS_FMLA",
  "ARM64_INS_FMLS",
  "ARM64_INS_FMOV",
  "ARM64_INS_FMSUB",
  "ARM64_INS_FMUL",
  "ARM64_INS_FMULX",
  "ARM64_INS_FNEG",
  "ARM64_INS_FNMADD",
  "ARM64_INS_FNMSUB",
  "ARM64_INS_FNMUL",
  "ARM64_INS_FRECPE",
  "ARM64_INS_FRECPS",
  "ARM64_INS_FRECPX",
  "ARM64_INS_FRINTA",
  "ARM64_INS_FRINTI",
  "ARM64_INS_FRINTM",
  "ARM64_INS_FRINTN",
  "ARM64_INS_FRINTP",
  "ARM64_INS_FRINTX",
  "ARM64_INS_FRINTZ",
  "ARM64_INS_FRSQRTE",
  "ARM64_INS_FRSQRTS",
  "ARM64_INS_FSQRT",
  "ARM64_INS_FSUB",
  "ARM64_INS_HINT",
  "ARM64_INS_HLT",
  "ARM64_INS_HVC",
  "ARM64_INS_INS",

  "ARM64_INS_ISB",
  "ARM64_INS_LD1",
  "ARM64_INS_LD1R",
  "ARM64_INS_LD2R",
  "ARM64_INS_LD2",
  "ARM64_INS_LD3R",
  "ARM64_INS_LD3",
  "ARM64_INS_LD4",
  "ARM64_INS_LD4R",

  "ARM64_INS_LDARB",
  "ARM64_INS_LDARH",
  "ARM64_INS_LDAR",
  "ARM64_INS_LDAXP",
  "ARM64_INS_LDAXRB",
  "ARM64_INS_LDAXRH",
  "ARM64_INS_LDAXR",
  "ARM64_INS_LDNP",
  "ARM64_INS_LDP",
  "ARM64_INS_LDPSW",
  "ARM64_INS_LDRB",
  "ARM64_INS_LDR",
  "ARM64_INS_LDRH",
  "ARM64_INS_LDRSB",
  "ARM64_INS_LDRSH",
  "ARM64_INS_LDRSW",
  "ARM64_INS_LDTRB",
  "ARM64_INS_LDTRH",
  "ARM64_INS_LDTRSB",

  "ARM64_INS_LDTRSH",
  "ARM64_INS_LDTRSW",
  "ARM64_INS_LDTR",
  "ARM64_INS_LDURB",
  "ARM64_INS_LDUR",
  "ARM64_INS_LDURH",
  "ARM64_INS_LDURSB",
  "ARM64_INS_LDURSH",
  "ARM64_INS_LDURSW",
  "ARM64_INS_LDXP",
  "ARM64_INS_LDXRB",
  "ARM64_INS_LDXRH",
  "ARM64_INS_LDXR",
  "ARM64_INS_LSL",
  "ARM64_INS_LSR",
  "ARM64_INS_MADD",
  "ARM64_INS_MLA",
  "ARM64_INS_MLS",
  "ARM64_INS_MOVI",
  "ARM64_INS_MOVK",
  "ARM64_INS_MOVN",
  "ARM64_INS_MOVZ",
  "ARM64_INS_MRS",
  "ARM64_INS_MSR",
  "ARM64_INS_MSUB",
  "ARM64_INS_MUL",
  "ARM64_INS_MVNI",
  "ARM64_INS_NEG",
  "ARM64_INS_NOT",
  "ARM64_INS_ORN",
  "ARM64_INS_ORR",
  "ARM64_INS_PMULL2",
  "ARM64_INS_PMULL",
  "ARM64_INS_PMUL",
  "ARM64_INS_PRFM",
  "ARM64_INS_PRFUM",
  "ARM64_INS_RADDHN",
  "ARM64_INS_RADDHN2",
  "ARM64_INS_RBIT",
  "ARM64_INS_RET",
  "ARM64_INS_REV16",
  "ARM64_INS_REV32",
  "ARM64_INS_REV64",
  "ARM64_INS_REV",
  "ARM64_INS_ROR",
  "ARM64_INS_RSHRN2",
  "ARM64_INS_RSHRN",
  "ARM64_INS_RSUBHN",
  "ARM64_INS_RSUBHN2",
  "ARM64_INS_SABAL2",
  "ARM64_INS_SABAL",

  "ARM64_INS_SABA",
  "ARM64_INS_SABDL2",
  "ARM64_INS_SABDL",
  "ARM64_INS_SABD",
  "ARM64_INS_SADALP",
  "ARM64_INS_SADDLP",
  "ARM64_INS_SADDLV",
  "ARM64_INS_SADDL2",
  "ARM64_INS_SADDL",
  "ARM64_INS_SADDW2",
  "ARM64_INS_SADDW",
  "ARM64_INS_SBC",
  "ARM64_INS_SBFM",
  "ARM64_INS_SCVTF",
  "ARM64_INS_SDIV",
  "ARM64_INS_SHA1C",
  "ARM64_INS_SHA1H",
  "ARM64_INS_SHA1M",
  "ARM64_INS_SHA1P",
  "ARM64_INS_SHA1SU0",
  "ARM64_INS_SHA1SU1",
  "ARM64_INS_SHA256H2",
  "ARM64_INS_SHA256H",
  "ARM64_INS_SHA256SU0",
  "ARM64_INS_SHA256SU1",
  "ARM64_INS_SHADD",
  "ARM64_INS_SHLL2",
  "ARM64_INS_SHLL",
  "ARM64_INS_SHL",
  "ARM64_INS_SHRN2",
  "ARM64_INS_SHRN",
  "ARM64_INS_SHSUB",
  "ARM64_INS_SLI",
  "ARM64_INS_SMADDL",
  "ARM64_INS_SMAXP",
  "ARM64_INS_SMAXV",
  "ARM64_INS_SMAX",
  "ARM64_INS_SMC",
  "ARM64_INS_SMINP",
  "ARM64_INS_SMINV",
  "ARM64_INS_SMIN",
  "ARM64_INS_SMLAL2",
  "ARM64_INS_SMLAL",
  "ARM64_INS_SMLSL2",
  "ARM64_INS_SMLSL",
  "ARM64_INS_SMOV",
  "ARM64_INS_SMSUBL",
  "ARM64_INS_SMULH",
  "ARM64_INS_SMULL2",
  "ARM64_INS_SMULL",
  "ARM64_INS_SQABS",
  "ARM64_INS_SQADD",
  "ARM64_INS_SQDMLAL",
  "ARM64_INS_SQDMLAL2",
  "ARM64_INS_SQDMLSL",
  "ARM64_INS_SQDMLSL2",
  "ARM64_INS_SQDMULH",
  "ARM64_INS_SQDMULL",
  "ARM64_INS_SQDMULL2",
  "ARM64_INS_SQNEG",
  "ARM64_INS_SQRDMULH",
  "ARM64_INS_SQRSHL",
  "ARM64_INS_SQRSHRN",
  "ARM64_INS_SQRSHRN2",
  "ARM64_INS_SQRSHRUN",
  "ARM64_INS_SQRSHRUN2",
  "ARM64_INS_SQSHLU",
  "ARM64_INS_SQSHL",
  "ARM64_INS_SQSHRN",
  "ARM64_INS_SQSHRN2",
  "ARM64_INS_SQSHRUN",
  "ARM64_INS_SQSHRUN2",
  "ARM64_INS_SQSUB",
  "ARM64_INS_SQXTN2",
  "ARM64_INS_SQXTN",
  "ARM64_INS_SQXTUN2",
  "ARM64_INS_SQXTUN",
  "ARM64_INS_SRHADD",
  "ARM64_INS_SRI",
  "ARM64_INS_SRSHL",
  "ARM64_INS_SRSHR",
  "ARM64_INS_SRSRA",
  "ARM64_INS_SSHLL2",
  "ARM64_INS_SSHLL",
  "ARM64_INS_SSHL",
  "ARM64_INS_SSHR",
  "ARM64_INS_SSRA",
  "ARM64_INS_SSUBL2",
  "ARM64_INS_SSUBL",
  "ARM64_INS_SSUBW2",
  "ARM64_INS_SSUBW",
  "ARM64_INS_ST1",
  "ARM64_INS_ST2",
  "ARM64_INS_ST3",
  "ARM64_INS_ST4",
  "ARM64_INS_STLRB",
  "ARM64_INS_STLRH",
  "ARM64_INS_STLR",
  "ARM64_INS_STLXP",
  "ARM64_INS_STLXRB",
  "ARM64_INS_STLXRH",
  "ARM64_INS_STLXR",
  "ARM64_INS_STNP",
  "ARM64_INS_STP",
  "ARM64_INS_STRB",
  "ARM64_INS_STR",
  "ARM64_INS_STRH",
  "ARM64_INS_STTRB",
  "ARM64_INS_STTRH",
  "ARM64_INS_STTR",
  "ARM64_INS_STURB",
  "ARM64_INS_STUR",
  "ARM64_INS_STURH",
  "ARM64_INS_STXP",
  "ARM64_INS_STXRB",
  "ARM64_INS_STXRH",
  "ARM64_INS_STXR",
  "ARM64_INS_SUBHN",
  "ARM64_INS_SUBHN2",
  "ARM64_INS_SUB",
  "ARM64_INS_SUQADD",
  "ARM64_INS_SVC",
  "ARM64_INS_SYSL",
  "ARM64_INS_SYS",
  "ARM64_INS_TBL",
  "ARM64_INS_TBNZ",
  "ARM64_INS_TBX",
  "ARM64_INS_TBZ",
  "ARM64_INS_TRN1",
  "ARM64_INS_TRN2",
  "ARM64_INS_UABAL2",
  "ARM64_INS_UABAL",
  "ARM64_INS_UABA",
  "ARM64_INS_UABDL2",
  "ARM64_INS_UABDL",
  "ARM64_INS_UABD",
  "ARM64_INS_UADALP",
  "ARM64_INS_UADDLP",
  "ARM64_INS_UADDLV",
  "ARM64_INS_UADDL2",
  "ARM64_INS_UADDL",
  "ARM64_INS_UADDW2",
  "ARM64_INS_UADDW",
  "ARM64_INS_UBFM",
  "ARM64_INS_UCVTF",
  "ARM64_INS_UDIV",
  "ARM64_INS_UHADD",
  "ARM64_INS_UHSUB",
  "ARM64_INS_UMADDL",
  "ARM64_INS_UMAXP",
  "ARM64_INS_UMAXV",
  "ARM64_INS_UMAX",
  "ARM64_INS_UMINP",
  "ARM64_INS_UMINV",
  "ARM64_INS_UMIN",
  "ARM64_INS_UMLAL2",
  "ARM64_INS_UMLAL",
  "ARM64_INS_UMLSL2",
  "ARM64_INS_UMLSL",
  "ARM64_INS_UMOV",
  "ARM64_INS_UMSUBL",
  "ARM64_INS_UMULH",
  "ARM64_INS_UMULL2",
  "ARM64_INS_UMULL",
  "ARM64_INS_UQADD",
  "ARM64_INS_UQRSHL",
  "ARM64_INS_UQRSHRN",
  "ARM64_INS_UQRSHRN2",
  "ARM64_INS_UQSHL",
  "ARM64_INS_UQSHRN",
  "ARM64_INS_UQSHRN2",
  "ARM64_INS_UQSUB",
  "ARM64_INS_UQXTN2",
  "ARM64_INS_UQXTN",
  "ARM64_INS_URECPE",
  "ARM64_INS_URHADD",
  "ARM64_INS_URSHL",
  "ARM64_INS_URSHR",
  "ARM64_INS_URSQRTE",
  "ARM64_INS_URSRA",
  "ARM64_INS_USHLL2",
  "ARM64_INS_USHLL",
  "ARM64_INS_USHL",
  "ARM64_INS_USHR",
  "ARM64_INS_USQADD",
  "ARM64_INS_USRA",
  "ARM64_INS_USUBL2",
  "ARM64_INS_USUBL",
  "ARM64_INS_USUBW2",
  "ARM64_INS_USUBW",
  "ARM64_INS_UZP1",
  "ARM64_INS_UZP2",
  "ARM64_INS_XTN2",
  "ARM64_INS_XTN",
  "ARM64_INS_ZIP1",
  "ARM64_INS_ZIP2",

  // alias insn
  "ARM64_INS_MNEG",
  "ARM64_INS_UMNEGL",
  "ARM64_INS_SMNEGL",
  "ARM64_INS_NOP",
  "ARM64_INS_YIELD",
  "ARM64_INS_WFE",
  "ARM64_INS_WFI",
  "ARM64_INS_SEV",
  "ARM64_INS_SEVL",
  "ARM64_INS_NGC",
  "ARM64_INS_SBFIZ",
  "ARM64_INS_UBFIZ",
  "ARM64_INS_SBFX",
  "ARM64_INS_UBFX",
  "ARM64_INS_BFI",
  "ARM64_INS_BFXIL",
  "ARM64_INS_CMN",
  "ARM64_INS_MVN",
  "ARM64_INS_TST",
  "ARM64_INS_CSET",
  "ARM64_INS_CINC",
  "ARM64_INS_CSETM",
  "ARM64_INS_CINV",
  "ARM64_INS_CNEG",
  "ARM64_INS_SXTB",
  "ARM64_INS_SXTH",
  "ARM64_INS_SXTW",
  "ARM64_INS_CMP",
  "ARM64_INS_UXTB",
  "ARM64_INS_UXTH",
  "ARM64_INS_UXTW",
  "ARM64_INS_IC",
  "ARM64_INS_DC",
  "ARM64_INS_AT",
  "ARM64_INS_TLBI"
  //"ARM64_INS_ENDING"  // <-- mark the end of the list of insn
};

void a64_decoder_c::init_pin_convert()
{
  m_int_uop_table[ARM64_INS_ABS] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_ADC] = UOP_IADD;
  m_int_uop_table[ARM64_INS_ADDHN] = UOP_IADD;
  m_int_uop_table[ARM64_INS_ADDHN2] = UOP_IADD;
  m_int_uop_table[ARM64_INS_ADDP] = UOP_IADD;
  m_int_uop_table[ARM64_INS_ADD] = UOP_IADD;
  m_int_uop_table[ARM64_INS_ADDV] = UOP_IADD;
  m_int_uop_table[ARM64_INS_ADR] = UOP_LD;
  m_int_uop_table[ARM64_INS_ADRP] = UOP_LD;
  m_int_uop_table[ARM64_INS_AESD] = UOP_SSE;
  m_int_uop_table[ARM64_INS_AESE] = UOP_SSE;
  m_int_uop_table[ARM64_INS_AESIMC] = UOP_SSE;
  m_int_uop_table[ARM64_INS_AESMC] = UOP_SSE;
  m_int_uop_table[ARM64_INS_AND] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_ASR] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_B] = UOP_CF;
  m_int_uop_table[ARM64_INS_BFM] = UOP_BYTE;
  m_int_uop_table[ARM64_INS_BIC] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_BIF] = UOP_BYTE;
  m_int_uop_table[ARM64_INS_BIT] = UOP_BYTE;
  m_int_uop_table[ARM64_INS_BL] = UOP_CF;
  m_int_uop_table[ARM64_INS_BLR] = UOP_CF;
  m_int_uop_table[ARM64_INS_BR] = UOP_CF;
  m_int_uop_table[ARM64_INS_BRK] = UOP_CF;
  m_int_uop_table[ARM64_INS_BSL] = UOP_BYTE;
  m_int_uop_table[ARM64_INS_CBNZ] = UOP_CF;
  m_int_uop_table[ARM64_INS_CBZ] = UOP_CF;
  m_int_uop_table[ARM64_INS_CCMN] = UOP_ICMP;
  m_int_uop_table[ARM64_INS_CCMP] = UOP_ICMP;
  m_int_uop_table[ARM64_INS_CLREX] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_CLS] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_CLZ] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_CMEQ] = UOP_ICMP;
  m_int_uop_table[ARM64_INS_CMGE] = UOP_ICMP;
  m_int_uop_table[ARM64_INS_CMGT] = UOP_ICMP;
  m_int_uop_table[ARM64_INS_CMHI] = UOP_ICMP;
  m_int_uop_table[ARM64_INS_CMHS] = UOP_ICMP;
  m_int_uop_table[ARM64_INS_CMLE] = UOP_ICMP;
  m_int_uop_table[ARM64_INS_CMLT] = UOP_ICMP;
  m_int_uop_table[ARM64_INS_CMTST] = UOP_ICMP;
  m_int_uop_table[ARM64_INS_CNT] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_MOV] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_CRC32B] = UOP_SSE;
  m_int_uop_table[ARM64_INS_CRC32CB] = UOP_SSE;
  m_int_uop_table[ARM64_INS_CRC32CH] = UOP_SSE;
  m_int_uop_table[ARM64_INS_CRC32CW] = UOP_SSE;
  m_int_uop_table[ARM64_INS_CRC32CX] = UOP_SSE;
  m_int_uop_table[ARM64_INS_CRC32H] = UOP_SSE;
  m_int_uop_table[ARM64_INS_CRC32W] = UOP_SSE;
  m_int_uop_table[ARM64_INS_CRC32X] = UOP_SSE;
  m_int_uop_table[ARM64_INS_CSEL] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_CSINC] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_CSINV] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_CSNEG] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_DCPS1] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_DCPS2] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_DCPS3] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_DMB] = UOP_FULL_FENCE;
  m_int_uop_table[ARM64_INS_DRPS] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_DSB] = UOP_FULL_FENCE;
  m_int_uop_table[ARM64_INS_DUP] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_EON] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_EOR] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_ERET] = UOP_CF;
  m_int_uop_table[ARM64_INS_EXTR] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_EXT] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_FABD] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FABS] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FACGE] = UOP_FCMP;
  m_int_uop_table[ARM64_INS_FACGT] = UOP_FCMP;
  m_int_uop_table[ARM64_INS_FADD] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FADDP] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FCCMP] = UOP_FCMP;
  m_int_uop_table[ARM64_INS_FCCMPE] = UOP_FCMP;
  m_int_uop_table[ARM64_INS_FCMEQ] = UOP_FCMP;
  m_int_uop_table[ARM64_INS_FCMGE] = UOP_FCMP;
  m_int_uop_table[ARM64_INS_FCMGT] = UOP_FCMP;
  m_int_uop_table[ARM64_INS_FCMLE] = UOP_FCMP;
  m_int_uop_table[ARM64_INS_FCMLT] = UOP_FCMP;
  m_int_uop_table[ARM64_INS_FCMP] = UOP_FCMP;
  m_int_uop_table[ARM64_INS_FCMPE] = UOP_FCMP;
  m_int_uop_table[ARM64_INS_FCSEL] = UOP_FCMP;
  m_int_uop_table[ARM64_INS_FCVTAS] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_FCVTAU] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_FCVT] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_FCVTL] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_FCVTL2] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_FCVTMS] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_FCVTMU] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_FCVTNS] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_FCVTNU] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_FCVTN] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_FCVTN2] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_FCVTPS] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_FCVTPU] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_FCVTXN] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_FCVTXN2] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_FCVTZS] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_FCVTZU] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_FDIV] = UOP_FDIV;
  m_int_uop_table[ARM64_INS_FMADD] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FMAX] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FMAXNM] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FMAXNMP] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FMAXNMV] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FMAXP] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FMAXV] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FMIN] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FMINNM] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FMINNMP] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FMINNMV] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FMINP] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FMINV] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FMLA] = UOP_FMUL;
  m_int_uop_table[ARM64_INS_FMLS] = UOP_FMUL;
  m_int_uop_table[ARM64_INS_FMOV] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FMSUB] = UOP_FMUL;
  m_int_uop_table[ARM64_INS_FMUL] = UOP_FMUL;
  m_int_uop_table[ARM64_INS_FMULX] = UOP_FMUL;
  m_int_uop_table[ARM64_INS_FNEG] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FNMADD] = UOP_FMUL;
  m_int_uop_table[ARM64_INS_FNMSUB] = UOP_FMUL;
  m_int_uop_table[ARM64_INS_FNMUL] = UOP_FMUL;
  m_int_uop_table[ARM64_INS_FRECPE] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FRECPS] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FRECPX] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FRINTA] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FRINTI] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FRINTM] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FRINTN] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FRINTP] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FRINTX] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FRINTZ] = UOP_FADD;
  m_int_uop_table[ARM64_INS_FRSQRTE] = UOP_FDIV;
  m_int_uop_table[ARM64_INS_FRSQRTS] = UOP_FDIV;
  m_int_uop_table[ARM64_INS_FSQRT] = UOP_FDIV;
  m_int_uop_table[ARM64_INS_FSUB] = UOP_FADD;
  m_int_uop_table[ARM64_INS_HINT] = UOP_NOP;
  m_int_uop_table[ARM64_INS_HLT] = UOP_CF;
  m_int_uop_table[ARM64_INS_HVC] = UOP_CF;
  m_int_uop_table[ARM64_INS_INS] = UOP_LOGIC;

  m_int_uop_table[ARM64_INS_ISB] = UOP_NOP;
  m_int_uop_table[ARM64_INS_LD1] = UOP_LD;
  m_int_uop_table[ARM64_INS_LD1R] = UOP_LD;
  m_int_uop_table[ARM64_INS_LD2R] = UOP_LD;
  m_int_uop_table[ARM64_INS_LD2] = UOP_LD;
  m_int_uop_table[ARM64_INS_LD3R] = UOP_LD;
  m_int_uop_table[ARM64_INS_LD3] = UOP_LD;
  m_int_uop_table[ARM64_INS_LD4] = UOP_LD;
  m_int_uop_table[ARM64_INS_LD4R] = UOP_LD;

  m_int_uop_table[ARM64_INS_LDARB] = UOP_LD;
  m_int_uop_table[ARM64_INS_LDARH] = UOP_LD;
  m_int_uop_table[ARM64_INS_LDAR] = UOP_LD;
  m_int_uop_table[ARM64_INS_LDAXP] = UOP_LD;
  m_int_uop_table[ARM64_INS_LDAXRB] = UOP_LD;
  m_int_uop_table[ARM64_INS_LDAXRH] = UOP_LD;
  m_int_uop_table[ARM64_INS_LDAXR] = UOP_LD;
  m_int_uop_table[ARM64_INS_LDNP] = UOP_LD;
  m_int_uop_table[ARM64_INS_LDP] = UOP_LD;
  m_int_uop_table[ARM64_INS_LDPSW] = UOP_LD;
  m_int_uop_table[ARM64_INS_LDRB] = UOP_LD;
  m_int_uop_table[ARM64_INS_LDR] = UOP_LD;
  m_int_uop_table[ARM64_INS_LDRH] = UOP_LD;
  m_int_uop_table[ARM64_INS_LDRSB] = UOP_LD;
  m_int_uop_table[ARM64_INS_LDRSH] = UOP_LD;
  m_int_uop_table[ARM64_INS_LDRSW] = UOP_LD;
  m_int_uop_table[ARM64_INS_LDTRB] = UOP_LDA;
  m_int_uop_table[ARM64_INS_LDTRH] = UOP_LDA;
  m_int_uop_table[ARM64_INS_LDTRSB] = UOP_LDA;

  m_int_uop_table[ARM64_INS_LDTRSH] = UOP_LDA;
  m_int_uop_table[ARM64_INS_LDTRSW] = UOP_LDA;
  m_int_uop_table[ARM64_INS_LDTR] = UOP_LDA;
  m_int_uop_table[ARM64_INS_LDURB] = UOP_LDA;
  m_int_uop_table[ARM64_INS_LDUR] = UOP_LDA;
  m_int_uop_table[ARM64_INS_LDURH] = UOP_LDA;
  m_int_uop_table[ARM64_INS_LDURSB] = UOP_LDA;
  m_int_uop_table[ARM64_INS_LDURSH] = UOP_LDA;
  m_int_uop_table[ARM64_INS_LDURSW] = UOP_LDA;
  m_int_uop_table[ARM64_INS_LDXP] = UOP_LDA;
  m_int_uop_table[ARM64_INS_LDXRB] = UOP_LDA;
  m_int_uop_table[ARM64_INS_LDXRH] = UOP_LDA;
  m_int_uop_table[ARM64_INS_LDXR] = UOP_LDA;
  m_int_uop_table[ARM64_INS_LSL] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_LSR] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_MADD] = UOP_IADD;
  m_int_uop_table[ARM64_INS_MLA] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_MLS] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_MOVI] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_MOVK] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_MOVN] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_MOVZ] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_MRS] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_MSR] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_MSUB] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_MUL] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_MVNI] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_NEG] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_NOT] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_ORN] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_ORR] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_PMULL2] = UOP_FMUL;
  m_int_uop_table[ARM64_INS_PMULL] = UOP_FMUL;
  m_int_uop_table[ARM64_INS_PMUL] = UOP_FMUL;
  m_int_uop_table[ARM64_INS_PRFM] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_PRFUM] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_RADDHN] = UOP_IADD;
  m_int_uop_table[ARM64_INS_RADDHN2] = UOP_IADD;
  m_int_uop_table[ARM64_INS_RBIT] = UOP_BYTE;
  m_int_uop_table[ARM64_INS_RET] = UOP_CF;
  m_int_uop_table[ARM64_INS_REV16] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_REV32] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_REV64] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_REV] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_ROR] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_RSHRN2] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_RSHRN] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_RSUBHN] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_RSUBHN2] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SABAL2] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SABAL] = UOP_LOGIC;

  m_int_uop_table[ARM64_INS_SABA] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SABDL2] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SABDL] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SABD] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SADALP] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SADDLP] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SADDLV] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SADDL2] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SADDL] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SADDW2] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SADDW] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SBC] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SBFM] = UOP_BYTE;
  m_int_uop_table[ARM64_INS_SCVTF] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_SDIV] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SHA1C] = UOP_SSE;
  m_int_uop_table[ARM64_INS_SHA1H] = UOP_SSE;
  m_int_uop_table[ARM64_INS_SHA1M] = UOP_SSE;
  m_int_uop_table[ARM64_INS_SHA1P] = UOP_SSE;
  m_int_uop_table[ARM64_INS_SHA1SU0] = UOP_SSE;
  m_int_uop_table[ARM64_INS_SHA1SU1] = UOP_SSE;
  m_int_uop_table[ARM64_INS_SHA256H2] = UOP_SSE;
  m_int_uop_table[ARM64_INS_SHA256H] = UOP_SSE;
  m_int_uop_table[ARM64_INS_SHA256SU0] = UOP_SSE;
  m_int_uop_table[ARM64_INS_SHA256SU1] = UOP_SSE;
  m_int_uop_table[ARM64_INS_SHADD] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SHLL2] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SHLL] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SHL] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SHRN2] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SHRN] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SHSUB] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SLI] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SMADDL] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SMAXP] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SMAXV] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SMAX] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SMC] = UOP_CF;
  m_int_uop_table[ARM64_INS_SMINP] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SMINV] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SMIN] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SMLAL2] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SMLAL] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SMLSL2] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SMLSL] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SMOV] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SMSUBL] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SMULH] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SMULL2] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SMULL] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SQABS] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SQADD] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SQDMLAL] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SQDMLAL2] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SQDMLSL] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SQDMLSL2] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SQDMULH] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SQDMULL] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SQDMULL2] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SQNEG] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SQRDMULH] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SQRSHL] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SQRSHRN] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SQRSHRN2] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SQRSHRUN] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SQRSHRUN2] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SQSHLU] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SQSHL] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SQSHRN] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SQSHRN2] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SQSHRUN] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SQSHRUN2] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SQSUB] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SQXTN2] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SQXTN] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SQXTUN2] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SQXTUN] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SRHADD] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SRI] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SRSHL] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SRSHR] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SRSRA] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SSHLL2] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SSHLL] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SSHL] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SSHR] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SSRA] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_SSUBL2] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SSUBL] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SSUBW2] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SSUBW] = UOP_IADD;
  m_int_uop_table[ARM64_INS_ST1] = UOP_ST;
  m_int_uop_table[ARM64_INS_ST2] = UOP_ST;
  m_int_uop_table[ARM64_INS_ST3] = UOP_ST;
  m_int_uop_table[ARM64_INS_ST4] = UOP_ST;
  m_int_uop_table[ARM64_INS_STLRB] = UOP_ST;
  m_int_uop_table[ARM64_INS_STLRH] = UOP_ST;
  m_int_uop_table[ARM64_INS_STLR] = UOP_ST;
  m_int_uop_table[ARM64_INS_STLXP] = UOP_ST;
  m_int_uop_table[ARM64_INS_STLXRB] = UOP_ST;
  m_int_uop_table[ARM64_INS_STLXRH] = UOP_ST;
  m_int_uop_table[ARM64_INS_STLXR] = UOP_ST;
  m_int_uop_table[ARM64_INS_STNP] = UOP_ST;
  m_int_uop_table[ARM64_INS_STP] = UOP_ST;
  m_int_uop_table[ARM64_INS_STRB] = UOP_ST;
  m_int_uop_table[ARM64_INS_STR] = UOP_ST;
  m_int_uop_table[ARM64_INS_STRH] = UOP_ST;
  m_int_uop_table[ARM64_INS_STTRB] = UOP_ST;
  m_int_uop_table[ARM64_INS_STTRH] = UOP_ST;
  m_int_uop_table[ARM64_INS_STTR] = UOP_ST;
  m_int_uop_table[ARM64_INS_STURB] = UOP_ST;
  m_int_uop_table[ARM64_INS_STUR] = UOP_ST;
  m_int_uop_table[ARM64_INS_STURH] = UOP_ST;
  m_int_uop_table[ARM64_INS_STXP] = UOP_ST;
  m_int_uop_table[ARM64_INS_STXRB] = UOP_ST;
  m_int_uop_table[ARM64_INS_STXRH] = UOP_ST;
  m_int_uop_table[ARM64_INS_STXR] = UOP_ST;
  m_int_uop_table[ARM64_INS_SUBHN] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SUBHN2] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SUB] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SUQADD] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SVC] = UOP_CF;
  m_int_uop_table[ARM64_INS_SYSL] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SYS] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_TBL] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_TBNZ] = UOP_CF;
  m_int_uop_table[ARM64_INS_TBX] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_TBZ] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_TRN1] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_TRN2] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_UABAL2] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_UABAL] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_UABA] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_UABDL2] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_UABDL] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_UABD] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_UADALP] = UOP_IADD;
  m_int_uop_table[ARM64_INS_UADDLP] = UOP_IADD;
  m_int_uop_table[ARM64_INS_UADDLV] = UOP_IADD;
  m_int_uop_table[ARM64_INS_UADDL2] = UOP_IADD;
  m_int_uop_table[ARM64_INS_UADDL] = UOP_IADD;
  m_int_uop_table[ARM64_INS_UADDW2] = UOP_IADD;
  m_int_uop_table[ARM64_INS_UADDW] = UOP_IADD;
  m_int_uop_table[ARM64_INS_UBFM] = UOP_BYTE;
  m_int_uop_table[ARM64_INS_UCVTF] = UOP_FCVT;
  m_int_uop_table[ARM64_INS_UDIV] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_UHADD] = UOP_IADD;
  m_int_uop_table[ARM64_INS_UHSUB] = UOP_IADD;
  m_int_uop_table[ARM64_INS_UMADDL] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_UMAXP] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_UMAXV] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_UMAX] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_UMINP] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_UMINV] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_UMIN] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_UMLAL2] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_UMLAL] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_UMLSL2] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_UMLSL] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_UMOV] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_UMSUBL] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_UMULH] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_UMULL2] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_UMULL] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_UQADD] = UOP_IADD;
  m_int_uop_table[ARM64_INS_UQRSHL] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_UQRSHRN] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_UQRSHRN2] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_UQSHL] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_UQSHRN] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_UQSHRN2] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_UQSUB] = UOP_IADD;
  m_int_uop_table[ARM64_INS_UQXTN2] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_UQXTN] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_URECPE] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_URHADD] = UOP_IADD;
  m_int_uop_table[ARM64_INS_URSHL] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_URSHR] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_URSQRTE] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_URSRA] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_USHLL2] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_USHLL] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_USHL] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_USHR] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_USQADD] = UOP_IADD;
  m_int_uop_table[ARM64_INS_USRA] = UOP_SHIFT;
  m_int_uop_table[ARM64_INS_USUBL2] = UOP_IADD;
  m_int_uop_table[ARM64_INS_USUBL] = UOP_IADD;
  m_int_uop_table[ARM64_INS_USUBW2] = UOP_IADD;
  m_int_uop_table[ARM64_INS_USUBW] = UOP_IADD;
  m_int_uop_table[ARM64_INS_UZP1] = UOP_SSE;
  m_int_uop_table[ARM64_INS_UZP2] = UOP_SSE;
  m_int_uop_table[ARM64_INS_XTN2] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_XTN] = UOP_IADD;
  m_int_uop_table[ARM64_INS_ZIP1] = UOP_SSE;
  m_int_uop_table[ARM64_INS_ZIP2] = UOP_SSE;

  // alias insn
  m_int_uop_table[ARM64_INS_MNEG] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_UMNEGL] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_SMNEGL] = UOP_IMUL;
  m_int_uop_table[ARM64_INS_NOP] = UOP_NOP;
  m_int_uop_table[ARM64_INS_YIELD] = UOP_NOP;
  m_int_uop_table[ARM64_INS_WFE] = UOP_NOP;
  m_int_uop_table[ARM64_INS_WFI] = UOP_NOP;
  m_int_uop_table[ARM64_INS_SEV] = UOP_NOP;
  m_int_uop_table[ARM64_INS_SEVL] = UOP_NOP;
  m_int_uop_table[ARM64_INS_NGC] = UOP_IADD;
  m_int_uop_table[ARM64_INS_SBFIZ] = UOP_BYTE;
  m_int_uop_table[ARM64_INS_UBFIZ] = UOP_BYTE;
  m_int_uop_table[ARM64_INS_SBFX] = UOP_BYTE;
  m_int_uop_table[ARM64_INS_UBFX] = UOP_BYTE;
  m_int_uop_table[ARM64_INS_BFI] = UOP_BYTE;
  m_int_uop_table[ARM64_INS_BFXIL] = UOP_BYTE;
  m_int_uop_table[ARM64_INS_CMN] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_MVN] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_TST] = UOP_ICMP;
  m_int_uop_table[ARM64_INS_CSET] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_CINC] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_CSETM] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_CINV] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_CNEG] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SXTB] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SXTH] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_SXTW] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_CMP] = UOP_ICMP;
  m_int_uop_table[ARM64_INS_UXTB] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_UXTH] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_UXTW] = UOP_LOGIC;
  m_int_uop_table[ARM64_INS_IC] = UOP_NOP;
  m_int_uop_table[ARM64_INS_DC] = UOP_NOP;
  m_int_uop_table[ARM64_INS_AT] = UOP_NOP;
  m_int_uop_table[ARM64_INS_TLBI] = UOP_NOP;

  m_fp_uop_table[ARM64_INS_ABS] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_ADC] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_ADDHN] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_ADDHN2] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_ADDP] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_ADD] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_ADDV] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_ADR] = UOP_LD;
  m_fp_uop_table[ARM64_INS_ADRP] = UOP_LD;
  m_fp_uop_table[ARM64_INS_AESD] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_AESE] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_AESIMC] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_AESMC] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_AND] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_ASR] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_B] = UOP_FCF;
  m_fp_uop_table[ARM64_INS_BFM] = UOP_FBIT;
  m_fp_uop_table[ARM64_INS_BIC] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_BIF] = UOP_FBIT;
  m_fp_uop_table[ARM64_INS_BIT] = UOP_FBIT;
  m_fp_uop_table[ARM64_INS_BL] = UOP_FCF;
  m_fp_uop_table[ARM64_INS_BLR] = UOP_FCF;
  m_fp_uop_table[ARM64_INS_BR] = UOP_FCF;
  m_fp_uop_table[ARM64_INS_BRK] = UOP_FCF;
  m_fp_uop_table[ARM64_INS_BSL] = UOP_FBIT;
  m_fp_uop_table[ARM64_INS_CBNZ] = UOP_FCF;
  m_fp_uop_table[ARM64_INS_CBZ] = UOP_FCF;
  m_fp_uop_table[ARM64_INS_CCMN] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_CCMP] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_CLREX] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_CLS] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_CLZ] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_CMEQ] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_CMGE] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_CMGT] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_CMHI] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_CMHS] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_CMLE] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_CMLT] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_CMTST] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_CNT] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_MOV] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_CRC32B] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_CRC32CB] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_CRC32CH] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_CRC32CW] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_CRC32CX] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_CRC32H] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_CRC32W] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_CRC32X] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_CSEL] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_CSINC] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_CSINV] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_CSNEG] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_DCPS1] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_DCPS2] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_DCPS3] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_DMB] = UOP_FULL_FENCE;
  m_fp_uop_table[ARM64_INS_DRPS] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_DSB] = UOP_FULL_FENCE;
  m_fp_uop_table[ARM64_INS_DUP] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_EON] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_EOR] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_ERET] = UOP_FCF;
  m_fp_uop_table[ARM64_INS_EXTR] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_EXT] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_FABD] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FABS] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FACGE] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_FACGT] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_FADD] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FADDP] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FCCMP] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_FCCMPE] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_FCMEQ] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_FCMGE] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_FCMGT] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_FCMLE] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_FCMLT] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_FCMP] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_FCMPE] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_FCSEL] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_FCVTAS] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_FCVTAU] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_FCVT] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_FCVTL] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_FCVTL2] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_FCVTMS] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_FCVTMU] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_FCVTNS] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_FCVTNU] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_FCVTN] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_FCVTN2] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_FCVTPS] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_FCVTPU] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_FCVTXN] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_FCVTXN2] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_FCVTZS] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_FCVTZU] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_FDIV] = UOP_FDIV;
  m_fp_uop_table[ARM64_INS_FMADD] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FMAX] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FMAXNM] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FMAXNMP] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FMAXNMV] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FMAXP] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FMAXV] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FMIN] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FMINNM] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FMINNMP] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FMINNMV] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FMINP] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FMINV] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FMLA] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_FMLS] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_FMOV] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FMSUB] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_FMUL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_FMULX] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_FNEG] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FNMADD] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_FNMSUB] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_FNMUL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_FRECPE] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FRECPS] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FRECPX] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FRINTA] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FRINTI] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FRINTM] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FRINTN] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FRINTP] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FRINTX] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FRINTZ] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_FRSQRTE] = UOP_FDIV;
  m_fp_uop_table[ARM64_INS_FRSQRTS] = UOP_FDIV;
  m_fp_uop_table[ARM64_INS_FSQRT] = UOP_FDIV;
  m_fp_uop_table[ARM64_INS_FSUB] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_HINT] = UOP_NOP;
  m_fp_uop_table[ARM64_INS_HLT] = UOP_FCF;
  m_fp_uop_table[ARM64_INS_HVC] = UOP_FCF;
  m_fp_uop_table[ARM64_INS_INS] = UOP_LOGIC;

  m_fp_uop_table[ARM64_INS_ISB] = UOP_NOP;
  m_fp_uop_table[ARM64_INS_LD1] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LD1R] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LD2R] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LD2] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LD3R] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LD3] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LD4] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LD4R] = UOP_LD;

  m_fp_uop_table[ARM64_INS_LDARB] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LDARH] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LDAR] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LDAXP] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LDAXRB] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LDAXRH] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LDAXR] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LDNP] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LDP] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LDPSW] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LDRB] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LDR] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LDRH] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LDRSB] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LDRSH] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LDRSW] = UOP_LD;
  m_fp_uop_table[ARM64_INS_LDTRB] = UOP_LDA;
  m_fp_uop_table[ARM64_INS_LDTRH] = UOP_LDA;
  m_fp_uop_table[ARM64_INS_LDTRSB] = UOP_LDA;

  m_fp_uop_table[ARM64_INS_LDTRSH] = UOP_LDA;
  m_fp_uop_table[ARM64_INS_LDTRSW] = UOP_LDA;
  m_fp_uop_table[ARM64_INS_LDTR] = UOP_LDA;
  m_fp_uop_table[ARM64_INS_LDURB] = UOP_LDA;
  m_fp_uop_table[ARM64_INS_LDUR] = UOP_LDA;
  m_fp_uop_table[ARM64_INS_LDURH] = UOP_LDA;
  m_fp_uop_table[ARM64_INS_LDURSB] = UOP_LDA;
  m_fp_uop_table[ARM64_INS_LDURSH] = UOP_LDA;
  m_fp_uop_table[ARM64_INS_LDURSW] = UOP_LDA;
  m_fp_uop_table[ARM64_INS_LDXP] = UOP_LDA;
  m_fp_uop_table[ARM64_INS_LDXRB] = UOP_LDA;
  m_fp_uop_table[ARM64_INS_LDXRH] = UOP_LDA;
  m_fp_uop_table[ARM64_INS_LDXR] = UOP_LDA;
  m_fp_uop_table[ARM64_INS_LSL] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_LSR] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_MADD] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_MLA] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_MLS] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_MOVI] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_MOVK] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_MOVN] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_MOVZ] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_MRS] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_MSR] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_MSUB] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_MUL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_MVNI] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_NEG] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_NOT] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_ORN] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_ORR] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_PMULL2] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_PMULL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_PMUL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_PRFM] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_PRFUM] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_RADDHN] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_RADDHN2] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_RBIT] = UOP_FBIT;
  m_fp_uop_table[ARM64_INS_RET] = UOP_FCF;
  m_fp_uop_table[ARM64_INS_REV16] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_REV32] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_REV64] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_REV] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_ROR] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_RSHRN2] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_RSHRN] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_RSUBHN] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_RSUBHN2] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SABAL2] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SABAL] = UOP_LOGIC;

  m_fp_uop_table[ARM64_INS_SABA] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SABDL2] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SABDL] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SABD] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SADALP] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SADDLP] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SADDLV] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SADDL2] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SADDL] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SADDW2] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SADDW] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SBC] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SBFM] = UOP_FBIT;
  m_fp_uop_table[ARM64_INS_SCVTF] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_SDIV] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SHA1C] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_SHA1H] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_SHA1M] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_SHA1P] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_SHA1SU0] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_SHA1SU1] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_SHA256H2] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_SHA256H] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_SHA256SU0] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_SHA256SU1] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_SHADD] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SHLL2] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SHLL] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SHL] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SHRN2] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SHRN] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SHSUB] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SLI] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SMADDL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SMAXP] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SMAXV] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SMAX] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SMC] = UOP_FCF;
  m_fp_uop_table[ARM64_INS_SMINP] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SMINV] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SMIN] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SMLAL2] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SMLAL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SMLSL2] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SMLSL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SMOV] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SMSUBL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SMULH] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SMULL2] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SMULL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SQABS] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SQADD] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SQDMLAL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SQDMLAL2] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SQDMLSL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SQDMLSL2] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SQDMULH] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SQDMULL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SQDMULL2] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SQNEG] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SQRDMULH] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SQRSHL] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SQRSHRN] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SQRSHRN2] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SQRSHRUN] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SQRSHRUN2] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SQSHLU] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SQSHL] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SQSHRN] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SQSHRN2] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SQSHRUN] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SQSHRUN2] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SQSUB] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SQXTN2] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SQXTN] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SQXTUN2] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SQXTUN] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SRHADD] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SRI] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SRSHL] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SRSHR] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SRSRA] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SSHLL2] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SSHLL] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SSHL] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SSHR] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SSRA] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_SSUBL2] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SSUBL] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SSUBW2] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SSUBW] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_ST1] = UOP_ST;
  m_fp_uop_table[ARM64_INS_ST2] = UOP_ST;
  m_fp_uop_table[ARM64_INS_ST3] = UOP_ST;
  m_fp_uop_table[ARM64_INS_ST4] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STLRB] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STLRH] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STLR] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STLXP] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STLXRB] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STLXRH] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STLXR] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STNP] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STP] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STRB] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STR] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STRH] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STTRB] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STTRH] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STTR] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STURB] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STUR] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STURH] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STXP] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STXRB] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STXRH] = UOP_ST;
  m_fp_uop_table[ARM64_INS_STXR] = UOP_ST;
  m_fp_uop_table[ARM64_INS_SUBHN] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SUBHN2] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SUB] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SUQADD] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SVC] = UOP_FCF;
  m_fp_uop_table[ARM64_INS_SYSL] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SYS] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_TBL] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_TBNZ] = UOP_FCF;
  m_fp_uop_table[ARM64_INS_TBX] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_TBZ] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_TRN1] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_TRN2] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_UABAL2] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_UABAL] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_UABA] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_UABDL2] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_UABDL] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_UABD] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_UADALP] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_UADDLP] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_UADDLV] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_UADDL2] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_UADDL] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_UADDW2] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_UADDW] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_UBFM] = UOP_FBIT;
  m_fp_uop_table[ARM64_INS_UCVTF] = UOP_FCVT;
  m_fp_uop_table[ARM64_INS_UDIV] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_UHADD] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_UHSUB] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_UMADDL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_UMAXP] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_UMAXV] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_UMAX] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_UMINP] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_UMINV] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_UMIN] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_UMLAL2] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_UMLAL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_UMLSL2] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_UMLSL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_UMOV] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_UMSUBL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_UMULH] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_UMULL2] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_UMULL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_UQADD] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_UQRSHL] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_UQRSHRN] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_UQRSHRN2] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_UQSHL] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_UQSHRN] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_UQSHRN2] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_UQSUB] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_UQXTN2] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_UQXTN] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_URECPE] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_URHADD] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_URSHL] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_URSHR] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_URSQRTE] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_URSRA] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_USHLL2] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_USHLL] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_USHL] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_USHR] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_USQADD] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_USRA] = UOP_SHIFT;
  m_fp_uop_table[ARM64_INS_USUBL2] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_USUBL] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_USUBW2] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_USUBW] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_UZP1] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_UZP2] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_XTN2] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_XTN] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_ZIP1] = UOP_SSE;
  m_fp_uop_table[ARM64_INS_ZIP2] = UOP_SSE;

  // alias insn
  m_fp_uop_table[ARM64_INS_MNEG] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_UMNEGL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_SMNEGL] = UOP_FMUL;
  m_fp_uop_table[ARM64_INS_NOP] = UOP_NOP;
  m_fp_uop_table[ARM64_INS_YIELD] = UOP_NOP;
  m_fp_uop_table[ARM64_INS_WFE] = UOP_NOP;
  m_fp_uop_table[ARM64_INS_WFI] = UOP_NOP;
  m_fp_uop_table[ARM64_INS_SEV] = UOP_NOP;
  m_fp_uop_table[ARM64_INS_SEVL] = UOP_NOP;
  m_fp_uop_table[ARM64_INS_NGC] = UOP_FADD;
  m_fp_uop_table[ARM64_INS_SBFIZ] = UOP_FBIT;
  m_fp_uop_table[ARM64_INS_UBFIZ] = UOP_FBIT;
  m_fp_uop_table[ARM64_INS_SBFX] = UOP_FBIT;
  m_fp_uop_table[ARM64_INS_UBFX] = UOP_FBIT;
  m_fp_uop_table[ARM64_INS_BFI] = UOP_FBIT;
  m_fp_uop_table[ARM64_INS_BFXIL] = UOP_FBIT;
  m_fp_uop_table[ARM64_INS_CMN] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_MVN] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_TST] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_CSET] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_CINC] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_CSETM] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_CINV] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_CNEG] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SXTB] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SXTH] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_SXTW] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_CMP] = UOP_FCMP;
  m_fp_uop_table[ARM64_INS_UXTB] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_UXTH] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_UXTW] = UOP_LOGIC;
  m_fp_uop_table[ARM64_INS_IC] = UOP_NOP;
  m_fp_uop_table[ARM64_INS_DC] = UOP_NOP;
  m_fp_uop_table[ARM64_INS_AT] = UOP_NOP;
  m_fp_uop_table[ARM64_INS_TLBI] = UOP_NOP;
}

inst_info_s* a64_decoder_c::get_inst_info(thread_s *thread_trace_info, int core_id, int sim_thread_id)
{
  trace_info_a64_s trace_info;
  inst_info_s *info;
  // Copy current instruction to data structure
  memcpy(&trace_info, thread_trace_info->m_prev_trace_info, sizeof(trace_info_a64_s));

  // Set next pc address
  trace_info_a64_s *next_trace_info = static_cast<trace_info_a64_s *>(thread_trace_info->m_next_trace_info);
  trace_info.m_instruction_next_addr = next_trace_info->m_instruction_addr;

  // Copy next instruction to current instruction field
  memcpy(thread_trace_info->m_prev_trace_info, thread_trace_info->m_next_trace_info, 
         sizeof(trace_info_a64_s));

  DEBUG_CORE(core_id, "trace_read core_id:%d thread_id:%d pc:0x%llx opcode:%d inst_count:%llu\n", core_id, sim_thread_id, 
             (Addr)(trace_info.m_instruction_addr), static_cast<int>(trace_info.m_opcode), (Counter)(thread_trace_info->m_temp_inst_count));

  // So far we have raw instruction format, so we need to MacSim specific trace format
  info = convert_pinuop_to_t_uop(&trace_info, thread_trace_info->m_trace_uop_array, 
                                 core_id, sim_thread_id);

  return info;
}

inst_info_s* a64_decoder_c::convert_pinuop_to_t_uop(void *trace_info, trace_uop_s **trace_uop, 
                                                    int core_id, int sim_thread_id)
{
  trace_info_a64_s *pi = static_cast<trace_info_a64_s *>(trace_info); 
  core_c* core = m_simBase->m_core_pointers[core_id];

  // simulator maintains a cache of decoded instructions (uop) for each process, 
  // this avoids decoding of instructions everytime an instruction is executed
  int process_id = core->get_trace_info(sim_thread_id)->m_process->m_process_id;
  hash_c<inst_info_s>* htable = m_simBase->m_inst_info_hash[process_id];

  // since each instruction can be decoded into multiple uops, the key to the 
  // hashtable has to be (instruction addr + something else)
  // the instruction addr is shifted left by 3-bits and the number of the uop 
  // in the decoded sequence is added to the shifted value to obtain the key
  bool new_entry = false;
  Addr key_addr = (pi->m_instruction_addr << 3);

  // Get instruction information from the hash table if exists. 
  // Else create a new entry
  inst_info_s *info = htable->hash_table_access_create(key_addr, &new_entry);

  inst_info_s *first_info = info;
  int  num_uop = 0;
  int  dyn_uop_counter = 0;
  bool tmp_reg_needed = false;
  bool inst_has_ALU_uop = false;
  bool inst_has_ld_uop = false;
  int  ii, jj, kk;

  ASSERT(pi->m_opcode != ARM64_INS_INVALID);
  if (new_entry) {
    // Since we found a new instruction, we need to decode this instruction and store all
    // uops to the hash table
    int write_dest_reg = 0;

    trace_uop[0]->m_rep_uop_num = 0;
    trace_uop[0]->m_opcode = pi->m_opcode;

    // temporal register rules:
    // load->dest_reg (through tmp), load->store (through tmp), dest_reg->store (real reg)
    // load->cf (through tmp), dest_reg->cf (thought dest), st->cf (no dependency)
    

    ///
    /// 1. This instruction has a memory load operation
    ///
    if (pi->m_num_ld) {
      num_uop = 1;

      trace_uop[0]->m_mem_type = MEM_LD;

      // prefetch instruction
      if (pi->m_opcode == ARM64_INS_PRFM || pi->m_opcode == ARM64_INS_PRFUM)  {
        arm64_prefetch_op pf = (arm64_prefetch_op)pi->m_st_vaddr;
        switch (pf) {
        case ARM64_PRFM_PLDL1STRM:
        case ARM64_PRFM_PLDL2STRM:
        case ARM64_PRFM_PLDL3STRM:
        case ARM64_PRFM_PSTL1STRM:
        case ARM64_PRFM_PSTL2STRM:
          trace_uop[0]->m_mem_type = MEM_SWPREF_NTA;
          break;

        case ARM64_PRFM_PLDL1KEEP:
        case ARM64_PRFM_PSTL1KEEP:
          trace_uop[0]->m_mem_type = MEM_SWPREF_T0;
          break;

        case ARM64_PRFM_PLDL2KEEP:
        case ARM64_PRFM_PSTL2KEEP:
          trace_uop[0]->m_mem_type = MEM_SWPREF_T1;
          break;

        case ARM64_PRFM_PLDL3KEEP:
        case ARM64_PRFM_PSTL3KEEP:
          trace_uop[0]->m_mem_type = MEM_SWPREF_T2;
          break;
        }
      }

      trace_uop[0]->m_cf_type  = NOT_CF;
      trace_uop[0]->m_op_type  = (pi->m_is_fp) ? UOP_FMEM : UOP_IMEM;
      trace_uop[0]->m_bar_type = NOT_BAR;

      trace_uop[0]->m_num_src_regs = pi->m_num_read_regs;
      trace_uop[0]->m_num_dest_regs = pi->m_num_dest_regs;

      trace_uop[0]->m_pin_2nd_mem  = 0;
      trace_uop[0]->m_eom      = 0;
      trace_uop[0]->m_alu_uop  = false;
      trace_uop[0]->m_inst_size = pi->m_size;

      ///
      /// There are two load operations in an instruction. Note that now array index becomes 1
      ///
      if (pi->m_num_ld == 2) {
        trace_uop[1]->m_opcode = pi->m_opcode;

        if (pi->m_opcode == ARM64_INS_PRFM || pi->m_opcode == ARM64_INS_PRFUM) {
          trace_uop[1]->m_mem_type = MEM_PF;
          arm64_prefetch_op pf = (arm64_prefetch_op)pi->m_st_vaddr;
          switch (pf) {
          case ARM64_PRFM_PLDL1STRM:
          case ARM64_PRFM_PLDL2STRM:
          case ARM64_PRFM_PLDL3STRM:
          case ARM64_PRFM_PSTL1STRM:
          case ARM64_PRFM_PSTL2STRM:
            trace_uop[1]->m_mem_type = MEM_SWPREF_NTA;
            break;

          case ARM64_PRFM_PLDL1KEEP:
          case ARM64_PRFM_PSTL1KEEP:
            trace_uop[1]->m_mem_type = MEM_SWPREF_T0;
            break;

          case ARM64_PRFM_PLDL2KEEP:
          case ARM64_PRFM_PSTL2KEEP:
            trace_uop[1]->m_mem_type = MEM_SWPREF_T1;
            break;

          case ARM64_PRFM_PLDL3KEEP:
          case ARM64_PRFM_PSTL3KEEP:
            trace_uop[1]->m_mem_type = MEM_SWPREF_T2;
            break;
          }
        } else {
          trace_uop[1]->m_mem_type = MEM_LD;
        }
        trace_uop[1]->m_cf_type    = NOT_CF;
        trace_uop[1]->m_op_type    = (pi->m_is_fp) ? UOP_FMEM : UOP_IMEM;
        trace_uop[1]->m_bar_type   = NOT_BAR;
        trace_uop[1]->m_num_dest_regs = 0;
        trace_uop[1]->m_num_src_regs = pi->m_num_read_regs;
        trace_uop[1]->m_num_dest_regs = pi->m_num_dest_regs;

        trace_uop[1]->m_pin_2nd_mem  = 1;
        trace_uop[1]->m_eom          = 0;
        trace_uop[1]->m_alu_uop      = false;
        trace_uop[1]->m_inst_size    = pi->m_size;
        trace_uop[1]->m_mul_mem_uops = 0; //pi->m_has_immediate; // uncoalesced memory accesses

        num_uop = 2;
      } // num_loads == 2

      // for arm64 we do not generate multiple uops based on has_immediate
      // m_has_immediate is overloaded - in case of ptx simulations, for uncoalesced 
      // accesses, multiple memory access are generated and for each access there is
      // an instruction in the trace. the m_has_immediate flag is used to mark the
      // first and last accesses of an uncoalesced memory instruction
      trace_uop[0]->m_mul_mem_uops = 0; //pi->m_has_immediate;

      // we do not need temporary registers in ARM64 for loads
      write_dest_reg = 1;

      if (trace_uop[0]->m_mem_type == MEM_LD) {
        inst_has_ld_uop = true;
      }
    } // HAS_LOAD

    // load acquire: create a new acquire UOP fence
    if (num_uop == 1 && (pi->m_opcode == ARM64_INS_LDAXR  ||
                         pi->m_opcode == ARM64_INS_LDAXRB ||
                         pi->m_opcode == ARM64_INS_LDAXRH ||
                         pi->m_opcode == ARM64_INS_LDAR   ||
                         pi->m_opcode == ARM64_INS_LDARB  ||
                         pi->m_opcode == ARM64_INS_LDARH))
    {
      trace_uop[1]->m_opcode        = pi->m_opcode;
      trace_uop[1]->m_mem_type      = NOT_MEM;
      trace_uop[1]->m_cf_type       = NOT_CF;
      trace_uop[1]->m_op_type       = UOP_ACQ_FENCE;
      trace_uop[1]->m_bar_type      = NOT_BAR;
      trace_uop[1]->m_num_dest_regs = 0;
      trace_uop[1]->m_num_src_regs  = 0;
      trace_uop[1]->m_pin_2nd_mem   = 0;
      trace_uop[1]->m_eom           = 1;
      trace_uop[1]->m_inst_size     = pi->m_size;
      ++num_uop;
    }

    // store release: create a new release UOP fence
    if (num_uop == 0 && (pi->m_opcode == ARM64_INS_STLXR  ||
                         pi->m_opcode == ARM64_INS_STLXRB ||
                         pi->m_opcode == ARM64_INS_STLXRH ||
                         pi->m_opcode == ARM64_INS_STLR   ||
                         pi->m_opcode == ARM64_INS_STLRB  ||
                         pi->m_opcode == ARM64_INS_STLRH))
    {
      trace_uop[0]->m_opcode        = pi->m_opcode;
      trace_uop[0]->m_mem_type      = NOT_MEM;
      trace_uop[0]->m_cf_type       = NOT_CF;
      trace_uop[0]->m_op_type       = UOP_REL_FENCE;
      trace_uop[0]->m_bar_type      = NOT_BAR;
      trace_uop[0]->m_num_dest_regs = 0;
      trace_uop[0]->m_num_src_regs  = 0;
      trace_uop[0]->m_pin_2nd_mem   = 0;
      trace_uop[0]->m_eom           = 0;
      trace_uop[0]->m_inst_size     = pi->m_size;
      trace_uop[0]->m_mem_size      = 0;
      ++num_uop;
    }

    /*
    // Add one more uop when temporary register is required
    if (pi->m_num_dest_regs && !write_dest_reg) {
      trace_uop_s* cur_trace_uop = trace_uop[num_uop++];
      if (inst_has_ld_uop) {
        tmp_reg_needed = true;
      }

      cur_trace_uop->m_opcode        = pi->m_opcode;
      cur_trace_uop->m_mem_type      = NOT_MEM;
      cur_trace_uop->m_cf_type       = NOT_CF;
      cur_trace_uop->m_op_type       = (Uop_Type)((pi->m_is_fp) ?
                                                  m_fp_uop_table[pi->m_opcode] :
                                                  m_int_uop_table[pi->m_opcode]);
      cur_trace_uop->m_bar_type      = NOT_BAR;
      cur_trace_uop->m_num_src_regs  = pi->m_num_read_regs;
      cur_trace_uop->m_num_dest_regs = pi->m_num_dest_regs;
      cur_trace_uop->m_pin_2nd_mem   = 0;
      cur_trace_uop->m_eom           = 0;
      cur_trace_uop->m_alu_uop       = true;

      inst_has_ALU_uop = true;
    }
    */

    ///
    /// 2. Instruction has a memory store operation
    ///
    if (pi->m_has_st) {
      trace_uop_s* cur_trace_uop = trace_uop[num_uop++];
      if (inst_has_ld_uop)
        tmp_reg_needed = true;
   
      cur_trace_uop->m_mem_type      = MEM_ST;
      cur_trace_uop->m_opcode        = pi->m_opcode;
      cur_trace_uop->m_cf_type       = NOT_CF;
      cur_trace_uop->m_op_type       = (pi->m_is_fp) ? UOP_FMEM : UOP_IMEM;
      if (pi->m_opcode == ARM64_INS_STLXR  ||
          pi->m_opcode == ARM64_INS_STLXRB ||
          pi->m_opcode == ARM64_INS_STLXRH ||
          pi->m_opcode == ARM64_INS_STLR   ||
          pi->m_opcode == ARM64_INS_STLRB  ||
          pi->m_opcode == ARM64_INS_STLRH)
        cur_trace_uop->m_bar_type      = REL_BAR;
      else
        cur_trace_uop->m_bar_type      = NOT_BAR;
      cur_trace_uop->m_num_src_regs  = pi->m_num_read_regs;
      cur_trace_uop->m_num_dest_regs = 0;
      cur_trace_uop->m_pin_2nd_mem   = 0;
      cur_trace_uop->m_eom           = 0;
      cur_trace_uop->m_alu_uop       = false;
      cur_trace_uop->m_inst_size     = pi->m_size;
      cur_trace_uop->m_mul_mem_uops  = 0;
    }


    ///
    /// 3. Instruction has a branch operation
    ///
    if (pi->m_cf_type) {
      assert(num_uop == 0);
      trace_uop_s* cur_trace_uop = trace_uop[num_uop++];

      if (inst_has_ld_uop)
        tmp_reg_needed = true;
   
      cur_trace_uop->m_mem_type      = NOT_MEM;
      switch (pi->m_opcode) {
      case ARM64_INS_B:
      case ARM64_INS_BR:
        cur_trace_uop->m_cf_type   = CF_BR;
        break;

      case ARM64_INS_CBNZ:
      case ARM64_INS_CBZ:
      case ARM64_INS_TBNZ:
      case ARM64_INS_TBZ:
        cur_trace_uop->m_cf_type   = CF_CBR;
        break;

      case ARM64_INS_BL:
      case ARM64_INS_BLR:
        cur_trace_uop->m_cf_type   = CF_CALL;
        break;

      case ARM64_INS_BRK:
      case ARM64_INS_HLT:
      case ARM64_INS_HVC:
      case ARM64_INS_SMC:
      case ARM64_INS_SVC:
        cur_trace_uop->m_cf_type   = CF_IBR;
        break;

        /* not currently mapped
           cur_trace_uop->m_cf_type   = CF_ICALL;
           cur_trace_uop->m_cf_type   = CF_ICO;
        */
      case ARM64_INS_RET:
      case ARM64_INS_ERET:
        cur_trace_uop->m_cf_type   = CF_RET;
        break;
      }
      cur_trace_uop->m_op_type       = UOP_CF;
      cur_trace_uop->m_bar_type      = NOT_BAR;
      cur_trace_uop->m_num_src_regs  = pi->m_num_read_regs;
      cur_trace_uop->m_num_dest_regs = 0;
      cur_trace_uop->m_pin_2nd_mem   = 0;
      cur_trace_uop->m_eom           = 0;
      cur_trace_uop->m_alu_uop       = false;
      cur_trace_uop->m_inst_size     = pi->m_size;
    }

    // Fence instruction: m_opcode == MISC && actually_taken == 1
    if (num_uop == 0 && (pi->m_opcode == ARM64_INS_DMB ||
                         pi->m_opcode == ARM64_INS_DSB))
    {
      trace_uop[0]->m_opcode        = pi->m_opcode;
      trace_uop[0]->m_mem_type      = NOT_MEM;
      trace_uop[0]->m_cf_type       = NOT_CF;
      if (pi->m_st_vaddr == ARM64_BARRIER_ISHLD ||
          pi->m_st_vaddr == ARM64_BARRIER_OSHLD ||
          pi->m_st_vaddr == ARM64_BARRIER_LD)
        trace_uop[0]->m_op_type       = UOP_LFENCE;
      else
        trace_uop[0]->m_op_type       = UOP_FULL_FENCE;

      trace_uop[0]->m_bar_type      = NOT_BAR;
      trace_uop[0]->m_num_dest_regs = 0;
      trace_uop[0]->m_num_src_regs  = 0;
      trace_uop[0]->m_pin_2nd_mem   = 0;
      trace_uop[0]->m_eom           = 1;
      trace_uop[0]->m_inst_size     = pi->m_size;
      ++num_uop;
    }

    ///
    /// Non-memory, non-branch instruction
    ///
    if (num_uop == 0) {
      trace_uop[0]->m_opcode        = pi->m_opcode;
      trace_uop[0]->m_mem_type      = NOT_MEM;
      trace_uop[0]->m_cf_type       = NOT_CF;
      trace_uop[0]->m_op_type       = (Uop_Type)((pi->m_is_fp) ?
                                                  m_fp_uop_table[pi->m_opcode] :
                                                  m_int_uop_table[pi->m_opcode]);
      trace_uop[0]->m_bar_type      = NOT_BAR;
      trace_uop[0]->m_num_dest_regs = 0;
      trace_uop[0]->m_num_src_regs  = 0;
      trace_uop[0]->m_pin_2nd_mem   = 0;
      trace_uop[0]->m_eom           = 1;
      trace_uop[0]->m_inst_size     = pi->m_size;
      ++num_uop;
    }

    info->m_trace_info.m_bom     = true;
    info->m_trace_info.m_eom     = false;
    info->m_trace_info.m_num_uop = num_uop;


    ///
    /// Process each static uop to dynamic uop
    ///
    for (ii = 0; ii < num_uop; ++ii) {
      // For the first uop, we have already created hash entry. However, for following uops
      // we need to create hash entries
      if (ii > 0) {
        key_addr                 = ((pi->m_instruction_addr << 3) + ii);
        info                     = htable->hash_table_access_create(key_addr, &new_entry);
        info->m_trace_info.m_bom = false;
        info->m_trace_info.m_eom = false;
      }
      ASSERTM(new_entry, "Add new uops to hash_table for core id::%d\n", core_id);

      trace_uop[ii]->m_addr = pi->m_instruction_addr;

      DEBUG_CORE(core_id, "pi->instruction_addr:0x%llx trace_uop[%d]->addr:0x%llx num_src_regs:%d num_read_regs:%d "
                 "pi:num_dst_regs:%d uop:num_dst_regs:%d \n", (Addr)(pi->m_instruction_addr), ii, trace_uop[ii]->m_addr,
                 trace_uop[ii]->m_num_src_regs, pi->m_num_read_regs, pi->m_num_dest_regs, trace_uop[ii]->m_num_dest_regs);

      // set source register
      for (jj = 0; jj < trace_uop[ii]->m_num_src_regs; ++jj) {
        (trace_uop[ii])->m_srcs[jj].m_type = (Reg_Type)0;
        (trace_uop[ii])->m_srcs[jj].m_id   = pi->m_src[jj];
        (trace_uop[ii])->m_srcs[jj].m_reg  = pi->m_src[jj];
      }

      // store or control flow has a dependency whoever the last one
      if ((trace_uop[ii]->m_mem_type == MEM_ST) || 
          (trace_uop[ii]->m_cf_type != NOT_CF)) {

        if (tmp_reg_needed && !inst_has_ALU_uop) {
          (trace_uop[ii])->m_srcs[jj].m_type = (Reg_Type)0;
          (trace_uop[ii])->m_srcs[jj].m_id  = TR_REG_TMP0;
          (trace_uop[ii])->m_srcs[jj].m_reg  = TR_REG_TMP0;
          trace_uop[ii]->m_num_src_regs += 1;
        } 
        else if (inst_has_ALU_uop) {
          for (kk = 0; kk < pi->m_num_dest_regs; ++kk) {
            (trace_uop[ii])->m_srcs[jj+kk].m_type = (Reg_Type)0;
            (trace_uop[ii])->m_srcs[jj+kk].m_id   = pi->m_dst[kk];
            (trace_uop[ii])->m_srcs[jj+kk].m_reg  = pi->m_dst[kk];
          }

          trace_uop[ii]->m_num_src_regs += pi->m_num_dest_regs;
        }
      }

      // alu uop only has a dependency with a temp register
      if (trace_uop[ii]->m_alu_uop) {
        if (tmp_reg_needed) {
          (trace_uop[ii])->m_srcs[jj].m_type = (Reg_Type)0;
          (trace_uop[ii])->m_srcs[jj].m_id  = TR_REG_TMP0;
          (trace_uop[ii])->m_srcs[jj].m_reg  = TR_REG_TMP0;
          trace_uop[ii]->m_num_src_regs     += 1;
        }
      }

      for (jj = 0; jj < trace_uop[ii]->m_num_dest_regs; ++jj) {
        (trace_uop[ii])->m_dests[jj].m_type = (Reg_Type)0;
        (trace_uop[ii])->m_dests[jj].m_id   = pi->m_dst[jj];
        (trace_uop[ii])->m_dests[jj].m_reg = pi->m_dst[jj];
      }

      // add tmp register as a destination register
      if (tmp_reg_needed && trace_uop[ii]->m_mem_type == MEM_LD) { 
        (trace_uop[ii])->m_dests[jj].m_type = (Reg_Type)0;
        (trace_uop[ii])->m_dests[jj].m_id   = TR_REG_TMP0;
        (trace_uop[ii])->m_dests[jj].m_reg = TR_REG_TMP0;
        trace_uop[ii]->m_num_dest_regs     += 1;
      }

      // update instruction information with MacSim trace
      convert_t_uop_to_info(trace_uop[ii], info);

      DEBUG_CORE(core_id, "tuop: pc 0x%llx num_src_reg:%d num_dest_reg:%d \n", trace_uop[ii]->m_addr, 
                 trace_uop[ii]->m_num_src_regs, trace_uop[ii]->m_num_dest_regs);
      
      trace_uop[ii]->m_info = info;

      // Add dynamic information to the uop
      convert_dyn_uop(info, pi, trace_uop[ii], 0, core_id);
    }
  
    // set end of macro flag to the last uop
    trace_uop[num_uop - 1]->m_eom = 1;

    ASSERT(num_uop > 0);
  } // NEW_ENTRY
    ///
    /// Hash table already has matching instruction, we can skip above decoding process
    ///
  else {
    ASSERT(info);

    num_uop = info->m_trace_info.m_num_uop;
    for (ii = 0; ii < num_uop; ++ii) {
      if (ii > 0) {
        key_addr  = ((pi->m_instruction_addr << 3) + ii);
        info      = htable->hash_table_access_create(key_addr, &new_entry);
      }
      ASSERTM(!new_entry, "Core id %d index %d\n", core_id, ii);

      // convert raw instruction trace to MacSim trace format
      convert_info_uop(info, trace_uop[ii]);

      // add dynamic information
      convert_dyn_uop(info, pi, trace_uop[ii], 0, core_id);

      trace_uop[ii]->m_info   = info;
      trace_uop[ii]->m_eom    = 0;
      trace_uop[ii]->m_addr   = pi->m_instruction_addr;
      trace_uop[ii]->m_opcode = pi->m_opcode;
    }

    // set end of macro flag to the last uop
    trace_uop[num_uop-1]->m_eom = 1;

    ASSERT(num_uop > 0);
  }

  /////////////////////////////////////////////////////////////////////////////////////////////
  // end of instruction decoding
  /////////////////////////////////////////////////////////////////////////////////////////////

  dyn_uop_counter = num_uop;

  ASSERT(dyn_uop_counter);


  // set eom flag and next pc address for the last uop of this instruction
  trace_uop[dyn_uop_counter-1]->m_eom = 1;
  trace_uop[dyn_uop_counter-1]->m_npc = pi->m_instruction_next_addr;

  ASSERT(num_uop > 0);
  first_info->m_trace_info.m_num_uop = num_uop;

  DEBUG("%s: read: %d write: %d\n", a64_opcode_names[pi->m_opcode], pi->m_num_read_regs, pi->m_num_dest_regs);

  return first_info;
}

void a64_decoder_c::convert_dyn_uop(inst_info_s *info, void *trace_info, trace_uop_s *trace_uop, 
                                    Addr rep_offset, int core_id)
{
  trace_info_a64_s *pi = static_cast<trace_info_a64_s *>(trace_info);
  core_c* core    = m_simBase->m_core_pointers[core_id];
  trace_uop->m_va = 0;

  if (info->m_table_info->m_cf_type) {
    trace_uop->m_actual_taken = pi->m_actually_taken;
    trace_uop->m_target       = pi->m_branch_target;
  } else if (info->m_table_info->m_mem_type) {
    if (info->m_table_info->m_mem_type == MEM_ST) {
      trace_uop->m_va       = pi->m_st_vaddr;
      trace_uop->m_mem_size = pi->m_mem_write_size;
      // In rare cases, if a page fault occurs due to a write you can miss the
      // memory callback because of which m_mem_write_size will be 0.
      // Hack it up using dummy values
      if (pi->m_mem_write_size == 0) {
          trace_uop->m_va       = 0xabadabad;
          trace_uop->m_mem_size = 4;
      }
    } 
    else if ((info->m_table_info->m_mem_type == MEM_LD) || 
             (info->m_table_info->m_mem_type == MEM_PF) ||
             (info->m_table_info->m_mem_type >= MEM_SWPREF_NTA &&
              info->m_table_info->m_mem_type <= MEM_SWPREF_T2)) {
      if (info->m_trace_info.m_second_mem)
        trace_uop->m_va = pi->m_ld_vaddr2;
      else 
        trace_uop->m_va = pi->m_ld_vaddr1;

      if (pi->m_mem_read_size)
        trace_uop->m_mem_size = pi->m_mem_read_size;
    }
  }

  // next pc
  trace_uop->m_npc = trace_uop->m_addr;
}

void a64_decoder_c::dprint_inst(void *trace_info, int core_id, int thread_id)
{
  if (m_dprint_count++ >= 50000 || !*KNOB(KNOB_DEBUG_PRINT_TRACE))
    return ;

  trace_info_a64_s *t_info = static_cast<trace_info_a64_s *>(trace_info);
 
  *m_dprint_output << "*** begin of the data strcture *** " << endl;
  *m_dprint_output << "core_id:" << core_id << " thread_id:" << thread_id << endl;
  *m_dprint_output << "uop_opcode " <<a64_opcode_names[(uint32_t) t_info->m_opcode]  << endl;
  *m_dprint_output << "num_read_regs: " << hex <<  (uint32_t) t_info->m_num_read_regs << endl;
  *m_dprint_output << "num_dest_regs: " << hex << (uint32_t) t_info->m_num_dest_regs << endl;
  /* TODO: Register name mappings
  for (uint32_t ii = 0; ii < (uint32_t) t_info->m_num_read_regs; ++ii)
    *m_dprint_output << "src" << ii << ": " 
      << hex << g_tr_reg_names[static_cast<uint32_t>(t_info->m_src[ii])] << endl;

  for (uint32_t ii = 0; ii < (uint32_t) t_info->m_num_dest_regs; ++ii)
    *m_dprint_output << "dst" << ii << ": " 
      << hex << g_tr_reg_names[static_cast<uint32_t>(t_info->m_dst[ii])] << endl;
  *m_dprint_output << "cf_type: " << hex << g_tr_cf_names[(uint32_t) t_info->m_cf_type] << endl;
  */
  *m_dprint_output << "has_immediate: " << hex << (uint32_t) t_info->m_has_immediate << endl;
  *m_dprint_output << "r_dir:" << (uint32_t) t_info->m_rep_dir << endl;
  *m_dprint_output << "has_st: " << hex << (uint32_t) t_info->m_has_st << endl;
  *m_dprint_output << "num_ld: " << hex << (uint32_t) t_info->m_num_ld << endl;
  *m_dprint_output << "mem_read_size: " << hex << (uint32_t) t_info->m_mem_read_size << endl;
  *m_dprint_output << "mem_write_size: " << hex << (uint32_t) t_info->m_mem_write_size << endl;
  *m_dprint_output << "is_fp: " << (uint32_t) t_info->m_is_fp << endl;
  *m_dprint_output << "ld_vaddr1: " << hex << (uint32_t) t_info->m_ld_vaddr1 << endl;
  *m_dprint_output << "ld_vaddr2: " << hex << (uint32_t) t_info->m_ld_vaddr2 << endl;
  *m_dprint_output << "st_vaddr: " << hex << (uint32_t) t_info->m_st_vaddr << endl;
  *m_dprint_output << "instruction_addr: " << hex << (uint32_t)t_info->m_instruction_addr << endl;
  *m_dprint_output << "branch_target: " << hex << (uint32_t)t_info->m_branch_target << endl;
  *m_dprint_output << "actually_taken: " << hex << (uint32_t)t_info->m_actually_taken << endl;
  *m_dprint_output << "write_flg: " << hex << (uint32_t)t_info->m_write_flg << endl;
  *m_dprint_output << "size: " << hex << (uint32_t) t_info->m_size << endl;
  *m_dprint_output << "*** end of the data strcture *** " << endl << endl;

}
