#ifdef RAMULATOR

#include <map>

#include "ramulator/src/Config.h"
#include "ramulator/src/DDR3.h"
#include "ramulator/src/DDR4.h"
#include "ramulator/src/GDDR5.h"
#include "ramulator/src/HBM.h"
#include "ramulator/src/LPDDR3.h"
#include "ramulator/src/LPDDR4.h"
#include "ramulator/src/Memory.h"
#include "ramulator/src/MemoryFactory.h"
#include "ramulator/src/Request.h"
#include "ramulator/src/SALP.h"
#include "ramulator/src/Statistics.h"
#include "ramulator/src/WideIO.h"
#include "ramulator/src/WideIO2.h"

#include "ramulator_wrapper.h"

using namespace ramulator;

static map<string, function<MemoryBase *(const Config &, int)>> name_to_func = {
  {"DDR3", &MemoryFactory<DDR3>::create},
  {"DDR4", &MemoryFactory<DDR4>::create},
  {"LPDDR3", &MemoryFactory<LPDDR3>::create},
  {"LPDDR4", &MemoryFactory<LPDDR4>::create},
  {"GDDR5", &MemoryFactory<GDDR5>::create},
  {"WideIO", &MemoryFactory<WideIO>::create},
  {"WideIO2", &MemoryFactory<WideIO2>::create},
  {"HBM", &MemoryFactory<HBM>::create},
  {"SALP-1", &MemoryFactory<SALP>::create},
  {"SALP-2", &MemoryFactory<SALP>::create},
  {"SALP-MASA", &MemoryFactory<SALP>::create},
};

RamulatorWrapper::RamulatorWrapper(const Config &configs, int cacheline) {
  Stats::statlist.output("ramulator.stat.out");
  const string &std_name = configs["standard"];
  assert(name_to_func.find(std_name) != name_to_func.end() && "unrecognized standard name");
  mem = name_to_func[std_name](configs, cacheline);
  tCK = mem->clk_ns();
}

RamulatorWrapper::~RamulatorWrapper() { 
  Stats::statlist.printall();
  delete mem; 
}

void RamulatorWrapper::tick() { 
  mem->tick(); 
}

bool RamulatorWrapper::send(Request req) {
  return mem->send(req); 
}

void RamulatorWrapper::finish(void) {
  mem->finish(); 
}

#endif // RAMULATOR
