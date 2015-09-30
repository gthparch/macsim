#include <sys/time.h>

#include <stdint.h>

#include <sst_config.h>
#include <sst/core/serialization/element.h>
#include <sst/core/element.h>
#include <sst/core/simulation.h>
#include <sst/core/params.h>

#include <sst/core/interfaces/stringEvent.h>
#include <sst/core/interfaces/simpleMem.h>

#ifdef USE_VAULTSIM_HMC  
#include <sst/elements/memHierarchy/simpleMemHMCExtension.h>
#endif

#include "src/global_defs.h"
#include "src/uop.h"
#include "src/frontend.h"

#include "macsimEvent.h"
#include "macsimComponent.h"

#define MSC_DEBUG(fmt, args...) m_dbg->debug(CALL_INFO,INFO,0,fmt,##args)

using namespace SST;
using namespace SST::MacSim;

macsimComponent::macsimComponent(ComponentId_t id, Params& params) : Component(id)
{
  m_dbg = new Output();

  int debug_level = params.find_integer("debug_level", DebugLevel::ERROR);
  if (debug_level < DebugLevel::ERROR || debug_level > DebugLevel::L6) 
    m_dbg->fatal(CALL_INFO, -1, "Debugging level must be between 0 and 9. \n");

  string prefix = "[" + getName() + "] ";
  m_dbg->init(prefix, debug_level, 0, (Output::output_location_t)params.find_integer("debug", Output::NONE));
  MSC_DEBUG("------- Initializing -------\n");

  if (params.find("param_file") == params.end())
    m_dbg->fatal(CALL_INFO, -1, "Couldn't find params.in file\n");
  if (params.find("trace_file") == params.end()) 
    m_dbg->fatal(CALL_INFO, -1, "Couldn't find trace_file_list file\n");
  if (params.find("output_dir") == params.end()) 
    m_dbg->fatal(CALL_INFO, -1, "Couldn't find statistics output directory parameter");

  m_param_file = string(params["param_file"]);
  m_trace_file = string(params["trace_file"]);
  m_output_dir = string(params["output_dir"]);

  if (params.find("command_line") != params.end()) 
    m_command_line = string(params["command_line"]);

  m_num_link = params.find_integer("num_link", 1);
  configureLinks(params);

  m_cube_connected = params.find_integer("cube_connected", 0);
  if (m_cube_connected) {
    m_cube_link = dynamic_cast<Interfaces::SimpleMem*>(loadModuleWithComponent("memHierarchy.memInterface", this, params));
    if (!m_cube_link) m_dbg->fatal(CALL_INFO, -1, "Unable to load Module as memory\n");
    m_cube_link->initialize("cube_link", new Interfaces::SimpleMem::Handler<macsimComponent>(this, &macsimComponent::handleCubeEvent));
  } else {
    m_cube_link = NULL;
  }

  string clock_freq = params.find_string("frequency", "1GHz");
  registerClock(clock_freq, new Clock::Handler<macsimComponent>(this, &macsimComponent::ticReceived));

  m_mem_size = params.find_integer("mem_size", 1*1024*1024*1024);
  MSC_DEBUG("Size of memory address space: 0x%" PRIx64 "\n", m_mem_size);

  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  m_macsim = new macsim_c();
  m_sim_running = false;

  // When MASTER mode, MacSim begins execution right away.
  // When SLAVE mode, MacSim awaits trigger event to arrive, which will cause MacSim to begin execution of a specified kernel.
  //   Upon completion, MacSim will return an event to another SST component.
  m_operation_mode = params.find_integer("operation_mode", MASTER);
  if (m_operation_mode == MASTER) {
    m_triggered = true;
    m_ipc_link = NULL;
    m_macsim->start();
  } else { // if (m_operation_mode == SLAVE)
    m_triggered = false;
    m_ipc_link = configureLink("ipc_link", "1 ns");
    m_macsim->halt();
  }
}

macsimComponent::macsimComponent() : Component(-1) {}  //for serialization only 

