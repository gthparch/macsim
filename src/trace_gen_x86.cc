#ifdef USING_QSIM

#include "trace_gen_x86.h"

#include <qsim.h>
#include <qsim-load.h>
#include <qsim-x86-regs.h>

#include <set>

extern "C" {
#include "xed-interface.h"
}

enum CPU_OPCODE_enum {
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
  CPU_MEM_EXT_OP,  // To denote that the op is a continuation of the same parent op but has more mem accesses.
  CPU_OPCODE_LAST,
} CPU_OPCODE;

enum CF_TYPE_enum {
  NOT_CF,  // not a control flow instruction
  CF_BR,  // an unconditional branch
  CF_CBR,  // a conditional branch
  CF_CALL,  // a call
  // below this point are indirect cfs
  CF_IBR,  // an indirect branch
  CF_ICALL,  // an indirect call
  CF_ICO,  // an indirect jump to co-routine
  CF_RET,  // a return
  CF_SYS,
  CF_ICBR  // an indirect conditional branch
} CF_TYPE;

// Is floating point register
inline bool XED_REG_is_fr(xed_reg_enum_t reg) {
  return (xed_reg_class(reg) == XED_REG_CLASS_X87 ||
          xed_reg_class(reg) == XED_REG_CLASS_XMM ||
          xed_reg_class(reg) == XED_REG_CLASS_YMM ||
          xed_reg_class(reg) == XED_REG_CLASS_MXCSR);
}

// Merge 13,32 and 64 bit versions of registers into one macro.
xed_reg_enum_t XED_REG_FullRegName(xed_reg_enum_t reg) {
  switch (reg) {
    case XED_REG_FLAGS:
    case XED_REG_EFLAGS:
      return XED_REG_RFLAGS;

    case XED_REG_AX:
    case XED_REG_EAX:
    case XED_REG_AL:
    case XED_REG_AH:
      return XED_REG_RAX;

    case XED_REG_CX:
    case XED_REG_ECX:
    case XED_REG_CL:
    case XED_REG_CH:
      return XED_REG_RCX;

    case XED_REG_DX:
    case XED_REG_EDX:
    case XED_REG_DL:
    case XED_REG_DH:
      return XED_REG_RDX;

    case XED_REG_BX:
    case XED_REG_EBX:
    case XED_REG_BL:
    case XED_REG_BH:
      return XED_REG_RBX;

    case XED_REG_SP:
    case XED_REG_ESP:
    case XED_REG_SPL:
      return XED_REG_RSP;

    case XED_REG_BP:
    case XED_REG_EBP:
    case XED_REG_BPL:
      return XED_REG_RBP;

    case XED_REG_SI:
    case XED_REG_ESI:
    case XED_REG_SIL:
      return XED_REG_RSI;

    case XED_REG_DI:
    case XED_REG_EDI:
    case XED_REG_DIL:
      return XED_REG_RDI;

    case XED_REG_R8W:
    case XED_REG_R8D:
    case XED_REG_R8B:
      return XED_REG_R8;

    case XED_REG_R9W:
    case XED_REG_R9D:
    case XED_REG_R9B:
      return XED_REG_R9;

    case XED_REG_R10W:
    case XED_REG_R10D:
    case XED_REG_R10B:
      return XED_REG_R10;

    case XED_REG_R11W:
    case XED_REG_R11D:
    case XED_REG_R11B:
      return XED_REG_R11;

    case XED_REG_R12W:
    case XED_REG_R12D:
    case XED_REG_R12B:
      return XED_REG_R12;

    case XED_REG_R13W:
    case XED_REG_R13D:
    case XED_REG_R13B:
      return XED_REG_R13;

    case XED_REG_R14W:
    case XED_REG_R14D:
    case XED_REG_R14B:
      return XED_REG_R14;

    case XED_REG_R15W:
    case XED_REG_R15D:
    case XED_REG_R15B:
      return XED_REG_R15;

    case XED_REG_EIP:
    case XED_REG_IP:
      return XED_REG_RIP;

    default:
      return reg;
  };
}

using Qsim::OSDomain;

InstHandler_x86::InstHandler_x86() {
  prev_op = NULL;
  finished = false;
  stop_gen = false;
}

