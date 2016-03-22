/*****************************************************************************\
 * Qemu Simulation Framework (qsim)                                            *
 * Qsim is a modified version of the Qemu emulator (www.qemu.org), couled     *
 * a C++ API, for the use of computer architecture researchers.                *
 *                                                                             *
 * This work is licensed under the terms of the GNU GPL, version 2. See the    *
 * COPYING file in the top-level directory.                                    *
 \*****************************************************************************/
#ifdef USING_QSIM

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>

#include <qsim.h>
#include <stdio.h>
#include <zlib.h>
#include <getopt.h>

#include "trace_gen_a64.h"
#include "assert_macros.h"

#define DEBUG 0

using Qsim::OSDomain;

using std::ostream;
using std::ofstream;
ofstream* debug_file;

cs_disas dis(CS_ARCH_ARM64, CS_MODE_ARM);

#define MILLION(x) (x * 1000000)

void InstHandler::processInst(unsigned char *b, uint64_t v, uint8_t len)
{
  cs_insn *insn = NULL;
  uint8_t regs_read_count, regs_write_count;
  cs_regs regs_read, regs_write;

  int count = dis.decode(b, len, insn);
  if (count) {
    insn->address = v;
    dis.get_regs_access(insn, regs_read, regs_write, &regs_read_count, &regs_write_count);
    populateInstInfo(insn, regs_read, regs_write, regs_read_count, regs_write_count);
    dis.free_insn(insn, count);
  }
}

void InstHandler::processMem(uint64_t v, uint64_t p, uint8_t len, int w)
{
  populateMemInfo(v, p, len, w);
}

InstHandler::InstHandler()
{
  prev_op      = NULL;
  finished     = false;
  stop_gen     = false;
}

void InstHandler::openDebugFile()
{
#if DEBUG
  debug_file = new ofstream("debug.log");
#endif
}

void InstHandler::closeDebugFile()
{
#if DEBUG
  if (!debug_file)
    return;
  debug_file->close();
  delete debug_file;
  debug_file = NULL;
#endif
}

InstHandler::~InstHandler()
{
}

int InstHandler::read_trace(void *buffer, unsigned int len)
{
  trace_info_a64_qsim_s* trace_buffer = (trace_info_a64_qsim_s *)buffer;
  int i = 0, num_elements = len / sizeof(trace_info_a64_qsim_s);
  trace_info_a64_qsim_s* op = NULL;

  while (i < num_elements) {
    if (stream.size_approx() == 0) {

      if (finished)
        break;

      std::this_thread::yield();
      continue;
    }

    if (stream.try_dequeue(op)) {
      ASSERT(op->m_opcode != ARM64_INS_INVALID);
      memcpy(trace_buffer+i, op, sizeof(trace_info_a64_qsim_s));
      delete op;
      i++;
    }
  }

  return i * sizeof(trace_info_a64_qsim_s);
}

void InstHandler::populateMemInfo(uint64_t v, uint64_t p, uint8_t s, int w)
{
  trace_info_a64_qsim_s *op = prev_op;

  if (!op)
    return;

  // since inst_cb if called first
  if (w) {
    if (!op->m_has_st) { /* first write */
      op->m_has_st            = 1;
      op->m_mem_write_size    = s;
      op->m_st_vaddr          = p;
    } else {
      op->m_mem_write_size   += s;
    }
  } else {
    if (!op->m_num_ld) { /* first load */
      op->m_num_ld++;
      op->m_ld_vaddr1         = p;
      op->m_mem_read_size     = s;
    } else if (op->m_ld_vaddr1 + op->m_mem_read_size == p) { /* second load */
      op->m_mem_read_size    += s;
    } else {
      op->m_num_ld++;
      op->m_ld_vaddr2         = p;
    }
  }
#if DEBUG
  if (debug_file) {
    *debug_file << std::endl
      << (w ? "Write: " : "Read: ")
      << "v: 0x" << std::hex << v
      << " p: 0x" << std::hex << p
      << " s: " << std::dec << (int)s
      << " val: " << std::hex << *(uint32_t *)p;
  }
#endif /* DEBUG */

  return;
}

