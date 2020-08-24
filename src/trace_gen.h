#ifndef _TRACE_GEN_H
#define _TRACE_GEN_H

#include "macsim.h"

#include <zlib.h>
#include <thread>

#include "qsim.h"
#include "capstone.h"
#include "cs_disas.h"
#include "readerwriterqueue.h"

using namespace moodycamel;
using namespace Qsim;

class InstHandler
{
public:
  virtual void populateMemInfo(uint64_t v, uint64_t p, uint8_t s, int w) = 0;
  void openDebugFile();
  void closeDebugFile();
  virtual void processInst(unsigned char *b, uint64_t v, uint8_t len) = 0;
  virtual void processMem(uint64_t v, uint64_t p, uint8_t len, int w) = 0;
  void finish(void) {
    finished = true;
  }
  virtual int read_trace(void *buffer, unsigned int len) = 0;

  virtual uint64_t instq_size() = 0;
  void set_simbase(macsim_c *simBase) {
    m_simBase = simBase;
  }

  std::atomic<bool> stop_gen;

protected:
  int m_fp_uop_table[ARM64_INS_ENDING];
  int m_int_uop_table[ARM64_INS_ENDING];
  bool finished;

  macsim_c *m_simBase;
};

class trace_gen
{
public:
  trace_gen(macsim_c *simBase, OSDomain &osd)
    : osd(osd), finished(false), started(false), inst_count(0), nop_count(0) {
    trace_file_count = 0;
    finished = false;

    m_simBase = simBase;

    unid_fences = full_fences = llsc = 0;
  }

  ~trace_gen() {
  }

  void set_gen_thread(std::thread *thread) {
    gen_thread = thread;
  }
  virtual bool trace_avail(int c) = 0;

  virtual void gen_trace(void) = 0;

  virtual void count_fences(const uint8_t *b, uint8_t l) = 0;
  virtual void inst_cb(int c, uint64_t v, uint64_t p, uint8_t l,
                       const uint8_t *b, enum inst_type t) = 0;

  virtual void mem_cb(int c, uint64_t v, uint64_t p, uint8_t s, int w) = 0;

  OSDomain *get_osd(void) {
    return &osd;
  }

  virtual int read_trace(int c, void *buffer, unsigned int len) = 0;
  virtual int app_start_cb(int c) = 0;
  virtual int app_end_cb(int c) = 0;

protected:
  OSDomain &osd;
  bool finished, started;
  int trace_file_count;
  uint64_t inst_count, nop_count;
  std::thread *gen_thread;

  macsim_c *m_simBase;
  uint64_t unid_fences, full_fences, llsc;
};

#endif /* _TRACE_GEN_H */