trace_gen_x86::trace_gen_x86(macsim_c *simBase, OSDomain &osd)
  : trace_gen(simBase, osd) {
  osd.set_app_start_cb(this, &trace_gen_x86::app_start_cb);
  set_gen_thread(new std::thread(&trace_gen_x86::gen_trace, this));
  inst_handle = NULL;
  nop = new trace_info_x86_qsim_s();
  memset(nop, 0, sizeof(trace_info_x86_qsim_s));
  nop->m_opcode = TR_NOP;
  xed_tables_init();
}

/////////////////////////////////////////////////////////////////////////////////////////
/// Instruction callback Function
/////////////////////////////////////////////////////////////////////////////////////////
void trace_gen_x86::inst_cb(int c, uint64_t v, uint64_t p, uint8_t l,
                            const uint8_t *b, enum inst_type t) {
  inst_handle[c].processInst((unsigned char *)b, v, l);
  inst_count++;

  return;
}

void InstHandler_x86::processInst(unsigned char *b, uint64_t v, uint8_t l) {
  trace_info_x86_qsim_s *op = new trace_info_x86_qsim_s();
  bzero(op, sizeof(trace_info_x86_qsim_s));

  set<xed_reg_enum_t> src_regs;
  set<xed_reg_enum_t> dst_regs;
  src_regs.clear();
  dst_regs.clear();

  xed_machine_mode_enum_t mmode;
  xed_address_width_enum_t stack_addr_width;
  // Hard coded right now since qsim only supports ia32
  xed_bool_t long_mode = 1;
  // The state of the machine -- required for decoding
  if (long_mode) {
    mmode = XED_MACHINE_MODE_LONG_64;
    stack_addr_width = XED_ADDRESS_WIDTH_64b;
  } else {
    mmode = XED_MACHINE_MODE_LEGACY_32;
    stack_addr_width = XED_ADDRESS_WIDTH_32b;
  }

  xed_error_enum_t xed_error;
  xed_uint_t noperands;
  const xed_inst_t *xi = 0;
  xed_decoded_inst_t xedd;
  xed_decoded_inst_zero(&xedd);
  xed_decoded_inst_set_mode(&xedd, mmode, stack_addr_width);
  xed_error = xed_decode(&xedd, XED_STATIC_CAST(const xed_uint8_t *, b), l);

  if (xed_error == XED_ERROR_NONE) {
    xi = xed_decoded_inst_inst(&xedd);
    xed_uint_t memops = xed_decoded_inst_number_of_memory_operands(&xedd);
    noperands = xed_inst_noperands(xi);

    // check all operands
    for (uint32_t ii = 0; ii < noperands; ++ii) {
      // operand - Register
      // add source and destination registers
      const xed_operand_t *op = xed_inst_operand(xi, ii);
      xed_operand_enum_t opname = xed_operand_name(op);
      if (xed_operand_is_register(opname) ||
          xed_operand_is_memory_addressing_register(opname)) {
        xed_reg_enum_t reg = xed_decoded_inst_get_reg(&xedd, opname);

        if ((xed_reg_class(reg) != XED_REG_CLASS_PSEUDO) &&
            (xed_reg_class(reg) != XED_REG_CLASS_PSEUDOX87)) {
          if (xed_operand_read(op)) {
            src_regs.insert(reg);
          }
          if (xed_operand_written(op)) {
            dst_regs.insert(reg);
          }
        }
      }
    }

    for (uint32_t ii = 0; ii < memops; ++ii) {
      // operand - Memory
      if (xed_decoded_inst_mem_read(&xedd, ii) ||
          xed_decoded_inst_mem_written(&xedd, ii)) {
        // Can have 2 loads per ins in some cases.(CMPS). This refers to
        // explicit loads in the instruction.
        if (xed_decoded_inst_mem_read(&xedd, ii)) {
          op->m_num_ld++;
          op->m_mem_read_size = 4;
        }

        assert(op->m_num_ld <= 2);

        if (xed_decoded_inst_mem_written(&xedd, ii)) {
          // Can have only one st per ins
          assert(!op->m_has_st);
          op->m_has_st = 1;
          op->m_mem_write_size = 4;
        }

        // Add base, index, or segment registers if they exist
        xed_reg_enum_t base = xed_decoded_inst_get_base_reg(&xedd, ii);
        if (base != XED_REG_INVALID) {
          src_regs.insert(base);
        }

        xed_reg_enum_t indx = xed_decoded_inst_get_index_reg(&xedd, ii);
        if (ii == 0 && indx != XED_REG_INVALID) {
          src_regs.insert(indx);
        }

        xed_reg_enum_t seg = xed_decoded_inst_get_seg_reg(&xedd, ii);
        if (seg != XED_REG_INVALID) {
          src_regs.insert(seg);
        }
      } else {
        // TODO: LEA instructions reach here. LEA is typically not a memory read,
        // but in some cases (immediate operand) its behaviour can be similar to
        // MOV. Not sure if LEA with imm reaches here.
      }
    }

    // handle source registers
    if (!src_regs.empty()) {
      op->m_num_read_regs = src_regs.size();
      assert(op->m_num_read_regs < MAX_SRC_NUM);

      set<xed_reg_enum_t>::iterator begin(src_regs.begin()),
        end(src_regs.end());
      uint8_t *ptr = op->m_src;

      while (begin != end) {
        if (XED_REG_is_fr(*begin)) {
          op->m_is_fp = true;
        }

        *ptr = XED_REG_FullRegName(*begin);
        *ptr = *begin;
        ++ptr;
        ++begin;
      }
    }

    // destination registers
    if (!dst_regs.empty()) {
      op->m_num_dest_regs = dst_regs.size();
      assert(op->m_num_dest_regs < MAX_DST_NUM);
      set<xed_reg_enum_t>::iterator begin(dst_regs.begin()),
        end(dst_regs.end());
      uint8_t *ptr = op->m_dst;
      while (begin != end) {
        if (xed_reg_class(*begin) == XED_REG_CLASS_FLAGS) op->m_write_flg = 1;

        if (XED_REG_is_fr(*begin)) op->m_is_fp = 1;

        *ptr = XED_REG_FullRegName(*begin);
        *ptr = *begin;
        ++ptr;
        ++begin;
      }
    }

    // instruction size
    op->m_size = l;

    // PC address
    op->m_instruction_addr = v;

    // set the opcode
    xed_iclass_enum_t inst_iclass = xed_decoded_inst_get_iclass(&xedd);
    xed_category_enum_t inst_category = xed_decoded_inst_get_category(&xedd);
    string inst_iclass_str(xed_iclass_enum_t2str(inst_iclass));

    if (inst_category == XED_CATEGORY_NOP ||
        inst_category == XED_CATEGORY_WIDENOP) {
      op->m_opcode = TR_NOP;
    } else if (inst_iclass_str.find("MUL") != string::npos) {
      if (inst_iclass == XED_ICLASS_IMUL || inst_iclass == XED_ICLASS_MUL) {
        op->m_opcode = TR_MUL;
      } else {
        op->m_opcode = TR_FMUL;
      }
    } else if (inst_iclass_str.find("DIV") != string::npos) {
      if (inst_iclass == XED_ICLASS_DIV || inst_iclass == XED_ICLASS_IDIV) {
        op->m_opcode = TR_DIV;
      } else {
        op->m_opcode = TR_FDIV;
      }
    }

    // opcode : prefetch
    else if (inst_iclass == XED_ICLASS_PREFETCHNTA) {
      op->m_opcode = PREFETCH_NTA;
      op->m_mem_read_size = 128;
    } else if (inst_iclass == XED_ICLASS_PREFETCHT0) {
      op->m_opcode = PREFETCH_T0;
      op->m_mem_read_size = 128;
    } else if (inst_iclass == XED_ICLASS_PREFETCHT1) {
      op->m_opcode = PREFETCH_T1;
      op->m_mem_read_size = 128;
    } else if (inst_iclass == XED_ICLASS_PREFETCHT2) {
      op->m_opcode = PREFETCH_T2;
      op->m_mem_read_size = 128;
    }

    // opcode : others
    else {
      op->m_opcode = (uint8_t)(inst_category);
    }

    // Branch instruction - set branch type
    xed_uint_t disp_bits =
      xed_decoded_inst_get_branch_displacement_width(&xedd);
    if (disp_bits) {
      // Direct branch
      xed_int32_t disp = xed_decoded_inst_get_branch_displacement(&xedd);
      op->m_branch_target = v + l + disp;
    }

    if (inst_category == XED_CATEGORY_UNCOND_BR) {
      if (disp_bits) {
        op->m_cf_type = CF_BR;
      } else {
        op->m_cf_type = CF_IBR;
      }
    } else if (inst_category == XED_CATEGORY_COND_BR) {
      if (disp_bits) {
        op->m_cf_type = CF_CBR;
      } else {
        op->m_cf_type = CF_ICBR;
      }
    } else if (inst_category == XED_CATEGORY_CALL) {
      if (disp_bits) {
        op->m_cf_type = CF_CALL;
      } else {
        op->m_cf_type = CF_ICALL;
      }
    } else if (inst_category == XED_CATEGORY_RET) {
      op->m_cf_type = CF_RET;
    } else if (inst_category == XED_CATEGORY_INTERRUPT) {
      op->m_cf_type = CF_ICO;
    } else {
      assert(!disp_bits);
      op->m_cf_type = NOT_CF;
    }
  }

  if (prev_op) {
    uint8_t prev_cf_type = prev_op->m_cf_type;

    if (prev_cf_type != NOT_CF) {
      switch (prev_cf_type) {
        case CF_IBR:
        case CF_BR:
        case CF_ICALL:
        case CF_CALL:
        case CF_RET:
        case CF_ICO:
          prev_op->m_branch_target = v;
          prev_op->m_actually_taken = true;
          break;
        case CF_CBR:
          assert(prev_op->m_branch_target);
          if (prev_op->m_branch_target == v) {
            prev_op->m_actually_taken = true;
          } else {
            prev_op->m_actually_taken = false;
          }
          break;
        case CF_ICBR:
          if (prev_op->m_instruction_addr + prev_op->m_size == v) {
            // TODO: Find g_inst_storage[tid][prev_iaddr[tid]] ->
            // branch_target for branches that are not taken. Edit: x86
            // doesn't have indirect conditional branches. Other ISAs might.
            prev_op->m_actually_taken = false;
          } else {
            prev_op->m_branch_target = v;
            prev_op->m_actually_taken = true;
          }
          break;
        default:
          assert(0);
      }
    }

    while (!child_q.empty()) {
      child_memop dm = child_q.front();
      if (dm.is_read) {
        prev_op->m_num_ld = 1;
        prev_op->m_has_st = 0;
        prev_op->m_ld_vaddr1 = dm.addr;
        prev_op->m_mem_read_size = dm.size;
      } else {
        prev_op->m_num_ld = 0;
        prev_op->m_has_st = 1;
        prev_op->m_st_vaddr = dm.addr;
        prev_op->m_mem_write_size = dm.size;
      }

      child_q.pop();

      // create new child uop
      trace_info_x86_qsim_s *child_op = new trace_info_x86_qsim_s();
      memcpy(child_op, prev_op, sizeof(trace_info_x86_qsim_s));
      child_op->m_opcode = CPU_MEM_EXT_OP;
      stream.enqueue(child_op);
    }
    if (prev_op->m_num_ld || prev_op->m_has_st)
      assert(prev_op->m_mem_read_size || prev_op->m_mem_write_size);
    stream.enqueue(prev_op);
  }

  prev_op = op;
}

