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

    //Interfaces::SimpleMem *icache_link;
    //Interfaces::SimpleMem *dcache_link;
    unsigned int m_num_links;
    std::vector<Interfaces::SimpleMem*> m_icache_links;
    std::vector<Interfaces::SimpleMem*> m_dcache_links;
    Interfaces::SimpleMem *m_cube_link;

    std::vector<std::map<uint64_t, uint64_t>> m_icache_requests;
    std::vector<std::map<uint64_t, uint64_t>> m_dcache_requests;
    std::map<uint64_t, uint64_t> m_cube_requests;
    std::vector<std::set<uint64_t>> m_icache_responses;
    std::vector<std::set<uint64_t>> m_dcache_responses;
    set<uint64_t> m_cube_responses;

    void sendInstReq(int,uint64_t,uint64_t,int);
    void sendDataReq(int,uint64_t,uint64_t,int,int);
    void sendCubeReq(uint64_t,uint64_t,int,int);
    bool strobeInstRespQ(int,uint64_t);
    bool strobeDataRespQ(int,uint64_t);
    bool strobeCubeRespQ(uint64_t);

    Output* m_dbg;
    Cycle_t m_cycle;
};

}}
#endif // MACSIM_COMPONENT_H
