/*****************************************************************************\
 * Qemu Simulation Framework (qsim)                                            *
 * Qsim is a modified version of the Qemu emulator (www.qemu.org), couled     *
 * a C++ API, for the use of computer architecture researchers.                *
 *                                                                             *
 * This work is licensed under the terms of the GNU GPL, version 2. See the    *
 * COPYING file in the top-level directory.                                    *
 \*****************************************************************************/
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>

#include <qsim.h>
#include <stdio.h>
#include <capstone.h>
#include <zlib.h>
#include <getopt.h>

#include "gzstream.h"
#include "cs_disas.h"
#include "macsim_tracegen.h"

#include "readerwriterqueue.h"

using namespace moodycamel;

#define DEBUG 0

using Qsim::OSDomain;

using std::ostream;
ogzstream* debug_file;

cs_disas dis(CS_ARCH_ARM64, CS_MODE_ARM);

#define MILLION(x) (x * 1000000)

#define STREAM_SIZE 10000000

class InstHandler {
  public:
    InstHandler();
    ~InstHandler();
    InstHandler(gzFile& outfile);
    void setOutFile(gzFile outfile);
    void closeOutFile(void);
    bool populateInstInfo(cs_insn *insn, cs_regs regs_read, cs_regs regs_write,
        uint8_t regs_read_count, uint8_t regs_write_count);
    void populateMemInfo(uint64_t v, uint64_t p, uint8_t s, int w);
    void dumpInstInfo();
    void openDebugFile();
    void closeDebugFile();
    void processInst(unsigned char *b, uint8_t len);
    void processMem(uint64_t v, uint64_t p, uint8_t len, int w);
    void processAll(void);
    void finish(void) { finished = true; ithread->join();}

  private:

    class instInfo {
    public:
      instInfo() {}
      
      instInfo(uint64_t v, uint64_t p, uint8_t s, int w) :
        size(s), virt(v), phys(p), rw(w) {}

      instInfo(unsigned char *b, uint8_t len) :
        mem_ptr(b), size(len) { rw = -1; }

      ~instInfo() {}
      unsigned char *mem_ptr;
      uint8_t  size;
      uint64_t virt, phys;
      int      rw;
    };

    trace_info_a64_s *stream;
    int stream_idx;
    gzFile outfile;
    int m_fp_uop_table[ARM64_INS_ENDING];
    int m_int_uop_table[ARM64_INS_ENDING];
    bool finished;
    std::thread *ithread;

    ReaderWriterQueue<instInfo *> instructions;
};

void InstHandler::processInst(unsigned char *b, uint8_t len)
{
  instructions.enqueue(new instInfo(b, len));
}

void InstHandler::processMem(uint64_t v, uint64_t p, uint8_t len, int w)
{
  instructions.enqueue(new instInfo(v, p, len, w));
}

void InstHandler::processAll(void)
{
  while (!finished) {

    instInfo *inst;
    if (instructions.try_dequeue(inst) == false) {
      std::this_thread::yield();
      continue;
    }

    if (inst->rw != -1)
      populateMemInfo(inst->virt, inst->phys, inst->size, inst->rw);
    else {
      cs_insn *insn = NULL;
      uint8_t regs_read_count, regs_write_count;
      cs_regs regs_read, regs_write;

      int count = dis.decode((unsigned char *)inst->mem_ptr, inst->size, insn);
      insn[0].address = inst->virt;
      dis.get_regs_access(insn, regs_read, regs_write, &regs_read_count, &regs_write_count);
      populateInstInfo(insn, regs_read, regs_write, regs_read_count, regs_write_count);
      dis.free_insn(insn, count);
    }

    delete inst;
  }
}

InstHandler::InstHandler()
{
  stream = new trace_info_a64_s[STREAM_SIZE];
  stream_idx = 0;
  finished = false;
  ithread = new std::thread(&InstHandler::processAll, this);
}

void InstHandler::openDebugFile()
{
#if DEBUG
  debug_file = new ogzstream("debug.log.gz");
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
  delete[] stream;
}

void InstHandler::dumpInstInfo(void)
{
  gzwrite(outfile, stream, stream_idx*sizeof(trace_info_a64_s));
  stream_idx = 0;
}

void InstHandler::setOutFile(gzFile file)
{
  outfile = file;
}

void InstHandler::closeOutFile(void)
{
  dumpInstInfo();
  gzclose(outfile);
}

void InstHandler::populateMemInfo(uint64_t v, uint64_t p, uint8_t s, int w)
{
  trace_info_a64_s *op = &stream[stream_idx-1]; // stream_idx is atleast 1,
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
  trace_info_a64_s *op = &stream[stream_idx];
  trace_info_a64_s *prev_op = NULL;

  if (stream_idx)
    prev_op = &stream[stream_idx-1];

  if (insn->detail == NULL)
    return false;

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

    if (stream_idx+1 == STREAM_SIZE) {
      // dump trace for previous ops
      gzwrite(outfile, stream, stream_idx*sizeof(trace_info_a64_s));

      // copy the current op to the head of the stream
      memcpy(stream, op, sizeof(trace_info_a64_s));
      stream_idx = 0;
    }
#if DEBUG
    if (debug_file) {
      if (prev_op->m_cf_type)
        *debug_file << " Taken " << prev_op->m_actually_taken << std::endl;
      else
        *debug_file << std::endl;
    }
#endif /* DEBUG */
  }
  stream_idx++;

