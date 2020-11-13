#ifndef MACSIM_COMPONENT_H
#define MACSIM_COMPONENT_H

#ifndef MACSIMCOMPONENT_DBG
#define MACSIMCOMPONENT_DBG 1
#endif

#include <inttypes.h>

#include <sst/core/event.h>
#include <sst/core/sst_types.h>
#include <sst/core/component.h>
#include <sst/core/link.h>
#include <sst/core/timeConverter.h>
#include <sst/core/interfaces/simpleMem.h>

#include "src/macsim.h"

using namespace std;
using namespace SST::Interfaces;

namespace SST {
namespace MacSim {

enum DebugLevel {
  ERROR,
  WARNING,
  INFO,
  L0,
  L1,
  L2,
  L3,
  L4,
  L5,
  L6
};  // debug level
enum OperationMode { MASTER, SLAVE };  // mode

class macsimComponent : public SST::Component
{
public:
  /* SST ELI */
  SST_ELI_REGISTER_COMPONENT(macsimComponent, "macsimComponent",
                             "macsimComponent",
                             SST_ELI_ELEMENT_VERSION(1, 0, 0),
                             "A Heterogeneous Architecture Simulator",
                             COMPONENT_CATEGORY_PROCESSOR)
  SST_ELI_DOCUMENT_PARAMS(
    {"param_file", "params.in", NULL}, {"trace_file", "trace_file_list", NULL},
    {"output_dir", "output (stats, params.out, etc.) directory", NULL},
    {"command_line", "specify knobs, if any", NULL},
    {"cube_connected", "Depricated", "0"},
    {"operation_mode", "0: Master, 1: Slave", "0"},
    {"frequency", "clock frequency", "1GHz"},
    {"mem_size",
     "memory size in bytes. E.g., 1073741824 (=1024*1024*1024) for 1GB",
     "1073741824"},
    {"ptx_core", "1: GPU core, 0: CPU core", "0"},
    {"num_link", "this should match to the number of cores and/or L1 caches",
     "1"},
    {"debug", "0:No debugging, 1:stdout, 2:stderr, 3:file", "0"},
    {"debug_level", "debugging level: 0 to 9", "8"},
    {"debug_addr",
     "address (in decimal) to be debugged, if not specified or specified as "
     "-1, debug output for all addresses will be printed",
     "-1"})

  SST_ELI_DOCUMENT_PORTS(
    {"ipc_link", "Port for Inter Processor Communication", {}},
    {"core%(core_id)d-icache", "Ports connected to instruction cache", {}},
    {"core%(core_id)d-dcache", "Ports connected to data cache", {}},
    {"core%(core_id)d-ccache",
     "Ports connected to const cache (only for GPU core)",
     {}},
    {"core%(core_id)d-tcache",
     "Ports connected to texture cache (only for GPU core)",
     {}})

public:
  macsimComponent(SST::ComponentId_t id, SST::Params &params);
  void init(unsigned int phase);
  void setup();
  void finish();

private:
  macsimComponent();  // for serialization only
  macsimComponent(const macsimComponent &);  // do not implement
  void operator=(const macsimComponent &);  // do not implement

  void configureLinks(SST::Params &params, TimeConverter *tc);

  virtual bool ticReceived(Cycle_t);
  void handleInstructionCacheEvent(SimpleMem::Request *req);
  void handleDataCacheEvent(SimpleMem::Request *req);
  void handleConstCacheEvent(SimpleMem::Request *req);
  void handleTextureCacheEvent(SimpleMem::Request *req);
  void handleCubeEvent(SimpleMem::Request *req);

  string m_param_file;
  string m_trace_file;
  string m_output_dir;
  string m_command_line;

  string m_clock_freq;

  macsim_c *m_macsim;
  bool m_sim_running;
  bool m_acc_core;
  ACC_Type m_acc_type;
  bool m_cube_connected;
  bool m_debug_all;
  int64_t m_debug_addr;
  uint64_t m_mem_size;

  int m_operation_mode;
  bool m_triggered;
  SST::Link *m_ipc_link;

  // links
  uint32_t m_num_link;
  vector<Interfaces::SimpleMem *> m_instruction_cache_links;
  vector<Interfaces::SimpleMem *> m_data_cache_links;
  vector<Interfaces::SimpleMem *> m_const_cache_links;
  vector<Interfaces::SimpleMem *> m_texture_cache_links;
  Interfaces::SimpleMem *m_cube_link;

  // debugging
  vector<uint64_t> m_instruction_cache_request_counters;
  vector<uint64_t> m_instruction_cache_response_counters;
  vector<uint64_t> m_data_cache_request_counters;
  vector<uint64_t> m_data_cache_response_counters;
  vector<uint64_t> m_const_cache_request_counters;
  vector<uint64_t> m_const_cache_response_counters;
  vector<uint64_t> m_texture_cache_request_counters;
  vector<uint64_t> m_texture_cache_response_counters;

  // request queues
  vector<map<uint64_t, uint64_t>> m_instruction_cache_requests;
  vector<map<uint64_t, uint64_t>> m_data_cache_requests;
  vector<map<uint64_t, uint64_t>> m_const_cache_requests;
  vector<map<uint64_t, uint64_t>> m_texture_cache_requests;
  map<uint64_t, uint64_t> m_cube_requests;

  // response queues
  vector<set<uint64_t>> m_instruction_cache_responses;
  vector<set<uint64_t>> m_data_cache_responses;
  vector<set<uint64_t>> m_const_cache_responses;
  vector<set<uint64_t>> m_texture_cache_responses;
  set<uint64_t> m_cube_responses;

  // callback functions for sending requests; these will be called by macsim
  void sendInstructionCacheRequest(int, uint64_t, uint64_t, int);
#ifdef USE_VAULTSIM_HMC
  void sendDataCacheRequest(int, uint64_t, uint64_t, int, int, uint32_t,
                            uint64_t);
#else
  void sendDataCacheRequest(int, uint64_t, uint64_t, int, int);
#endif
  void sendConstCacheRequest(int, uint64_t, uint64_t, int);
  void sendTextureCacheRequest(int, uint64_t, uint64_t, int);
  void sendCubeRequest(uint64_t, uint64_t, int, int);

  // callback functions for strobbing responses; these will be called by macsim
  bool strobeInstructionCacheRespQ(int, uint64_t);
  bool strobeDataCacheRespQ(int, uint64_t);
  bool strobeConstCacheRespQ(int, uint64_t);
  bool strobeTextureCacheRespQ(int, uint64_t);
  bool strobeCubeRespQ(uint64_t);

  Output *m_dbg;
  Cycle_t m_cycle;
};  // class macsimComponent
}
}
#endif  // MACSIM_COMPONENT_H
