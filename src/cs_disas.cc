#ifdef USING_QSIM

#include "cs_disas.h"
#include <iostream>

cs_disas::cs_disas(cs_arch arch, cs_mode mode) {
  pf = {arch, mode};
  cs_err err = cs_open(pf.arch, pf.mode, &handle);
  if (err) {
    std::cerr << "Failed on cs_open with error: " << err << std::endl;
    return;
  }

  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
}

cs_disas::~cs_disas() {
  cs_close(&handle);
}

int cs_disas::decode(unsigned char *code, int size, cs_insn *&insn) {
  int count;

  count = cs_disasm(handle, code, size, 0, 0, &insn);

  return count;
}

void cs_disas::free_insn(cs_insn *insn, int count) {
  cs_free(insn, count);
}

bool cs_disas::get_regs_access(cs_insn *insn, cs_regs regs_read,
                               cs_regs regs_write, uint8_t *regs_read_count,
                               uint8_t *regs_write_count) {
  bool ret;

  ret = cs_regs_access(handle, insn, regs_read, regs_read_count, regs_write,
                       regs_write_count);

  return ret;
}

#endif /* USING_QSIM */
