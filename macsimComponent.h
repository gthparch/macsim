#ifndef MACSIM_COMPONENT_H
#define MACSIM_COMPONENT_H

#ifndef MACSIMCOMPONENT_DBG
#define MACSIMCOMPONENT_DBG 1
#endif

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
    void icacheHandleEvent(SimpleMem::Request *req);
    void dcacheHandleEvent(SimpleMem::Request *req);
		
    string paramFile;
    string traceFile;
    string outputDir;
    string commandLine;

    macsim_c* macsim;
    bool simRunning;

    Interfaces::SimpleMem *icache_link;
    Interfaces::SimpleMem *dcache_link;

    map<uint64_t, frontend_s*> icache_requests;
    map<frontend_s*, uint64_t> icache_responses;
    map<uint64_t, uop_c*> dcache_requests;
    map<uop_c*, uint64_t> dcache_responses;

    void sendInstReq(frontend_s*, uint64_t, int);
    bool strobeInstRespQ(frontend_s*);
    void sendDataReq(uop_c*);
    bool strobeDataRespQ(uop_c*);

    Output* dbg;
    Cycle_t timestamp;
};

}}
#endif // MACSIM_COMPONENT_H
