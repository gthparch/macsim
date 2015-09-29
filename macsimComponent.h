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

enum DebugLevel { ERROR, WARNING, INFO, L0, L1, L2, L3, L4, L5, L6 }; // debug level
enum OperationMode { MASTER, SLAVE }; // mode

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

    void configureLinks(SST::Params& params);

    virtual bool ticReceived(Cycle_t);
    void handleIcacheEvent(SimpleMem::Request *req);
    void handleDcacheEvent(SimpleMem::Request *req);
    void handleCubeEvent(SimpleMem::Request *req);
		
    string m_param_file;
    string m_trace_file;
    string m_output_dir;
    string m_command_line;

    macsim_c* m_macsim;
    bool m_sim_running;
    bool m_cube_connected;
    uint64_t m_mem_size;

    int m_operation_mode;
    bool m_triggered;
    SST::Link *m_ipc_link;

    unsigned int m_num_link;
    vector<Interfaces::SimpleMem*> m_icache_links;
    vector<Interfaces::SimpleMem*> m_dcache_links;
    Interfaces::SimpleMem *m_cube_link;

    // debugging 
    vector<uint64_t> m_icache_request_counters;
    vector<uint64_t> m_icache_response_counters;
    vector<uint64_t> m_dcache_request_counters;
    vector<uint64_t> m_dcache_response_counters;

    vector<map<uint64_t, uint64_t>> m_icache_requests;
    vector<map<uint64_t, uint64_t>> m_dcache_requests;
    vector<set<uint64_t>> m_icache_responses;
    vector<set<uint64_t>> m_dcache_responses;
    map<uint64_t, uint64_t> m_cube_requests;
    set<uint64_t> m_cube_responses;

    void sendInstReq(int,uint64_t,uint64_t,int);
#ifdef USE_VAULTSIM_HMC 
    void sendDataReq(int,uint64_t,uint64_t,int,int,uint8_t);
#else
    void sendDataReq(int,uint64_t,uint64_t,int,int);
#endif
    void sendCubeReq(uint64_t,uint64_t,int,int);
    bool strobeInstRespQ(int,uint64_t);
    bool strobeDataRespQ(int,uint64_t);
    bool strobeCubeRespQ(uint64_t);

    Output* m_dbg;
    Cycle_t m_cycle;
}; // class macsimComponent
}}
#endif // MACSIM_COMPONENT_H
