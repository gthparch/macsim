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
    {"paramFile", "", NULL},
    {"traceFile", "", NULL},
    {"outputDir", "", NULL},
    {"clockFreq", "Clock frequency", "1GHz"},
    {"printLocation", "Prints debug statements --0[No debugging], 1[STDOUT], 2[STDERR], 3[FILE]--", "0"},
    {"debugLevel", "Debugging level: 0 to 10", "8"},
    {NULL, NULL, NULL}
};

static const ElementInfoPort macsim_ports[] = {
    {"icache_link", "", NULL},
    {"dcache_link", "", NULL},
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