void macsimComponent::configureLinks(SST::Params& params) 
{
  Interfaces::SimpleMem *icache_link;
  Interfaces::SimpleMem *dcache_link;
  for (unsigned int l = 0 ; l < m_num_link; ++l) {
    icache_link = dynamic_cast<Interfaces::SimpleMem*>(loadModuleWithComponent("memHierarchy.memInterface", this, params));
    if (!icache_link) m_dbg->fatal(CALL_INFO, -1, "Unable to load Module as memory\n");
    dcache_link = dynamic_cast<Interfaces::SimpleMem*>(loadModuleWithComponent("memHierarchy.memInterface", this, params));
    if (!dcache_link) m_dbg->fatal(CALL_INFO, -1, "Unable to load Module as memory\n");

    std::ostringstream icache_link_name;
    icache_link_name << "core" << l << "-icache";
    icache_link->initialize(icache_link_name.str(), new Interfaces::SimpleMem::Handler<macsimComponent>(this, &macsimComponent::handleIcacheEvent));
    m_icache_links.push_back(icache_link);
    m_icache_requests.push_back(std::map<uint64_t, uint64_t>());
    m_icache_responses.push_back(std::set<uint64_t>());

    std::ostringstream dcache_link_name;
    dcache_link_name << "core" << l << "-dcache";
    dcache_link->initialize(dcache_link_name.str(), new Interfaces::SimpleMem::Handler<macsimComponent>(this, &macsimComponent::handleDcacheEvent));
    m_dcache_links.push_back(dcache_link);
    m_dcache_requests.push_back(std::map<uint64_t, uint64_t>());
    m_dcache_responses.push_back(std::set<uint64_t>());
  }

  m_icache_request_counters = std::vector<uint64_t>(m_num_link, 0);
  m_icache_response_counters = std::vector<uint64_t>(m_num_link, 0);
  m_dcache_request_counters = std::vector<uint64_t>(m_num_link, 0);
  m_dcache_response_counters = std::vector<uint64_t>(m_num_link, 0);
}

void macsimComponent::init(unsigned int phase)
{
  if (!phase) {
    for (unsigned int l = 0 ; l < m_num_link; ++l) {
      m_icache_links[l]->sendInitData(new Interfaces::StringEvent("SST::MemHierarchy::MemEvent"));
      m_dcache_links[l]->sendInitData(new Interfaces::StringEvent("SST::MemHierarchy::MemEvent"));
    }

    if (m_cube_connected)
      m_cube_link->sendInitData(new Interfaces::StringEvent("SST::MemHierarchy::MemEvent"));
  }
}

#include <boost/tokenizer.hpp>
int countTokens(string command_line)
{
  int count = 0;
  boost::char_separator<char> sep(" ");
  boost::tokenizer<boost::char_separator<char>> tokens(command_line, sep);
  for (auto I = tokens.begin(), E = tokens.end(); I != E; ++I) ++count;
  return count;
}

void macsimComponent::setup() 
{
  MSC_DEBUG("------- Setting up -------\n");

  // Build arguments
  char** argv = new char*[1+3+countTokens(m_command_line)];

  int argc = 0;
  argv[argc] = new char[                     4 ]; strcpy(argv[argc],                "sst"); argc++;
  argv[argc] = new char[ m_param_file.size()+1 ]; strcpy(argv[argc], m_param_file.c_str()); argc++;
  argv[argc] = new char[ m_trace_file.size()+1 ]; strcpy(argv[argc], m_trace_file.c_str()); argc++;
  argv[argc] = new char[ m_output_dir.size()+1 ]; strcpy(argv[argc], m_output_dir.c_str()); argc++;

  boost::char_separator<char> sep(" ");
  boost::tokenizer<boost::char_separator<char>> tokens(m_command_line, sep);
  for (auto I = tokens.begin(), E = tokens.end(); I != E; ++I) {
    string command = *I;
    argv[argc] = new char[ command.size()+1 ]; strcpy(argv[argc], command.c_str()); argc++;
  }

  // Pass paramaters to simulator if applicable
  m_macsim->initialize(argc, argv);

  // Cleanup
  for (int trav = 0; trav < argc; ++trav) 
    delete argv[trav];
  delete argv;

  CallbackSendInstReq* sir = 
    new Callback4<macsimComponent,void,int,uint64_t,uint64_t,int>(this, &macsimComponent::sendInstReq);
#ifdef USE_VAULTSIM_HMC  
  CallbackSendDataReq* sdr =
    new Callback6<macsimComponent,void,int,uint64_t,uint64_t,int,int,uint8_t>(this, &macsimComponent::sendDataReq);
#else
  CallbackSendDataReq* sdr =
    new Callback5<macsimComponent,void,int,uint64_t,uint64_t,int,int>(this, &macsimComponent::sendDataReq);
#endif
  CallbackStrobeInstRespQ* sirq = 
    new Callback2<macsimComponent,bool,int,uint64_t>(this, &macsimComponent::strobeInstRespQ);
  CallbackStrobeDataRespQ* sdrq =
    new Callback2<macsimComponent,bool,int,uint64_t>(this, &macsimComponent::strobeDataRespQ);

  m_macsim->registerCallback(sir, sdr, sirq, sdrq);

  if (m_cube_connected) {
    CallbackSendCubeReq* scr = 
      new Callback4<macsimComponent,void,uint64_t,uint64_t,int,int>(this, &macsimComponent::sendCubeReq);
    CallbackStrobeCubeRespQ* scrq = 
      new Callback1<macsimComponent,bool,uint64_t>(this, &macsimComponent::strobeCubeRespQ);

    m_macsim->registerCallback(scr, scrq);
  }
}