#if DEBUG
  if (debug_file) {
    *debug_file << std::endl << std::endl;
    *debug_file << "IsBranch: " << (int)op->m_cf_type
      << " Offset:   " << std::setw(8) << std::hex << offset
      << " Target:  "  << std::setw(8) << std::hex << op->m_branch_target << " ";
    *debug_file << std::endl;
    *debug_file << a64_opcode_names[insn->id] << 
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

  return true;
}

class TraceWriter {
  public:
    TraceWriter(OSDomain &osd, unsigned long max_inst) :
      osd(osd), finished(false)
    { 
      osd.set_app_start_cb(this, &TraceWriter::app_start_cb);
      trace_file_count = 0;
      finished = false;
      max_inst_n = max_inst;
    }

    ~TraceWriter()
    {
    }

    bool hasFinished() { return finished; }

    int app_start_cb(int c)
    {
      static bool ran = false;
      int n_cpus = osd.get_n();
      if (!ran) {
        ran = true;
        osd.set_inst_cb(this, &TraceWriter::inst_cb);
        osd.set_mem_cb(this, &TraceWriter::mem_cb);
        osd.set_app_end_cb(this, &TraceWriter::app_end_cb);
      }
      inst_handle = new InstHandler[osd.get_n()];
      for (int i = 0; i < n_cpus; i++) {
        gzFile tracefile  = gzopen(("trace_" + std::to_string(trace_file_count)
              + "-" + std::to_string(i) + ".log.gz").c_str(), "w");
        inst_handle[i].setOutFile(tracefile);
      }
      inst_handle[0].openDebugFile();
      trace_file_count++;
      finished = false;
      curr_inst_n = max_inst_n;

      return 0;
    }

    int app_end_cb(int c)
    {
      if (finished)
        return 0;

      std::cout << "App end cb called" << std::endl;
      finished = true;

      for (int i = 0; i < osd.get_n(); i++) {
        inst_handle[i].finish();
        inst_handle[i].closeOutFile();
      }

      inst_handle[0].closeDebugFile();

      delete [] inst_handle;
      return 0;
    }

    void inst_cb(int c, uint64_t v, uint64_t p, uint8_t l, const uint8_t *b,
        enum inst_type t)
    {
      if (!curr_inst_n)
        return;

      inst_handle[c].processInst((unsigned char*)b, l);

      --curr_inst_n;
      if (!curr_inst_n) {

        app_end_cb(0);
      }

      return;
    }

    void mem_cb(int c, uint64_t v, uint64_t p, uint8_t s, int w)
    {
      if (!curr_inst_n)
        return;

      inst_handle[c].processMem(v, p, s, w);
    }

  private:
    OSDomain &osd;
    bool finished;
    int  trace_file_count;
    InstHandler *inst_handle;
    unsigned long max_inst_n;
    unsigned long curr_inst_n;
};

int main(int argc, char** argv) {
  using std::istringstream;
  using std::ofstream;

  int n_cpus = 1, max_inst = MILLION(500);

  std::string qsim_prefix(getenv("QSIM_PREFIX"));

  static struct option long_options[] = {
    {"help",  no_argument, NULL, 'h'},
    {"ncpu", required_argument, NULL, 'n'},
    {"max_inst", required_argument, NULL, 'm'},
    {"state", required_argument, NULL, 's'}
  };

  int c = 0;
  char *state_file = NULL;
  while((c = getopt_long(argc, argv, "hn:m:", long_options, NULL)) != -1) {
    switch(c) {
      case 'n':
        n_cpus = atoi(optarg);
        break;
      case 'm':
        max_inst = MILLION(atoi(optarg));
        break;
      case 's':
        state_file = strdup(optarg);
      case 'h':
      case '?':
      default:
        std::cout << "Usage: " << argv[0] << " --ncpu(-n) <num_cpus> --max_inst(-m)" <<
          "  <num_inst(M)> --state <state_file>" << std::endl;
        exit(0);
    }
  }

  unsigned long max_inst_n = n_cpus * max_inst;

  OSDomain *osd_p(NULL);

  if (!state_file)
    osd_p = new OSDomain(n_cpus, qsim_prefix + "/../arm64_images/vmlinuz", "a64", QSIM_INTERACTIVE);
  else
    osd_p = new OSDomain(n_cpus, state_file);

  OSDomain &osd(*osd_p);

  // Attach a TraceWriter if a trace file is given.
  TraceWriter tw(osd, max_inst_n);

  // If this OSDomain was created from a saved state, the app start callback was
  // received prior to the state being saved.
  //if (argc >= 4) tw.app_start_cb(0);

  osd.connect_console(std::cout);

  //tw.app_start_cb(0);
  // The main loop: run until 'finished' is true.
  uint64_t inst_per_iter = 1000000000;
  int inst_run = inst_per_iter;
  while (!(inst_per_iter - inst_run)) {
    inst_run = osd.run(0, inst_per_iter);
    osd.timer_interrupt();
  }

  delete osd_p;

  return 0;
}