void InstHandler_x86::processMem(uint64_t v, uint64_t p, uint8_t s, int w) {
  if (!prev_op) return;

  if ((prev_op->m_num_ld) && !w) {
    if (!prev_op->m_ld_vaddr1) {
      assert(!prev_op->m_ld_vaddr2);
      prev_op->m_ld_vaddr1 = v;
      prev_op->m_mem_read_size = s;
    } else {
      if (prev_op->m_num_ld == 2 && !prev_op->m_ld_vaddr2) {
        // There is only one size field. So, both loads should be of the same
        // size, as in CMPS.
        prev_op->m_mem_read_size = s;
        prev_op->m_ld_vaddr2 = v;
      }

      // Note: The trace structure has room for only 2 ld addresses and 1 st addr per ins.
      // Certain instructions can have multiple loads and stores.
      // Skip logging those right now unless the user explicitly enables them.
      // Push a new child ins with the same PC address and print it later to the trace.

      child_memop dm;
      dm.addr = v;
      dm.size = s;
      dm.is_read = true;
      child_q.push(dm);
    }

  } else if ((prev_op->m_has_st) && w) {
    if (!prev_op->m_st_vaddr) {
      prev_op->m_st_vaddr = v;
      prev_op->m_mem_write_size = s;
    } else {
      // Note: The trace structure has room for only 2 ld addresses and 1 st addr per ins.
      // Certain instructions can have multiple loads and stores.
      // Skip logging those right now unless the user explicitly enables them.

      // Push a new child ins with the same PC address and print it later to the trace.
      child_memop dm;
      dm.addr = v;
      dm.size = s;
      dm.is_read = false;
      child_q.push(dm);
    }
  } else {
    // We reach here when the instruction does not have explicit memory traffic,
    // but it implicitly does mem RW/WR.
    // For example, some JMP or CALL instructions which trigger state save/restore.

    // Note: The trace structure has room for only 2 ld addresses and 1 st addr per ins.
    // Certain instructions can have multiple loads and stores.
    // Skip logging those right now unless the user explicitly enables them.

    // Push a new child ins with the same PC address and print it later to the trace.

    child_memop dm;
    dm.addr = v;
    dm.size = s;

    if (w) {
      dm.is_read = false;
    } else {
      dm.is_read = true;
    }

    child_q.push(dm);
  }
}