void macsimComponent::finish() 
{
  MSC_DEBUG("------- Finishing simulation -------\n");
  m_macsim->finalize();
}

/*******************************************************
 *  ticReceived
 *    return value
 *      true  : indicates the component finished; 
 *              no more clock events to the component
 *      false : component not done yet
 *******************************************************/
bool macsimComponent::ticReceived(Cycle_t) 
{
  ++m_cycle;

  if (!m_triggered) { // When SLAVE mode, wait until triggering event arrives.
    SST::Event* e = NULL;
    if ((e = m_ipc_link->recv())) {
      MacSimEvent *event = dynamic_cast<MacSimEvent*>(e);
      if (event->getType() == NONE) {
        m_dbg->fatal(CALL_INFO, -1, "macsimComponent got bad event from another component\n");
      }
      MSC_DEBUG("Received an event (%p) of type: %d at cycle %lu\n", event, event->getType(), m_cycle);
      if (event->getType() == START) {
        MSC_DEBUG("Beginning execution\n");
        m_triggered = true;
        m_macsim->start();
      }
    } else {
      return false;
    }
  }

	// Run a cycle of the simulator
	m_sim_running = m_macsim->run_a_cycle();

  // Debugging 
  if (m_cycle % 100000 == 0) {
    for (unsigned int l = 0 ; l < m_num_link; ++l) {
      MSC_DEBUG("Core[%2d] I$: (%lu, %lu), D$: (%lu, %lu)\n", l, 
          m_icache_request_counters[l], m_icache_response_counters[l], 
          m_dcache_request_counters[l], m_dcache_response_counters[l]);
    }
  }

	// Still has more cycles to run
	if (m_sim_running) {
		return false;
	}
	// Let SST know that this component is done and could be terminated
	else {
    if (m_operation_mode == SLAVE) {
      // Send a report event to another SST component upon completion
      MacSimEvent *event = new MacSimEvent(FINISHED);
      m_ipc_link->send(event);
    }

    primaryComponentOKToEndSim();
		return true;
	}
}

////////////////////////////////////////
//
// I-Cache related routines go here
//
////////////////////////////////////////
void macsimComponent::sendInstReq(int core_id, uint64_t key, uint64_t addr, int size)
{
  SimpleMem::Request *req = 
    new SimpleMem::Request(SimpleMem::Request::Read, addr & (m_mem_size-1), size);
  m_icache_links[core_id]->sendRequest(req);
  m_icache_request_counters[core_id]++;
  m_icache_requests[core_id].insert(make_pair(req->id, key));
  MSC_DEBUG("I$[%d] request sent: addr = %#" PRIx64 " (orig addr = %#" PRIx64 ", size = %d\n", 
      core_id, addr & 0x3FFFFFFF, addr, size);
}

bool macsimComponent::strobeInstRespQ(int core_id, uint64_t key)
{
  auto I = m_icache_responses[core_id].find(key);
  if (I == m_icache_responses[core_id].end()) 
    return false;
  else {
    m_icache_responses[core_id].erase(I);
    return true;
  }
}

// incoming events are scanned and deleted
void macsimComponent::handleIcacheEvent(Interfaces::SimpleMem::Request *req)
{
  for (unsigned int l = 0; l < m_num_link; ++l) {
    auto i = m_icache_requests[l].find(req->id);
    if (m_icache_requests[l].end() == i) {
      // No matching request
      continue;
    } else {
      MSC_DEBUG("I$[%d] response arrived\n", l);
      m_icache_responses[l].insert(i->second);
      m_icache_response_counters[l]++;
      m_icache_requests[l].erase(i);
      break;
    }
  }
  
  delete req;
}

