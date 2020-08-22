#ifndef _TRACE_GEN_X86_H
#define _TRACE_GEN_X86_H

#include "macsim.h"
#include "trace_gen.h"

#include <inttypes.h>

#include <stdio.h>
#include <zlib.h>
#include <ostream>

#define MAX_SRC_NUM 9
#define MAX_DST_NUM 6
#define USE_MAP 0
#define MAX_THREADS 1000

struct trace_info_x86_qsim_s {
  uint8_t m_num_read_regs; /**< num read registers */
  uint8_t m_num_dest_regs; /**< num dest registers */
  uint8_t m_src[MAX_SRC_NUM]; /**< src register id */
  uint8_t m_dst[MAX_DST_NUM]; /**< dest register id */
  uint8_t m_cf_type; /**< branch type */
  bool m_has_immediate; /**< has immediate field */
  uint8_t m_opcode; /**< opcode */
  bool m_has_st; /**< has store operation */
  bool m_is_fp; /**< fp operation */
  bool m_write_flg; /**< write flag */
  uint8_t m_num_ld; /**< number of load operations */
  uint8_t m_size; /**< instruction size */
  // dynamic information
  uint64_t m_ld_vaddr1; /**< load address 1 */
  uint64_t m_ld_vaddr2; /**< load address 2 */
  uint64_t m_st_vaddr; /**< store address */
  uint64_t m_instruction_addr; /**< pc address */
  uint64_t m_branch_target; /**< branch target address */
  uint8_t m_mem_read_size; /**< memory read size */
  uint8_t m_mem_write_size; /**< memory write size */
  bool m_rep_dir; /**< repetition direction */
  bool m_actually_taken; /**< branch actually taken */
};

#define BUF_SIZE (10000 * sizeof(trace_info_x86_qsim_s))

struct Trace_info {
  gzFile trace_stream;
  char trace_buf[BUF_SIZE];
  int bytes_accumulated;
  std::ofstream *debug_stream;
};

typedef struct {
  uint64_t addr;
  uint8_t size;
  bool is_read;
} child_memop;

typedef std::pair<uint64_t, uint64_t> va_pa_pair;

class InstHandler_x86 : public InstHandler
{
public:
  InstHandler_x86();
  ~InstHandler_x86() {
  }

  int read_trace(void *buffer, unsigned int len);
  uint64_t instq_size() {
    return stream.size_approx();
  }

  void populateMemInfo(uint64_t v, uint64_t p, uint8_t s, int w) {
  }
  void processInst(unsigned char *b, uint64_t v, uint8_t len);
  void processMem(uint64_t v, uint64_t p, uint8_t len, int w);

  void set_rep_dir(bool dir) {
    if (prev_op) prev_op->m_rep_dir = dir;
  }

private:
  trace_info_x86_qsim_s *prev_op, *nop;
  BlockingReaderWriterQueue<trace_info_x86_qsim_s *> stream;
  queue<child_memop> child_q;
};

class trace_gen_x86 : public trace_gen
{
public:
  trace_gen_x86(macsim_c *simBase, OSDomain &osd);
  void gen_trace(void);
  void count_fences(const uint8_t *b, uint8_t l) {
  }
  void inst_cb(int c, uint64_t v, uint64_t p, uint8_t l, const uint8_t *b,
               enum inst_type t);

  ~trace_gen_x86() {
  }

  int read_trace(int c, void *buffer, unsigned int len) {
    /* DEBUG */
    /*
    for (int i = 0; i < osd.get_n(); i++)
      std::cout << "(" << i << ", " << osd.get_tid(i) << ", " << osd.idle(i)
                << ", " << std::setw(6) << inst_handle[i].instq_size() << ") ";
    std::cout << "\r";
    */

    if (inst_handle[c].instq_size() > 0) {
      return inst_handle[c].read_trace(buffer, len);
    } else if (osd.runnable(c)) {
      if (osd.get_tid(c) == osd.get_bench_pid()) {
        // wait for instructions since a thread of the benchmark is running
        return inst_handle[c].read_trace(buffer, len);
      } else {
        // the benchmark thread is not running, use NOPs
        trace_info_x86_qsim_s *trace_buffer = (trace_info_x86_qsim_s *)buffer;
        int i = 0, num_elements = len / sizeof(trace_info_x86_qsim_s);

        while (i < num_elements) {
          nop_count++;
          memcpy(trace_buffer + i, nop, sizeof(trace_info_x86_qsim_s));
          i++;
        }

        return i * sizeof(trace_info_x86_qsim_s);
      }
    }

    // core is done executing thread and no more instructions, end simulation
    return 0;
  }

  bool trace_avail(int c) {
    return inst_handle[c].instq_size() || !finished;
  }

  void mem_cb(int c, uint64_t v, uint64_t p, uint8_t s, int w) {
    inst_handle[c].set_rep_dir((osd.get_reg(c, QSIM_X86_RFLAGS) & 0x400) >> 10);
    inst_handle[c].processMem(v, p, s, w);
  }

  int app_start_cb(int c);
  int app_end_cb(int c);

private:
  InstHandler_x86 *inst_handle;
  trace_info_x86_qsim_s *nop;
};

#endif /* _TRACE_GEN_X86_H */