bool InstHandler::populateInstInfo(cs_insn *insn, cs_regs regs_read, cs_regs regs_write, 
    uint8_t regs_read_count, uint8_t regs_write_count)
{
  cs_arm64* arm64;

  if (insn->detail == NULL)
    return false;

  trace_info_a64_qsim_s *op = new trace_info_a64_qsim_s();
  arm64 = &(insn->detail->arm64);

  op->m_num_read_regs = regs_read_count;
  op->m_num_dest_regs = regs_write_count;

  for (int i = 0; i < regs_read_count; i++)
    op->m_src[i] = regs_read[i];
  for (int i = 0; i < regs_write_count; i++)
    op->m_dst[i] = regs_write[i];

  op->m_cf_type = 0;
  for (int grp_idx = 0; grp_idx < insn->detail->groups_count; grp_idx++) {
    if (insn->detail->groups[grp_idx] == ARM64_GRP_JUMP) {
      op->m_cf_type = 1;
      break;
    }
  }

  op->m_has_immediate = 0;
  for (int op_idx = 0; op_idx < arm64->op_count; op_idx++) {
    if (arm64->operands[op_idx].type == ARM64_OP_IMM ||
        arm64->operands[op_idx].type == ARM64_OP_CIMM) {
      op->m_has_immediate = 1;
      break;
    }
  }

  op->m_opcode = insn->id;
  op->m_has_st = 0;
  op->m_is_fp = 0;
  for (int op_idx = 0; op_idx < arm64->op_count; op_idx++) {
    if (arm64->operands[op_idx].type == ARM64_OP_FP) {
      op->m_is_fp = 1;
      break;
    }
  }

  op->m_write_flg  = arm64->writeback;

  // TODO: figure out based on opcode
  op->m_num_ld = 0;
  op->m_size = 4;

  // initialize current inst dynamic information
  op->m_ld_vaddr2 = 0;
  op->m_st_vaddr = 0;
  op->m_instruction_addr  = insn->address;

  op->m_branch_target = 0;
  int offset = 0;
  if (op->m_cf_type) {
    if (op->m_has_immediate)
      for (int op_idx = 0; op_idx < arm64->op_count; op_idx++) {
        if (arm64->operands[op_idx].type == ARM64_OP_IMM) {
          offset = (int64_t) arm64->operands[op_idx].imm;
          op->m_branch_target = op->m_instruction_addr + offset;
          break;
        }
      }
  }

  op->m_mem_read_size = 0;
  op->m_mem_write_size = 0;
  op->m_rep_dir = 0;
  op->m_actually_taken = 0;

  // auxiliary information for prefetch and barrier instructions
  if (arm64->op_count) {
    if (arm64->operands[0].type == ARM64_OP_PREFETCH)
      op->m_st_vaddr = arm64->operands[0].prefetch;

    if (arm64->operands[0].type == ARM64_OP_BARRIER)
      op->m_st_vaddr = arm64->operands[0].barrier;
  }

  // populate prev inst dynamic information
  if (prev_op) {
    if (op->m_instruction_addr == prev_op->m_branch_target)
      prev_op->m_actually_taken = 1;

    // push prev op into the stream
    stream.enqueue(prev_op);

#if DEBUG
    if (debug_file) {
      if (prev_op->m_cf_type)
        *debug_file << " Taken " << prev_op->m_actually_taken << std::endl;
      else
        *debug_file << std::endl;
    }
#endif /* DEBUG */
  }

#if DEBUG
  if (debug_file) {
    *debug_file << std::endl << std::endl;
    *debug_file << "IsBranch: " << (int)op->m_cf_type
      << " Offset:   " << std::setw(8) << std::hex << offset
      << " Target:  "  << std::setw(8) << std::hex << op->m_branch_target << " ";
    *debug_file << std::endl;
    *debug_file << //a64_opcode_names[insn->id] << 
      ": " << std::hex << insn->address <<
      ": " << insn->mnemonic <<
      ": " << insn->op_str;
    *debug_file << std::endl;
    *debug_file << "Src: ";
    for (int i = 0; i < op->m_num_read_regs; i++)
      *debug_file << std::dec << (int) op->m_src[i] << " ";
    *debug_file << std::endl << "Dst: ";
    for (int i = 0; i < op->m_num_dest_regs; i++)
      *debug_file << std::dec << (int) op->m_dst[i] << " ";
    *debug_file << std::endl;
  } else {
    std::cout << "Writing to a null tracefile" << std::endl;
  }
#endif /* DEBUG */

  prev_op = op;

  return true;
}

void tracegen_a64::count_fences(const uint8_t *b, uint8_t l)
{
  cs_insn *insn = NULL;

  int count = dis.decode((unsigned char *)b, l, insn);

  switch (insn[0].id) {
  case ARM64_INS_STLR:
  case ARM64_INS_STLRB:
  case ARM64_INS_STLRH:
  case ARM64_INS_LDAR:
  case ARM64_INS_LDARB:
  case ARM64_INS_LDARH:
	  unid_fences++;
	  break;
  case ARM64_INS_STLXR:
  case ARM64_INS_STLXRB:
  case ARM64_INS_STLXRH:
  case ARM64_INS_LDAXR:
  case ARM64_INS_LDAXRB:
  case ARM64_INS_LDAXRH:
	  llsc++;
	  break;
  case ARM64_INS_DMB:
  case ARM64_INS_DSB:
  case ARM64_INS_ISB:
	  full_fences++;
	  break;
  default:
	  break;
  }

  dis.free_insn(insn, count);
}

void tracegen_a64::inst_cb(int c, uint64_t v, uint64_t p, uint8_t l, const uint8_t *b,
	     enum inst_type t)
{
  inst_handle[c].processInst((unsigned char*)b, v, l);
  inst_count++;

  return;
}

int tracegen_a64::app_start_cb(int c)
{
  int n_cpus = osd.get_n();
  inst_handle = new InstHandler[osd.get_n()];

  for (int i = 0; i < osd.get_n(); i++)
    inst_handle[i].set_simbase(m_simBase);

  if (!started) {
    started = true;
    osd.set_inst_cb(this, &tracegen_a64::inst_cb);
    osd.set_mem_cb(this, &tracegen_a64::mem_cb);
    osd.set_app_end_cb(this, &tracegen_a64::app_end_cb);
  }

  inst_handle[0].openDebugFile();
  trace_file_count++;

  return 0;
}

int tracegen_a64::app_end_cb(int c)
{
  if (finished)
    return 1;

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

void tracegen_a64::gen_trace(void)
{
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
