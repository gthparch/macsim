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
#include "src/all_knobs.h"

#include "macsimEvent.h"
#include "macsimComponent.h"

#define MSC_DEBUG(fmt, args...) m_dbg->debug(CALL_INFO,INFO,0,fmt,##args)

using namespace SST;
using namespace SST::MacSim;

macsimComponent::macsimComponent(ComponentId_t id, Params& params) : Component(id)
{
  m_dbg = new Output();

  int debug_level = params.find("debug_level", (int)DebugLevel::ERROR);
  if (debug_level < DebugLevel::ERROR || debug_level > DebugLevel::L6)
    m_dbg->fatal(CALL_INFO, -1, "Debugging level must be between 0 and 9. \n");

  m_debug_addr = (uint64_t)params.find("debug_addr", -1);
  if (m_debug_addr == -1)
    m_debug_all = true;

  string prefix = "[" + getName() + "] ";
  int debug_output = params.find("debug", (int)Output::NONE);
  m_dbg->init(prefix, debug_level, 0, (Output::output_location_t)debug_output);
  MSC_DEBUG("------- Initializing -------\n");

  bool found;
  m_param_file = params.find("param_file", found);
  if (!found)
    m_dbg->fatal(CALL_INFO, -1, "Couldn't find params.in file\n");
  //
  m_trace_file = params.find("trace_file", found);
  if (!found)
    m_dbg->fatal(CALL_INFO, -1, "Couldn't find trace_file_list file\n");
  //
  m_output_dir = params.find("output_dir", found);
  if (!found)
    m_dbg->fatal(CALL_INFO, -1, "Couldn't find statistics output directory parameter");

  m_command_line = params.find("command_line", found);

  m_ptx_core = params.find("ptx_core", 0);
  m_num_link = params.find("num_link", 1);
  configureLinks(params);

  m_cube_connected = params.find("cube_connected", 0);
  if (m_cube_connected) {
    m_cube_link = dynamic_cast<Interfaces::SimpleMem*>(loadModuleWithComponent("memHierarchy.memInterface", this, params));
    if (!m_cube_link) m_dbg->fatal(CALL_INFO, -1, "Unable to load Module as memory\n");
    m_cube_link->initialize("cube_link", new Interfaces::SimpleMem::Handler<macsimComponent>(this, &macsimComponent::handleCubeEvent));
  } else {
    m_cube_link = NULL;
  }

  m_clock_freq = params.find("frequency", found);
  registerClock(m_clock_freq, new Clock::Handler<macsimComponent>(this, &macsimComponent::ticReceived));

  m_mem_size = params.find("mem_size", 1*1024*1024*1024);
  MSC_DEBUG("Size of memory address space: 0x%" PRIx64 "\n", m_mem_size);

  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  m_macsim = new macsim_c();
  m_sim_running = false;

  // When MASTER mode, MacSim begins execution right away.
  // When SLAVE mode, MacSim awaits trigger event to arrive, which will cause MacSim to begin execution of a specified kernel.
  //   Upon completion, MacSim will return an event to another SST component.
  m_operation_mode = params.find("operation_mode", (int)OperationMode::MASTER);
  if (m_operation_mode == OperationMode::MASTER) {
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
  for (unsigned int l = 0 ; l < m_num_link; ++l) {
    Interfaces::SimpleMem *icache_link = dynamic_cast<Interfaces::SimpleMem*>(loadModuleWithComponent("memHierarchy.memInterface", this, params));
    if (!icache_link) m_dbg->fatal(CALL_INFO, -1, "Unable to load Module as memory\n");
    Interfaces::SimpleMem *dcache_link = dynamic_cast<Interfaces::SimpleMem*>(loadModuleWithComponent("memHierarchy.memInterface", this, params));
    if (!dcache_link) m_dbg->fatal(CALL_INFO, -1, "Unable to load Module as memory\n");

    std::ostringstream icache_link_name;
    icache_link_name << "core" << l << "-icache";
    icache_link->initialize(icache_link_name.str(), new Interfaces::SimpleMem::Handler<macsimComponent>(this, &macsimComponent::handleInstructionCacheEvent));
    m_instruction_cache_links.push_back(icache_link);
    m_instruction_cache_requests.push_back(std::map<uint64_t, uint64_t>());
    m_instruction_cache_responses.push_back(std::set<uint64_t>());

    std::ostringstream dcache_link_name;
    dcache_link_name << "core" << l << "-dcache";
    dcache_link->initialize(dcache_link_name.str(), new Interfaces::SimpleMem::Handler<macsimComponent>(this, &macsimComponent::handleDataCacheEvent));
    m_data_cache_links.push_back(dcache_link);
    m_data_cache_requests.push_back(std::map<uint64_t, uint64_t>());
    m_data_cache_responses.push_back(std::set<uint64_t>());

    if (m_ptx_core) {
      Interfaces::SimpleMem *ccache_link = dynamic_cast<Interfaces::SimpleMem*>(loadModuleWithComponent("memHierarchy.memInterface", this, params));
      if (!ccache_link) m_dbg->fatal(CALL_INFO, -1, "Unable to load Module as memory\n");
      Interfaces::SimpleMem *tcache_link = dynamic_cast<Interfaces::SimpleMem*>(loadModuleWithComponent("memHierarchy.memInterface", this, params));
      if (!tcache_link) m_dbg->fatal(CALL_INFO, -1, "Unable to load Module as memory\n");

      std::ostringstream ccache_link_name;
      ccache_link_name << "core" << l << "-ccache";
      ccache_link->initialize(ccache_link_name.str(), new Interfaces::SimpleMem::Handler<macsimComponent>(this, &macsimComponent::handleConstCacheEvent));
      m_const_cache_links.push_back(ccache_link);
      m_const_cache_requests.push_back(std::map<uint64_t, uint64_t>());
      m_const_cache_responses.push_back(std::set<uint64_t>());

      std::ostringstream tcache_link_name;
      tcache_link_name << "core" << l << "-tcache";
      tcache_link->initialize(tcache_link_name.str(), new Interfaces::SimpleMem::Handler<macsimComponent>(this, &macsimComponent::handleTextureCacheEvent));
      m_texture_cache_links.push_back(tcache_link);
      m_texture_cache_requests.push_back(std::map<uint64_t, uint64_t>());
      m_texture_cache_responses.push_back(std::set<uint64_t>());
    }
  }

  m_instruction_cache_request_counters = std::vector<uint64_t>(m_num_link, 0);
  m_instruction_cache_response_counters = std::vector<uint64_t>(m_num_link, 0);
  m_data_cache_request_counters = std::vector<uint64_t>(m_num_link, 0);
  m_data_cache_response_counters = std::vector<uint64_t>(m_num_link, 0);

  if (m_ptx_core) {
    m_const_cache_request_counters = std::vector<uint64_t>(m_num_link, 0);
    m_const_cache_response_counters = std::vector<uint64_t>(m_num_link, 0);
    m_texture_cache_request_counters = std::vector<uint64_t>(m_num_link, 0);
    m_texture_cache_response_counters = std::vector<uint64_t>(m_num_link, 0);
  }
}

void macsimComponent::init(unsigned int phase)
{
  if (!phase) {
    for (unsigned int l = 0 ; l < m_num_link; ++l) {
      m_instruction_cache_links[l]->sendInitData(new Interfaces::StringEvent("SST::MemHierarchy::MemEvent"));
      m_data_cache_links[l]->sendInitData(new Interfaces::StringEvent("SST::MemHierarchy::MemEvent"));
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

  CallbackSendInstructionCacheRequest* sir =
    new Callback<macsimComponent,void,int,uint64_t,uint64_t,int>(this, &macsimComponent::sendInstructionCacheRequest);
  CallbackSendDataCacheRequest* sdr =
#ifdef USE_VAULTSIM_HMC
    new Callback<macsimComponent,void,int,uint64_t,uint64_t,int,int,uint8_t,uint64_t>(this, &macsimComponent::sendDataCacheRequest);
#else
    new Callback<macsimComponent,void,int,uint64_t,uint64_t,int,int>(this, &macsimComponent::sendDataCacheRequest);
#endif
  CallbackStrobeInstructionCacheRespQ* sirq =
    new Callback<macsimComponent,bool,int,uint64_t>(this, &macsimComponent::strobeInstructionCacheRespQ);
  CallbackStrobeDataCacheRespQ* sdrq =
    new Callback<macsimComponent,bool,int,uint64_t>(this, &macsimComponent::strobeDataCacheRespQ);

  if (m_ptx_core) {
    CallbackSendConstCacheRequest* scr =
      new Callback<macsimComponent,void,int,uint64_t,uint64_t,int>(this, &macsimComponent::sendConstCacheRequest);
    CallbackSendTextureCacheRequest* str =
      new Callback<macsimComponent,void,int,uint64_t,uint64_t,int>(this, &macsimComponent::sendTextureCacheRequest);
    CallbackStrobeConstCacheRespQ* scrq =
      new Callback<macsimComponent,bool,int,uint64_t>(this, &macsimComponent::strobeConstCacheRespQ);
    CallbackStrobeTextureCacheRespQ* strq =
      new Callback<macsimComponent,bool,int,uint64_t>(this, &macsimComponent::strobeTextureCacheRespQ);

    m_macsim->registerCallback(sir, sdr, scr, str, sirq, sdrq, scrq, strq);
  } else {
    m_macsim->registerCallback(sir, sdr, sirq, sdrq);
  }

  if (m_cube_connected) {
    CallbackSendCubeRequest* scr =
      new Callback<macsimComponent,void,uint64_t,uint64_t,int,int>(this, &macsimComponent::sendCubeRequest);
    CallbackStrobeCubeRespQ* scrq =
      new Callback<macsimComponent,bool,uint64_t>(this, &macsimComponent::strobeCubeRespQ);

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
      if (m_ptx_core) {
        MSC_DEBUG("Core[%2d] I$: (%lu, %lu), D$: (%lu, %lu) C$: (%lu, %lu), T$: (%lu, %lu)\n", l,
          m_instruction_cache_request_counters[l], m_instruction_cache_response_counters[l],
          m_data_cache_request_counters[l], m_data_cache_response_counters[l],
          m_const_cache_request_counters[l], m_const_cache_response_counters[l],
          m_texture_cache_request_counters[l], m_texture_cache_response_counters[l]);
      } else {
        MSC_DEBUG("Core[%2d] I$: (%lu, %lu), D$: (%lu, %lu)\n", l,
          m_instruction_cache_request_counters[l], m_instruction_cache_response_counters[l],
          m_data_cache_request_counters[l], m_data_cache_response_counters[l]);
      }
    }
  }

  // Still has more cycles to run
  if (m_sim_running) {
    return false;
  }
  // Let SST know that this component is done and could be terminated
  else {
    if (m_operation_mode == OperationMode::SLAVE) {
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
void macsimComponent::sendInstructionCacheRequest(int core_id, uint64_t key, uint64_t addr, int size)
{
#ifndef USE_VAULTSIM_HMC
  SimpleMem::Request *req = new SimpleMem::Request(SimpleMem::Request::Read, addr & (m_mem_size-1), size);
#else
  SimpleMem::Request *req = new SimpleMemHMCExtension::HMCRequest(SimpleMem::Request::Read, addr & (m_mem_size-1), size, 0, HMC_NONE);
#endif
  m_instruction_cache_links[core_id]->sendRequest(req);
  m_instruction_cache_request_counters[core_id]++;
  m_instruction_cache_requests[core_id].insert(make_pair(req->id, key));

  if (m_debug_all || m_debug_addr == addr) {
    MSC_DEBUG("I$[%d] request sent: addr = %#" PRIx64 " (orig addr = %#" PRIx64 ", size = %d\n", core_id, addr & 0x3FFFFFFF, addr, size);
  }
}

bool macsimComponent::strobeInstructionCacheRespQ(int core_id, uint64_t key)
{
  auto I = m_instruction_cache_responses[core_id].find(key);
  if (I == m_instruction_cache_responses[core_id].end())
    return false;
  else {
    m_instruction_cache_responses[core_id].erase(I);
    return true;
  }
}

// incoming events are scanned and deleted
void macsimComponent::handleInstructionCacheEvent(Interfaces::SimpleMem::Request *req)
{
  for (unsigned int l = 0; l < m_num_link; ++l) {
    auto i = m_instruction_cache_requests[l].find(req->id);
    if (m_instruction_cache_requests[l].end() == i) {
      // No matching request
      continue;
    } else {
      if (m_debug_all || m_debug_addr == req->addr) {
        MSC_DEBUG("I$[%d] response arrived: addr = %#" PRIx64 "\n", l, req->addr);
      }
      m_instruction_cache_responses[l].insert(i->second);
      m_instruction_cache_response_counters[l]++;
      m_instruction_cache_requests[l].erase(i);
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
void macsimComponent::sendDataCacheRequest(int core_id, uint64_t key, uint64_t addr, int size, int type)
{
  bool doWrite = isStore((Mem_Type)type);
  SimpleMem::Request *req =
    new SimpleMem::Request(doWrite ? SimpleMem::Request::Write : SimpleMem::Request::Read, addr & (m_mem_size-1), size);
  m_data_cache_links[core_id]->sendRequest(req);
  m_data_cache_request_counters[core_id]++;
  m_data_cache_requests[core_id].insert(make_pair(req->id, key));
  if (m_debug_all || m_debug_addr == addr) {
    MSC_DEBUG("D$[%d] request sent: addr = %#" PRIx64 " (orig addr = %#" PRIx64 "), %s, size = %d\n", core_id, addr & 0x3FFFFFFF, addr, doWrite ? "write" : "read", size);
  }
}
#else
void macsimComponent::sendDataCacheRequest(int core_id, uint64_t key, uint64_t addr, int size, int type,uint8_t hmc_type=0,uint64_t trans_id=0)
{
  bool doWrite = isStore((Mem_Type)type);
  unsigned flag = 0;
  if ( (hmc_type & 0b10000000) != 0) {
    flag = SimpleMem::Request::F_NONCACHEABLE;
    hmc_type = hmc_type & 0b01111111;
  }
  SimpleMem::Request *req =
    new SimpleMemHMCExtension::HMCRequest(doWrite ? SimpleMem::Request::Write : SimpleMem::Request::Read, addr & (m_mem_size-1), size, flag, hmc_type, trans_id);
  //if (hmc_type!=0) cout<<"HMC: "<<(unsigned)hmc_type<<"\t trans: "<<trans_id
  //    <<" isStore: "<<doWrite<<endl;
  m_data_cache_links[core_id]->sendRequest(req);
  m_data_cache_request_counters[core_id]++;
  m_data_cache_requests[core_id].insert(make_pair(req->id, key));
  if (m_debug_all || m_debug_addr == addr) {
    MSC_DEBUG("D$[%d] request sent: addr = %#" PRIx64 " (orig addr = %#" PRIx64 "), %s, size = %d\n",
      core_id, addr & 0x3FFFFFFF, addr, doWrite ? "write" : "read", size);
  }
}
#endif

bool macsimComponent::strobeDataCacheRespQ(int core_id, uint64_t key)
{
  auto I = m_data_cache_responses[core_id].find(key);
  if (I == m_data_cache_responses[core_id].end())
    return false;
  else {
    m_data_cache_responses[core_id].erase(I);
    return true;
  }
}

// incoming events are scanned and deleted
void macsimComponent::handleDataCacheEvent(Interfaces::SimpleMem::Request *req)
{
  for (unsigned int l = 0; l < m_num_link; ++l) {
    auto i = m_data_cache_requests[l].find(req->id);
    if (m_data_cache_requests[l].end() == i) {
      // No matching request
      continue;
    } else {
      if (m_debug_all || m_debug_addr == req->addr) {
        MSC_DEBUG("D$[%d] response arrived: addr = %#" PRIx64 ", size = %lu\n", l, req->addr, req->size);
      }
      m_data_cache_responses[l].insert(i->second);
      m_data_cache_response_counters[l]++;
      m_data_cache_requests[l].erase(i);
      break;
    }
  }
#ifdef USE_VAULTSIM_HMC
  SimpleMemHMCExtension::HMCRequest *req2 = static_cast<SimpleMemHMCExtension::HMCRequest *>(req);
  delete req2;
#else
  delete req;
#endif
}

////////////////////////////////////////
//
// Const-Cache related routines go here
//
////////////////////////////////////////
void macsimComponent::sendConstCacheRequest(int core_id, uint64_t key, uint64_t addr, int size)
{
  SimpleMem::Request *req = new SimpleMem::Request(SimpleMem::Request::Read, addr & (m_mem_size-1), size);
  m_const_cache_links[core_id]->sendRequest(req);
  m_const_cache_request_counters[core_id]++;
  m_const_cache_requests[core_id].insert(make_pair(req->id, key));

  if (m_debug_all || m_debug_addr == addr) {
    MSC_DEBUG("C$[%d] request sent: addr = %#" PRIx64 " (orig addr = %#" PRIx64 ", size = %d\n", core_id, addr & (m_mem_size-1), addr, size);
  }
}

bool macsimComponent::strobeConstCacheRespQ(int core_id, uint64_t key)
{
  auto I = m_const_cache_responses[core_id].find(key);
  if (I == m_const_cache_responses[core_id].end()) // Response has not yet arrived
    return false;
  else { // Response has arrived
    m_const_cache_responses[core_id].erase(I);
    return true;
  }
}

// incoming events are scanned and deleted
void macsimComponent::handleConstCacheEvent(Interfaces::SimpleMem::Request *req)
{
  for (unsigned int l = 0; l < m_num_link; ++l) {
    auto i = m_const_cache_requests[l].find(req->id);
    if (m_const_cache_requests[l].end() == i) { // No matching request
      continue;
    } else {
      if (m_debug_all || m_debug_addr == req->addr) {
        MSC_DEBUG("C$[%d] response arrived: addr = %#" PRIx64 ", size = %lu\n", l, req->addr, req->size);
      }
      m_const_cache_responses[l].insert(i->second);
      m_const_cache_response_counters[l]++;
      m_const_cache_requests[l].erase(i);
      break;
    }
  }

  delete req;
}

////////////////////////////////////////
//
// Texture-Cache related routines go here
//
////////////////////////////////////////
void macsimComponent::sendTextureCacheRequest(int core_id, uint64_t key, uint64_t addr, int size)
{
  SimpleMem::Request *req = new SimpleMem::Request(SimpleMem::Request::Read, addr & (m_mem_size-1), size);
  m_texture_cache_links[core_id]->sendRequest(req);
  m_texture_cache_request_counters[core_id]++;
  m_texture_cache_requests[core_id].insert(make_pair(req->id, key));
  MSC_DEBUG("T$[%d] request sent: addr = %#" PRIx64 " (orig addr = %#" PRIx64 ", size = %d\n", core_id, addr & (m_mem_size-1), addr, size);
}

bool macsimComponent::strobeTextureCacheRespQ(int core_id, uint64_t key)
{
  auto I = m_texture_cache_responses[core_id].find(key);
  if (I == m_texture_cache_responses[core_id].end()) // Response has not yet arrived
    return false;
  else { // Response has arrived
    m_texture_cache_responses[core_id].erase(I);
    return true;
  }
}

// incoming events are scanned and deleted
void macsimComponent::handleTextureCacheEvent(Interfaces::SimpleMem::Request *req)
{
  for (unsigned int l = 0; l < m_num_link; ++l) {
    auto i = m_texture_cache_requests[l].find(req->id);
    if (m_texture_cache_requests[l].end() == i) { // No matching request
      continue;
    } else {
      if (m_debug_all || m_debug_addr == req->addr) {
        MSC_DEBUG("T$[%d] response arrived: addr = %#" PRIx64 ", size = %lu\n", l, req->addr, req->size);
      }
      m_texture_cache_responses[l].insert(i->second);
      m_texture_cache_response_counters[l]++;
      m_texture_cache_requests[l].erase(i);
      break;
    }
  }

  delete req;
}

////////////////////////////////////////
//
// VaultSim related routines go here
//
////////////////////////////////////////
void macsimComponent::sendCubeRequest(uint64_t key, uint64_t addr, int size, int type)
{
  bool doWrite = isStore((Mem_Type)type);
  SimpleMem::Request *req =
    new SimpleMem::Request(doWrite ? SimpleMem::Request::Write : SimpleMem::Request::Read, addr & 0x3FFFFFFF, size);
  m_cube_link->sendRequest(req);
  if (m_debug_all || m_debug_addr == addr) {
    MSC_DEBUG("Cube request sent: addr = %#" PRIx64 "(orig addr = %#" PRIx64 "), %s %s, size = %d\n",
      addr & 0x3FFFFFFF, addr, (type == -1) ? "instruction" : "data",  doWrite ? "write" : "read", size);
  }
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
    if (m_debug_all || m_debug_addr == req->addr) {
      MSC_DEBUG("Cube response arrived: addr = %#" PRIx64 ", size = %lu\n", req->addr, req->size);
    }
    m_cube_responses.insert(i->second);
    m_cube_requests.erase(i);
  }

  delete req;
}
