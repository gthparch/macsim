#ifndef __RAMULATOR_WRAPPER_H
#define __RAMULATOR_WRAPPER_H

#ifdef RAMULATOR

#include <string>

#include "ramulator/src/Config.h"

using namespace std;

namespace ramulator {

class Request;
class MemoryBase;

class RamulatorWrapper {
private:
  MemoryBase *mem;

public:
  double tCK;
  RamulatorWrapper(const Config &configs, int cacheline);
  ~RamulatorWrapper();
  void tick();
  bool send(Request req);
  void finish(void);
};

} /*namespace ramulator*/

#endif // RAMULATOR
#endif /*__RAMULATOR_WRAPPER_H*/