////////////////////////////////////////
//
// D-Cache related routines go here
//
////////////////////////////////////////
inline bool isStore(Mem_Type type)
{
  switch (type) {
    case MEM_ST:
    case MEM_ST_LM:
    case MEM_ST_SM:
    case MEM_ST_GM:
      return true;
    default:
      return false;
  }
}
#ifndef USE_VAULTSIM_HMC
void macsimComponent::sendDataReq(int core_id, uint64_t key, uint64_t addr, int size, int type)
{
  bool doWrite = isStore((Mem_Type)type);
  SimpleMem::Request *req = 
    new SimpleMem::Request(doWrite ? SimpleMem::Request::Write : SimpleMem::Request::Read, addr & (m_mem_size-1), size);
  m_dcache_links[core_id]->sendRequest(req);
  m_dcache_request_counters[core_id]++;
  m_dcache_requests[core_id].insert(make_pair(req->id, key));
  MSC_DEBUG("D$[%d] request sent: addr = %#" PRIx64 " (orig addr = %#" PRIx64 "), %s, size = %d\n", 
      core_id, addr & 0x3FFFFFFF, addr, doWrite ? "write" : "read", size);
}
#else
void macsimComponent::sendDataReq(int core_id, uint64_t key, uint64_t addr, int size, int type,uint8_t hmc_type=0)
{
  bool doWrite = isStore((Mem_Type)type);
  SimpleMem::Request *req = 
    new SimpleMemHMCExtension::HMCRequest(doWrite ? SimpleMem::Request::Write : SimpleMem::Request::Read, addr & (m_mem_size-1), size, 
            (hmc_type==0)?0:SimpleMem::Request::F_NONCACHEABLE,hmc_type);
  m_dcache_links[core_id]->sendRequest(req);
  m_dcache_request_counters[core_id]++;
  m_dcache_requests[core_id].insert(make_pair(req->id, key));
  MSC_DEBUG("D$[%d] request sent: addr = %#" PRIx64 " (orig addr = %#" PRIx64 "), %s, size = %d\n", 
      core_id, addr & 0x3FFFFFFF, addr, doWrite ? "write" : "read", size);
}
#endif

bool macsimComponent::strobeDataRespQ(int core_id, uint64_t key)
{
  auto I = m_dcache_responses[core_id].find(key);
  if (I == m_dcache_responses[core_id].end()) 
    return false;
  else {
    m_dcache_responses[core_id].erase(I);
    return true;
  }
}

// incoming events are scanned and deleted
void macsimComponent::handleDcacheEvent(Interfaces::SimpleMem::Request *req)
{
  for (unsigned int l = 0; l < m_num_link; ++l) {
    auto i = m_dcache_requests[l].find(req->id);
    if (m_dcache_requests[l].end() == i) {
      // No matching request
      continue;
    } else {
      MSC_DEBUG("D$[%d] response arrived: addr = %#" PRIx64 ", size = %lu\n", l, req->addr, req->size);
      m_dcache_responses[l].insert(i->second);
      m_dcache_response_counters[l]++;
      m_dcache_requests[l].erase(i);
      break;
    }
  }
#ifdef USE_VAULTSIM_HMC
  SimpleMemHMCExtension::HMCRequest *req2 =
    static_cast<SimpleMemHMCExtension::HMCRequest *>(req);
  delete req2;
#else  
  delete req;
#endif  
}

////////////////////////////////////////
//
// VaultSim related routines go here
//
////////////////////////////////////////
void macsimComponent::sendCubeReq(uint64_t key, uint64_t addr, int size, int type)
{
  bool doWrite = isStore((Mem_Type)type);
  SimpleMem::Request *req = 
    new SimpleMem::Request(doWrite ? SimpleMem::Request::Write : SimpleMem::Request::Read, addr & 0x3FFFFFFF, size);
  m_cube_link->sendRequest(req);
  MSC_DEBUG("Cube request sent: addr = %#" PRIx64 "(orig addr = %#" PRIx64 "), %s %s, size = %d\n", 
      addr & 0x3FFFFFFF, addr, (type == -1) ? "instruction" : "data",  doWrite ? "write" : "read", size);
  m_cube_requests.insert(make_pair(req->id, key));
}

bool macsimComponent::strobeCubeRespQ(uint64_t key)
{
  auto I = m_cube_responses.find(key);
  if (I == m_cube_responses.end()) 
    return false;
  else {
    m_cube_responses.erase(I);
    return true;
  }
}

// incoming events are scanned and deleted
void macsimComponent::handleCubeEvent(Interfaces::SimpleMem::Request *req)
{
  auto i = m_cube_requests.find(req->id);
  if (m_cube_requests.end() == i) {
    // No matching request
    m_dbg->fatal(CALL_INFO, -1, "Event (%#" PRIx64 ") not found!\n", req->id);
  } else {
    MSC_DEBUG("Cube response arrived\n");
    m_cube_responses.insert(i->second);
    m_cube_requests.erase(i);
  }

  delete req;
}
