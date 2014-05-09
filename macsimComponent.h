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

namespace SST { namespace MacSim {

enum {ERROR, WARNING, INFO, L0, L1, L2, L3, L4, L5, L6}; // debug level

class macsimComponent : public SST::Component 
{
	public:
    macsimComponent(SST::ComponentId_t id, SST::Params& params);
    void init(unsigned int phase);
		void setup();
		void finish();
		
	private:
		macsimComponent();   // for serialization only
		macsimComponent(const macsimComponent&);   // do not implement
		void operator=(const macsimComponent&); // do not implement

    virtual bool ticReceived(Cycle_t);
    void handleIcacheEvent(SimpleMem::Request *req);
    void handleDcacheEvent(SimpleMem::Request *req);
    void handleCubeEvent(SimpleMem::Request *req);
		
    string paramFile;
    string traceFile;
    string outputDir;
    string commandLine;

    macsim_c* macsim;
    bool simRunning;
    bool cubeConnected;

    Interfaces::SimpleMem *icache_link;
    Interfaces::SimpleMem *dcache_link;
    Interfaces::SimpleMem *cube_link;

    map<uint64_t, uint64_t> icache_requests;
    map<uint64_t, uint64_t> dcache_requests;
    map<uint64_t, uint64_t> cube_requests;
    set<uint64_t> icache_responses;
    set<uint64_t> dcache_responses;
    set<uint64_t> cube_responses;

    void sendInstReq(uint64_t, uint64_t, int);
    void sendDataReq(uint64_t, uint64_t, int, int);
    void sendCubeReq(uint64_t, uint64_t, int, int);
    bool strobeInstRespQ(uint64_t);
    bool strobeDataRespQ(uint64_t);
    bool strobeCubeRespQ(uint64_t);

    Output* dbg;
    Cycle_t timestamp;
};

}}
#endif // MACSIM_COMPONENT_H
