/* ****************************************************************************************** */
/* ****************************************************************************************** */ 
#include "macsim.h"

#include <zlib.h>
#include <thread>

#include "capstone.h"
#include "cs_disas.h"
#include "readerwriterqueue.h"

using namespace moodycamel;
using namespace Qsim;

#define REP_MOV_MEM_SIZE_MAX 4
#define REP_MOV_MEM_SIZE_MAX_NEW MAX2(REP_MOV_MEM_SIZE_MAX, (*KNOB(KNOB_MEM_SIZE_AMP)*4))
#define MAX_SRC_NUM 9
#define MAX_DST_NUM 6

typedef struct trace_info_a64_qsim_s {
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
} trace_info_a64_qsim_s;

class InstHandler {
  public:
    InstHandler();
    ~InstHandler();
    bool populateInstInfo(cs_insn *insn, cs_regs regs_read, cs_regs regs_write,
        uint8_t regs_read_count, uint8_t regs_write_count);
    void populateMemInfo(uint64_t v, uint64_t p, uint8_t s, int w);
    void dumpInstInfo();
    void openDebugFile();
    void closeDebugFile();
    void processInst(unsigned char *b, uint64_t v, uint8_t len);
    void processMem(uint64_t v, uint64_t p, uint8_t len, int w);
    void finish(void) { finished = true; }
    int  read_trace(void *buffer, unsigned int len);

    uint64_t instq_size()
    {
      return stream.size_approx();
    }

    void set_simbase(macsim_c *simBase) { m_simBase = simBase; }

    std::atomic<bool> stop_gen;

  private:

    int m_fp_uop_table[ARM64_INS_ENDING];
    int m_int_uop_table[ARM64_INS_ENDING];
    bool finished;

    trace_info_a64_qsim_s *prev_op, *nop;
    BlockingReaderWriterQueue<trace_info_a64_qsim_s*> stream;

    macsim_c *m_simBase;
};

class tracegen_a64 {
  public:
    tracegen_a64(macsim_c* simBase, OSDomain &osd) :
    osd(osd), finished(false), started(false), inst_count(0), nop_count(0)
    { 
      osd.set_app_start_cb(this, &tracegen_a64::app_start_cb);
      trace_file_count = 0;
      finished = false;
      gen_thread = new std::thread(&tracegen_a64::gen_trace, this);
      inst_handle = NULL;

      nop = new trace_info_a64_qsim_s();
      memset(nop, 0, sizeof(trace_info_a64_qsim_s));
      nop->m_opcode = ARM64_INS_NOP;

      m_simBase = simBase;
    }

    ~tracegen_a64()
    {
    }

    bool trace_avail(int c) { return inst_handle[c].instq_size() || !finished; }

    bool hasFinished() { return finished; }
    bool hasStarted() { return started; }

    int app_start_cb(int c);
    int app_end_cb(int c);

    int read_trace(int c, void *buffer, unsigned int len)
    {
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
          trace_info_a64_qsim_s* trace_buffer = (trace_info_a64_qsim_s *)buffer;
          int i = 0, num_elements = len / sizeof(trace_info_a64_qsim_s);

          while (i < num_elements) {
            nop_count++;
            memcpy(trace_buffer+i, nop, sizeof(trace_info_a64_qsim_s));
            i++;
          }

          return i * sizeof(trace_info_a64_qsim_s);
        }
      }

      // core is done executing thread and no more instructions, end simulation
      return 0;
    }

    void gen_trace(void);

    void inst_cb(int c, uint64_t v, uint64_t p, uint8_t l, const uint8_t *b,
        enum inst_type t)
    {
      inst_handle[c].processInst((unsigned char*)b, v, l);
      inst_count++;

      return;
    }

    void mem_cb(int c, uint64_t v, uint64_t p, uint8_t s, int w)
    {
      inst_handle[c].processMem(v, p, s, w);
    }

    OSDomain *get_osd(void) {
      return &osd;
    }

  private:
    OSDomain &osd;
    bool finished, started;
    int  trace_file_count;
    InstHandler *inst_handle;
    uint64_t     inst_count, nop_count;
    std::thread *gen_thread;
    trace_info_a64_qsim_s *nop;

    macsim_c *m_simBase;
};