int trace_gen_x86::app_end_cb(int c) {
  if (finished) return 1;

  for (int i = 0; i < osd.get_n(); i++) {
    inst_handle[i].finish();
  }

  finished = true;
  inst_handle[0].closeDebugFile();

  std::cout << "App end cb called. inst: " << inst_count << std::endl
            << " nop: " << nop_count << std::endl
            << " unid: " << unid_fences << std::endl
            << " full: " << full_fences << std::endl
            << " llsc: " << llsc << std::endl;

  return 1;
}

int trace_gen_x86::app_start_cb(int c) {
  int n_cpus = osd.get_n();
  inst_handle = new InstHandler_x86[osd.get_n()];

  for (int i = 0; i < osd.get_n(); i++) inst_handle[i].set_simbase(m_simBase);

  if (!started) {
    started = true;
    osd.set_inst_cb(this, &trace_gen_x86::inst_cb);
    osd.set_mem_cb(this, &trace_gen_x86::mem_cb);
    osd.set_app_end_cb(this, &trace_gen_x86::app_end_cb);
  }

  inst_handle[0].openDebugFile();
  trace_file_count++;

  return 0;
}

int InstHandler_x86::read_trace(void *buffer, unsigned int len) {
  trace_info_x86_qsim_s *trace_buffer = (trace_info_x86_qsim_s *)buffer;
  int i = 0, num_elements = len / sizeof(trace_info_x86_qsim_s);
  trace_info_x86_qsim_s *op = NULL;

  while (i < num_elements) {
    if (stream.size_approx() == 0) {
      if (finished) break;

      std::this_thread::yield();
      continue;
    }

    if (stream.try_dequeue(op)) {
      // ASSERT(op->m_opcode != ARM64_INS_INVALID);
      memcpy(trace_buffer + i, op, sizeof(trace_info_x86_qsim_s));
      delete op;
      i++;
    }
  }

  return i * sizeof(trace_info_x86_qsim_s);
}

void trace_gen_x86::gen_trace(void) {
  int i;

  while (true) {
    std::this_thread::yield();
    if (!started || finished) {
      continue;
    }

    /* DEBUG */
    /*
    for (i = 0; i < osd.get_n(); i++)
      std::cout << "(" << i << ", " << osd.get_tid(i) << ", " << osd.idle(i)
                << ", " << inst_handle[i].instq_size() << ") ";
    std::cout << "\r";
    */

    for (i = 0; i < osd.get_n(); i++) {
      if (inst_handle[i].instq_size() < 1000000) {
        osd.run(i, 10000);
      }
    }
  }
}

#endif /* USING_QSIM */
