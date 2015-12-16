#include <sst_config.h>

#include "sst/core/serialization.h"
#include "sst/core/element.h"
#include "sst/core/component.h"

#include "macsimComponent.h"

using namespace SST;
using namespace SST::MacSim;

static Component* create_macsimComponent(SST::ComponentId_t id, SST::Params& params)
{
  return new macsimComponent(id, params);
}

static const ElementInfoParam macsim_params[] = {
  {"param_file", "params.in", NULL},
  {"trace_file", "trace_file_list", NULL},
  {"output_dir", "output (stats, params.out, etc.) directory", NULL},
  {"command_line", "specify knobs, if any", NULL},
  {"cube_connected", "Depricated", "0"},
  {"operation_mode", "0: Master, 1: Slave", "0"},
  {"frequency", "clock frequency", "1GHz"},
  {"mem_size", "memory size in bytes. E.g., 1073741824 (=1024*1024*1024) for 1GB", "1073741824"},
  {"ptx_core", "1: GPU core, 0: CPU core", "0"},
  {"num_link", "this should match to the number of cores and/or L1 caches", "1"},
  {"debug", "0:No debugging, 1:stdout, 2:stderr, 3:file", "0"},
  {"debug_level", "debugging level: 0 to 9", "8"},
  {"debug_addr", "address (in decimal) to be debugged, if not specified or specified as -1, debug output for all addresses will be printed", "-1"},
  {NULL, NULL, NULL}
};

static const ElementInfoPort macsim_ports[] = {
  {"ipc_link", "Port for Inter Processor Communication", NULL},
  {"core%(core_id)d-icache", "Ports connected to instruction cache", NULL},
  {"core%(core_id)d-dcache", "Ports connected to data cache", NULL},
  {"core%(core_id)d-ccache", "Ports connected to const cache (only for GPU core)", NULL},
  {"core%(core_id)d-tcache", "Ports connected to texture cache (only for GPU core)", NULL},
  {NULL, NULL, NULL}
};

static const ElementInfoComponent components[] = {
  { 
    "macsimComponent",
    "MacSim Simulator",
    NULL,
    create_macsimComponent,
    macsim_params,
    macsim_ports,
    COMPONENT_CATEGORY_PROCESSOR
  },
	{ NULL, NULL, NULL, NULL, NULL, NULL, 0}
};

extern "C" {
  ElementLibraryInfo macsimComponent_eli = {
    "macsimComponent",
    "MacSim Simulator",
    components,
  };
}


