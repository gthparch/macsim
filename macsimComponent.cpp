#include <sys/time.h>

#include <stdint.h>
#include <string.h>
#include <algorithm>

// #include <sst/core/sst_config.h> // Th  is include is REQUIRED for all implementation files
// #include <sst_config.h>           // FIXME: one of these should be removed
// #include <sst/core/simulation.h>
// #include <sst/core/params.h>

// #include <sst/core/interfaces/stringEvent.h>
// #include <sst/core/interfaces/simpleMem.h>

#include "src/global_defs.h"
#include "src/uop.h"
#include "src/frontend.h"
#include "src/all_knobs.h"

#include "macsimEvent.h"
#include "macsimComponent.h"

#define MSC_DEBUG(fmt, args...) m_dbg->debug(CALL_INFO, INFO, 0, fmt, ##args)

using namespace SST;
using namespace SST::MacSim;

macsimComponent::macsimComponent(ComponentId_t id, Params& params)
  : Component(id) {
  m_dbg = new Output();

  int debug_level = params.find<int>("debug_level", (int)DebugLevel::ERROR);
  if (debug_level < DebugLevel::ERROR || debug_level > DebugLevel::L6)
    m_dbg->fatal(CALL_INFO, -1, "Debugging level must be between 0 and 9. \n");

  m_debug_addr = params.find<int64_t>("debug_addr", -1);
  if (m_debug_addr == -1) m_debug_all = true;

  string prefix = "[" + getName() + "] ";
  int debug_output = params.find<int>("debug", (int)Output::NONE);
  m_dbg->init(prefix, debug_level, 0, (Output::output_location_t)debug_output);
  MSC_DEBUG("------- Initializing -------\n");

  bool found;
  m_param_file = params.find<string>("param_file", found);
  if (!found) m_dbg->fatal(CALL_INFO, -1, "Couldn't find params.in file\n");
  //
  m_trace_file = params.find<string>("trace_file", found);
  if (!found)
    m_dbg->fatal(CALL_INFO, -1, "Couldn't find trace_file_list file\n");
  //
  m_output_dir = params.find<string>("output_dir", found);
  if (!found)
    m_dbg->fatal(CALL_INFO, -1,
                 "Couldn't find statistics output directory parameter");

  m_command_line = params.find<string>("command_line", found);

  m_clock_freq = params.find<string>("frequency", found);
  if (!found) m_dbg->fatal(CALL_INFO, -1, "Couldn't find frequency parameter\n");

  if (params.find<bool>("ptx_core", 0)) {
    m_acc_type = PTX_CORE;
    m_acc_core = 1;
  } else if (params.find<bool>("igpu_core", 0)) {
    m_acc_type = IGPU_CORE;
    m_acc_core = 1;
  // } else if (params.find<bool>("nvbit_core", 0)) {    // FIXME: 
  //   m_acc_type = NVBIT_CORE;
  //   m_acc_core = 1;
    
  } else {
    m_acc_core = 0;
    m_acc_type = NO_ACC;
  }
  m_num_link = params.find<uint32_t>("num_link", 1);

  m_mem_size = params.find<uint64_t>("mem_size", 1 * 1024 * 1024 * 1024);
  MSC_DEBUG("Memory address space: %" PRId64 " Bytes (0x%" PRIx64 " - 0x%" PRIx64 ")\n", m_mem_size, 0x0UL, m_mem_size - 1);

  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  // Register clock handler
  tc = registerClock(
    m_clock_freq,
    new Clock::Handler<macsimComponent>(this, &macsimComponent::ticReceived));

  // Configure links
  configureLinks(params, tc);

  // Show link information
  MSC_DEBUG("Number of links: %d\n", m_num_link);
  for (unsigned int l = 0; l < m_num_link; ++l) {
    MSC_DEBUG("-- Core: %d --------\n", l);
    MSC_DEBUG("  I cache link: %s\n", m_instruction_cache_links[l]->getName().c_str());
    MSC_DEBUG("  D cache link: %s\n", m_data_cache_links[l]->getName().c_str());
    if (m_acc_core) {
      MSC_DEBUG("  C cache link: %s\n", m_const_cache_links[l]->getName().c_str());
      MSC_DEBUG("  T cache link: %s\n", m_texture_cache_links[l]->getName().c_str());
    }
  }

  m_cube_connected = params.find<bool>("cube_connected", 0);
  if (m_cube_connected) {
    m_cube_link = loadUserSubComponent<Interfaces::StandardMem>(
      "cube_link", ComponentInfo::SHARE_NONE, tc,
      new Interfaces::StandardMem::Handler<macsimComponent>(
        this, &macsimComponent::handleCubeEvent));
    if (!m_cube_link) {
      Params interfaceParams;
      interfaceParams.insert("port", "cube_link");
      m_cube_link = loadAnonymousSubComponent<Interfaces::StandardMem>(
        "memHierarchy.standardInterface", "cube_link", 0,
        ComponentInfo::SHARE_PORTS | ComponentInfo::INSERT_STATS,
        interfaceParams, tc,
        new Interfaces::StandardMem::Handler<macsimComponent>(
          this, &macsimComponent::handleCubeEvent));
    }
  } else {
    m_cube_link = NULL;
  }

  m_macsim = new macsim_c();
  m_sim_running = false;

  // When MASTER mode, MacSim begins execution right away.
  // When SLAVE mode, MacSim awaits trigger event to arrive, which will cause MacSim to begin execution of a specified kernel.
  //   Upon completion, MacSim will return an event to another SST component.
  m_operation_mode =
    params.find<int>("operation_mode", (int)OperationMode::MASTER);
  MSC_DEBUG("Operation mode: %s\n", m_operation_mode == OperationMode::MASTER ? "MASTER" : "SLAVE");
  if (m_operation_mode == OperationMode::MASTER) {
    m_triggered = true;
    m_ipc_link = NULL;
    m_macsim->start();
  } else {  // if (m_operation_mode == SLAVE)
    m_triggered = false;
    m_ipc_link = configureLink("ipc_link", "1 ns");
    m_macsim->halt();
  }
}

macsimComponent::macsimComponent() : Component(-1) {
}  //for serialization only

void macsimComponent::configureLinks(SST::Params& params, TimeConverter* tc) {
  for (unsigned int l = 0; l < m_num_link; ++l) {
    ////////////////////////////////////////
    // Configure ICache Link
    std::string icache_portname = "core" + std::to_string(l) + "_icache";
    auto icache_link = loadUserSubComponent<Interfaces::StandardMem>(
      icache_portname, ComponentInfo::SHARE_NONE, tc,
      new Interfaces::StandardMem::Handler<macsimComponent>(
        this, &macsimComponent::handleInstructionCacheEvent));
    if (!icache_link) {
      Params interfaceParams;
      interfaceParams.insert("port", icache_portname);
      icache_link = loadAnonymousSubComponent<Interfaces::StandardMem>(
        "memHierarchy.standardInterface", icache_portname, 0,
        ComponentInfo::SHARE_PORTS | ComponentInfo::INSERT_STATS,
        interfaceParams, tc,
        new Interfaces::StandardMem::Handler<macsimComponent>(
          this, &macsimComponent::handleInstructionCacheEvent));
    }
    if(!icache_link) m_dbg->fatal(CALL_INFO, -1, "Configuring icache link failed\n");
    m_instruction_cache_links.push_back(icache_link);
    m_instruction_cache_requests.push_back(std::map<uint64_t, uint64_t>());
    m_instruction_cache_responses.push_back(std::set<uint64_t>());

    ////////////////////////////////////////
    // Configure DCache Link
    std::string dcache_portname = "core" + std::to_string(l) + "_dcache";

    auto dcache_link = loadUserSubComponent<Interfaces::StandardMem>(
      dcache_portname, ComponentInfo::SHARE_NONE, tc,
      new Interfaces::StandardMem::Handler<macsimComponent>(
        this, &macsimComponent::handleDataCacheEvent));
    if (!dcache_link) {
      Params interfaceParams;
      interfaceParams.insert("port", dcache_portname);
      dcache_link = loadAnonymousSubComponent<Interfaces::StandardMem>(
        "memHierarchy.standardInterface", dcache_portname, 0,
        ComponentInfo::SHARE_PORTS | ComponentInfo::INSERT_STATS,
        interfaceParams, tc,
        new Interfaces::StandardMem::Handler<macsimComponent>(
          this, &macsimComponent::handleDataCacheEvent));
    }
    if(!dcache_link) m_dbg->fatal(CALL_INFO, -1, "Configuring dcache link failed\n");
    m_data_cache_links.push_back(dcache_link);
    m_data_cache_requests.push_back(std::map<uint64_t, uint64_t>());
    m_data_cache_responses.push_back(std::set<uint64_t>());

    if (m_acc_core) {
      ////////////////////////////////////////
      // Configure Const Cache Link
      std::string ccache_portname = "core" + std::to_string(l) + "_ccache";
      auto ccache_link = loadUserSubComponent<Interfaces::StandardMem>(
        ccache_portname, ComponentInfo::SHARE_NONE, tc,
        new Interfaces::StandardMem::Handler<macsimComponent>(
          this, &macsimComponent::handleConstCacheEvent));
      if (!ccache_link) {
        Params interfaceParams;
        interfaceParams.insert("port", ccache_portname);
        ccache_link = loadAnonymousSubComponent<Interfaces::StandardMem>(
          "memHierarchy.standardInterface", ccache_portname,
          0, ComponentInfo::SHARE_PORTS | ComponentInfo::INSERT_STATS,
          interfaceParams, tc,
          new Interfaces::StandardMem::Handler<macsimComponent>(
            this, &macsimComponent::handleConstCacheEvent));
      }
      if(!ccache_link) m_dbg->fatal(CALL_INFO, -1, "Configuring ccache link failed\n");
      m_const_cache_links.push_back(ccache_link);
      m_const_cache_requests.push_back(std::map<uint64_t, uint64_t>());
      m_const_cache_responses.push_back(std::set<uint64_t>());

      ////////////////////////////////////////
      // Configure Texture Cache Link
      std::string tcache_portname = "core" + std::to_string(l) + "_tcache";
      auto tcache_link = loadUserSubComponent<Interfaces::StandardMem>(
        tcache_portname, ComponentInfo::SHARE_NONE, tc,
        new Interfaces::StandardMem::Handler<macsimComponent>(
          this, &macsimComponent::handleTextureCacheEvent));
      if (!tcache_link) {
        Params interfaceParams;
        interfaceParams.insert("port", tcache_portname);
        tcache_link = loadAnonymousSubComponent<Interfaces::StandardMem>(
          "memHierarchy.standardInterface", tcache_portname,
          0, ComponentInfo::SHARE_PORTS | ComponentInfo::INSERT_STATS,
          interfaceParams, tc,
          new Interfaces::StandardMem::Handler<macsimComponent>(
            this, &macsimComponent::handleTextureCacheEvent));
      }
      if(!tcache_link) m_dbg->fatal(CALL_INFO, -1, "Configuring tcache link failed\n");
      m_texture_cache_links.push_back(tcache_link);
      m_texture_cache_requests.push_back(std::map<uint64_t, uint64_t>());
      m_texture_cache_responses.push_back(std::set<uint64_t>());
    }
  }

  m_instruction_cache_request_counters = std::vector<uint64_t>(m_num_link, 0);
  m_instruction_cache_response_counters = std::vector<uint64_t>(m_num_link, 0);
  m_data_cache_request_counters = std::vector<uint64_t>(m_num_link, 0);
  m_data_cache_response_counters = std::vector<uint64_t>(m_num_link, 0);

  if (m_acc_core) {
    m_const_cache_request_counters = std::vector<uint64_t>(m_num_link, 0);
    m_const_cache_response_counters = std::vector<uint64_t>(m_num_link, 0);
    m_texture_cache_request_counters = std::vector<uint64_t>(m_num_link, 0);
    m_texture_cache_response_counters = std::vector<uint64_t>(m_num_link, 0);
  }
}

void macsimComponent::init(unsigned int phase) {
  MSC_DEBUG("Participating in phase %d of init.\n", phase);

  // if (!phase) {
  for (unsigned int l = 0; l < m_num_link; ++l) {
    m_instruction_cache_links[l]->init(phase);
    m_data_cache_links[l]->init(phase);
    if (m_acc_core) {
      m_const_cache_links[l]->init(phase);
      m_texture_cache_links[l]->init(phase);
    }
  }

  if (m_cube_connected) m_cube_link->init(phase);
  // }
}

void macsimComponent::setup() {
  MSC_DEBUG("------- Setting up -------\n");

  // Tokenize command line
  vector<string> tokens;
  auto cl = strdup(m_command_line.c_str());
  auto token = strtok(cl, " ");
  while (token != nullptr) {
    tokens.push_back(token);
    token = strtok(nullptr, " ");
  }
  free(cl);

  // Build arguments
  char** argv = new char*[1 + 3 + tokens.size()];

  int argc = 0;
  argv[argc] = new char[4];
  strcpy(argv[argc], "sst");
  argc++;
  argv[argc] = new char[m_param_file.size() + 1];
  strcpy(argv[argc], m_param_file.c_str());
  argc++;
  argv[argc] = new char[m_trace_file.size() + 1];
  strcpy(argv[argc], m_trace_file.c_str());
  argc++;
  argv[argc] = new char[m_output_dir.size() + 1];
  strcpy(argv[argc], m_output_dir.c_str());
  argc++;

  for_each(tokens.begin(), tokens.end(), [&argc, &argv](string const& token) {
    argv[argc] = new char[token.size() + 1];
    strcpy(argv[argc], token.c_str());
    argc++;
  });

  // Pass paramaters to simulator if applicable
  m_macsim->initialize(argc, argv);

  // Cleanup
  for (int trav = 0; trav < argc; ++trav) delete argv[trav];
  delete argv;

  CallbackSendInstructionCacheRequest* sir =
    new Callback<macsimComponent, void, int, uint64_t, uint64_t, int>(
      this, &macsimComponent::sendInstructionCacheRequest);
  CallbackSendDataCacheRequest* sdr =
#ifdef USE_VAULTSIM_HMC
    new Callback<macsimComponent, void, int, uint64_t, uint64_t, int, int,
                 uint32_t, uint64_t>(this,
                                     &macsimComponent::sendDataCacheRequest);
#else
    new Callback<macsimComponent, void, int, uint64_t, uint64_t, int, int>(
      this, &macsimComponent::sendDataCacheRequest);
#endif
  CallbackStrobeInstructionCacheRespQ* sirq =
    new Callback<macsimComponent, bool, int, uint64_t>(
      this, &macsimComponent::strobeInstructionCacheRespQ);
  CallbackStrobeDataCacheRespQ* sdrq =
    new Callback<macsimComponent, bool, int, uint64_t>(
      this, &macsimComponent::strobeDataCacheRespQ);

  if (m_acc_core) {
    CallbackSendConstCacheRequest* scr =
      new Callback<macsimComponent, void, int, uint64_t, uint64_t, int>(
        this, &macsimComponent::sendConstCacheRequest);
    CallbackSendTextureCacheRequest* str =
      new Callback<macsimComponent, void, int, uint64_t, uint64_t, int>(
        this, &macsimComponent::sendTextureCacheRequest);
    CallbackStrobeConstCacheRespQ* scrq =
      new Callback<macsimComponent, bool, int, uint64_t>(
        this, &macsimComponent::strobeConstCacheRespQ);
    CallbackStrobeTextureCacheRespQ* strq =
      new Callback<macsimComponent, bool, int, uint64_t>(
        this, &macsimComponent::strobeTextureCacheRespQ);

    m_macsim->registerCallback(sir, sdr, scr, str, sirq, sdrq, scrq, strq);
  } else {
    m_macsim->registerCallback(sir, sdr, sirq, sdrq);
  }

  if (m_cube_connected) {
    CallbackSendCubeRequest* scr =
      new Callback<macsimComponent, void, uint64_t, uint64_t, int, int>(
        this, &macsimComponent::sendCubeRequest);
    CallbackStrobeCubeRespQ* scrq =
      new Callback<macsimComponent, bool, uint64_t>(
        this, &macsimComponent::strobeCubeRespQ);

    m_macsim->registerCallback(scr, scrq);
  }

  // Setup memory links
  for (unsigned int l = 0; l < m_num_link; ++l) {   // Standard CPU does this but others don't, why??
    m_instruction_cache_links[l]->setup();
    m_data_cache_links[l]->setup();
    if(m_acc_core){
      m_const_cache_links[l]->setup();
      m_texture_cache_links[l]->setup();
    }
  }
  if (m_cube_connected) m_cube_link->setup();

}

void macsimComponent::complete(unsigned int phase) {
    // output.verbose(CALL_INFO, 1, 0, "Component is participating in phase %d of complete.\n", phase);
    MSC_DEBUG("Participating in phase %d of complete.\n", phase);
}

void macsimComponent::finish() {
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
bool macsimComponent::ticReceived(Cycle_t) {
  ++m_cycle;

  if (!m_triggered) {  // When SLAVE mode, wait until triggering event arrives.
    SST::Event* e = NULL;
    if ((e = m_ipc_link->recv())) {
      MacSimEvent* event = dynamic_cast<MacSimEvent*>(e);
      if (event->getType() == NONE) {
        m_dbg->fatal(CALL_INFO, -1,
                     "macsimComponent got bad event from another component\n");
      }
      MSC_DEBUG("Received an event (%p) of type: %d at cycle %lu\n", event,
                event->getType(), m_cycle);
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
    for (unsigned int l = 0; l < m_num_link; ++l) {
      if (m_acc_core) {
        MSC_DEBUG(
          "Core[%2d] I$: (%lu, %lu), D$: (%lu, %lu) C$: (%lu, %lu), T$: (%lu, "
          "%lu)\n",
          l, m_instruction_cache_request_counters[l],
          m_instruction_cache_response_counters[l],
          m_data_cache_request_counters[l], m_data_cache_response_counters[l],
          m_const_cache_request_counters[l], m_const_cache_response_counters[l],
          m_texture_cache_request_counters[l],
          m_texture_cache_response_counters[l]);
      } else {
        MSC_DEBUG("Core[%2d] I$: (%lu, %lu), D$: (%lu, %lu)\n", l,
                  m_instruction_cache_request_counters[l],
                  m_instruction_cache_response_counters[l],
                  m_data_cache_request_counters[l],
                  m_data_cache_response_counters[l]);
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
      m_ipc_link->send(new MacSimEvent(FINISHED));
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
void macsimComponent::sendInstructionCacheRequest(int core_id, uint64_t key,
                                                  uint64_t addr, int size) {
  // mask address to make sure it lies in address range
  uint64_t req_addr = addr & (m_mem_size - 1);

#ifndef USE_VAULTSIM_HMC
  // StandardMem::Request* req = new StandardMem::Request(
  //   StandardMem::Request::Read, addr & (m_mem_size - 1), size);
  StandardMem::Request* req = new StandardMem::Read(req_addr, size);
#else
  StandardMem::Request* req = new StandardMem::Request(
    StandardMem::Request::Read, req_addr, size, 0, HMC_NONE);
#endif

  // Print out the request
  if (m_debug_all || m_debug_addr == addr) {
    MSC_DEBUG("I$[%d] request sent: addr = %#" PRIx64 " (orig addr = %#" PRIx64
              "), size = %d\n", core_id, req_addr, addr, size);
  }
  m_instruction_cache_links[core_id]->send(req);
  m_instruction_cache_request_counters[core_id]++;
  m_instruction_cache_requests[core_id].insert(make_pair(req->getID(), key));
}

bool macsimComponent::strobeInstructionCacheRespQ(int core_id, uint64_t key) {
  auto I = m_instruction_cache_responses[core_id].find(key);
  if (I == m_instruction_cache_responses[core_id].end())
    return false;
  else {
    m_instruction_cache_responses[core_id].erase(I);
    return true;
  }
}

// incoming events are scanned and deleted
void macsimComponent::handleInstructionCacheEvent(
  Interfaces::StandardMem::Request* req) {
  for (unsigned int l = 0; l < m_num_link; ++l) {
    auto i = m_instruction_cache_requests[l].find(req->getID());
    if (m_instruction_cache_requests[l].end() == i) {
      // No matching request
      continue;
    } else {
        // Get request
        auto m_instruction_req = m_instruction_cache_requests[l].find(req->getID());    // FIXME: 

        if (m_debug_all/* || m_debug_addr == req->pAddr*/) {    // FIXME: debugging a particular address requires sophisticated handling. see juno example
          MSC_DEBUG("I$[%d] response arrived: (%s)\n", l, req->getString().c_str());
          // MSC_DEBUG("I$[%d] response arrived: addr = %#" PRIx64 ", size = %lu (%s)\n",
          //   l, req->pAddr, req->size, req->getString().c_str());
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
inline bool isStore(Mem_Type type) {
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
void macsimComponent::sendDataCacheRequest(int core_id, uint64_t key,
                                           uint64_t addr, int size, int type) {
  // FIXME:
  // mask address to make sure it lies in address range
  uint64_t req_addr = addr & (m_mem_size - 1);

  bool doWrite = isStore((Mem_Type)type);

  StandardMem::Request* req;
  if(doWrite) {
    std::vector<uint8_t> data;
    data.push_back((req_addr >> 24) & 0xff);
    data.push_back((req_addr >> 16) & 0xff);
    data.push_back((req_addr >>  8) & 0xff);
    data.push_back((req_addr >>  0) & 0xff);
    req = new StandardMem::Write(req_addr, size, data);
  }
  else {
    req = new StandardMem::Read(req_addr, size);
  }

  // StandardMem::Request* req = new StandardMem::Request(
  //   doWrite ? StandardMem::Request::Write : StandardMem::Request::Read,
  //   addr & (m_mem_size - 1), size);
  if (m_debug_all || m_debug_addr == addr) {
    MSC_DEBUG("D$[%d] request sent: addr = %#" PRIx64 " (orig addr = %#" PRIx64
                "), %s, size = %d\n",
                core_id, req_addr, addr, doWrite ? "write" : "read",
                size);
  }
  
  m_data_cache_links[core_id]->send(req);
  m_data_cache_request_counters[core_id]++;
  m_data_cache_requests[core_id].insert(make_pair(req->getID(), key));
}
#else
void macsimComponent::sendDataCacheRequest(int core_id, uint64_t key,
                                           uint64_t addr, int size, int type,
                                           uint32_t hmc_type = 0,
                                           uint64_t hmc_trans = 0) {
  bool doWrite = isStore((Mem_Type)type);
  unsigned flag = 0;
  if ((hmc_type & 0x0080) != 0) {
    flag = StandardMem::Request::F_NONCACHEABLE;
    hmc_type = hmc_type & 0b01111111;
  }
  StandardMem::Request* req = new StandardMem::Request(
    doWrite ? StandardMem::Request::Write : StandardMem::Request::Read,
    addr & (m_mem_size - 1), size, flag, hmc_type);
  m_data_cache_links[core_id]->sendRequest(req);
  m_data_cache_request_counters[core_id]++;
  m_data_cache_requests[core_id].insert(make_pair(req->id, key));
  if (m_debug_all || m_debug_addr == addr) {
    MSC_DEBUG("D$[%d] request sent: addr = %#" PRIx64 " (orig addr = %#" PRIx64
              "), %s, size = %d\n",
              core_id, addr & 0x3FFFFFFF, addr, doWrite ? "write" : "read",
              size);
  }
}
#endif

bool macsimComponent::strobeDataCacheRespQ(int core_id, uint64_t key) {
  auto I = m_data_cache_responses[core_id].find(key);
  if (I == m_data_cache_responses[core_id].end())
    return false;
  else {
    m_data_cache_responses[core_id].erase(I);
    return true;
  }
}

// incoming events are scanned and deleted
void macsimComponent::handleDataCacheEvent(
  Interfaces::StandardMem::Request* req) {
  for (unsigned int l = 0; l < m_num_link; ++l) {
    auto i = m_data_cache_requests[l].find(req->getID());
    if (m_data_cache_requests[l].end() == i) {
      // No matching request
      continue;
    } else {
        // FIXME:
      if (m_debug_all /*|| m_debug_addr == req->pAddr*/) {
        MSC_DEBUG("D$[%d] response arrived: (%s)\n",
            l, req->getString().c_str());
        // MSC_DEBUG("D$[%d] response arrived: addr = %#" PRIx64 ", size = %lu (%s)\n",
        //   l, req->pAddr, req->size, req->getString().c_str());
      }
      m_data_cache_responses[l].insert(i->second);
      m_data_cache_response_counters[l]++;
      m_data_cache_requests[l].erase(i);
      break;
    }
  }
  delete req;
}

////////////////////////////////////////
//
// Const-Cache related routines go here
//
////////////////////////////////////////
void macsimComponent::sendConstCacheRequest(int core_id, uint64_t key,
                                            uint64_t addr, int size) {
// FIXME:
//   StandardMem::Request* req = new StandardMem::Request(
//     StandardMem::Request::Read, addr & (m_mem_size - 1), size);
//   m_const_cache_links[core_id]->sendRequest(req);
//   m_const_cache_request_counters[core_id]++;
//   m_const_cache_requests[core_id].insert(make_pair(req->id, key));

//   if (m_debug_all || m_debug_addr == addr) {
//     MSC_DEBUG("C$[%d] request sent: addr = %#" PRIx64 " (orig addr = %#" PRIx64
//               ", size = %d\n",
//               core_id, addr & (m_mem_size - 1), addr, size);
//   }
}

bool macsimComponent::strobeConstCacheRespQ(int core_id, uint64_t key) {
  auto I = m_const_cache_responses[core_id].find(key);
  if (I ==
      m_const_cache_responses[core_id].end())  // Response has not yet arrived
    return false;
  else {  // Response has arrived
    m_const_cache_responses[core_id].erase(I);
    return true;
  }
}

// incoming events are scanned and deleted
void macsimComponent::handleConstCacheEvent(Interfaces::StandardMem::Request* req) {
// FIXME:
//   for (unsigned int l = 0; l < m_num_link; ++l) {
//     auto i = m_const_cache_requests[l].find(req->id);
//     if (m_const_cache_requests[l].end() == i) {  // No matching request
//       continue;
//     } else {
//       if (m_debug_all || m_debug_addr == req->getAddr()) {
//         MSC_DEBUG("C$[%d] response arrived: addr = %#" PRIx64 ", size = %lu\n",
//                   l, req->getAddr(), req->size);
//       }
//       m_const_cache_responses[l].insert(i->second);
//       m_const_cache_response_counters[l]++;
//       m_const_cache_requests[l].erase(i);
//       break;
//     }
//   }

  delete req;
}

////////////////////////////////////////
//
// Texture-Cache related routines go here
//
////////////////////////////////////////
void macsimComponent::sendTextureCacheRequest(int core_id, uint64_t key,
                                              uint64_t addr, int size) {
// FIXME:
//   StandardMem::Request* req = new StandardMem::Request(
//     StandardMem::Request::Read, addr & (m_mem_size - 1), size);
//   m_texture_cache_links[core_id]->sendRequest(req);
//   m_texture_cache_request_counters[core_id]++;
//   m_texture_cache_requests[core_id].insert(make_pair(req->id, key));
//   MSC_DEBUG("T$[%d] request sent: addr = %#" PRIx64 " (orig addr = %#" PRIx64
//             ", size = %d\n",
//             core_id, addr & (m_mem_size - 1), addr, size);
}

bool macsimComponent::strobeTextureCacheRespQ(int core_id, uint64_t key) {
  auto I = m_texture_cache_responses[core_id].find(key);
  if (I ==
      m_texture_cache_responses[core_id].end())  // Response has not yet arrived
    return false;
  else {  // Response has arrived
    m_texture_cache_responses[core_id].erase(I);
    return true;
  }
}

// incoming events are scanned and deleted
void macsimComponent::handleTextureCacheEvent(
  Interfaces::StandardMem::Request* req) {
// FIXME:
//   for (unsigned int l = 0; l < m_num_link; ++l) {
//     auto i = m_texture_cache_requests[l].find(req->id);
//     if (m_texture_cache_requests[l].end() == i) {  // No matching request
//       continue;
//     } else {
//       if (m_debug_all || m_debug_addr == req->getAddr()) {
//         MSC_DEBUG("T$[%d] response arrived: addr = %#" PRIx64 ", size = %lu\n",
//                   l, req->getAddr(), req->size);
//       }
//       m_texture_cache_responses[l].insert(i->second);
//       m_texture_cache_response_counters[l]++;
//       m_texture_cache_requests[l].erase(i);
//       break;
//     }
//   }

//   delete req;
}

////////////////////////////////////////
//
// VaultSim related routines go here
//
////////////////////////////////////////
void macsimComponent::sendCubeRequest(uint64_t key, uint64_t addr, int size,
                                      int type) {
// FIXME:
//   bool doWrite = isStore((Mem_Type)type);
//   StandardMem::Request* req = new StandardMem::Request(
//     doWrite ? StandardMem::Request::Write : StandardMem::Request::Read,
//     addr & 0x3FFFFFFF, size);
//   m_cube_link->sendRequest(req);
//   if (m_debug_all || m_debug_addr == addr) {
//     MSC_DEBUG("Cube request sent: addr = %#" PRIx64 "(orig addr = %#" PRIx64
//               "), %s %s, size = %d\n",
//               addr & 0x3FFFFFFF, addr, (type == -1) ? "instruction" : "data",
//               doWrite ? "write" : "read", size);
//   }
//   m_cube_requests.insert(make_pair(req->getID(), key));
}

bool macsimComponent::strobeCubeRespQ(uint64_t key) {
  auto I = m_cube_responses.find(key);
  if (I == m_cube_responses.end())
    return false;
  else {
    m_cube_responses.erase(I);
    return true;
  }
}

// incoming events are scanned and deleted
void macsimComponent::handleCubeEvent(Interfaces::StandardMem::Request* req) {
// FIXME:
//   auto i = m_cube_requests.find(req->getID());
//   if (m_cube_requests.end() == i) {
//     // No matching request
//     m_dbg->fatal(CALL_INFO, -1, "Event (%#" PRIx64 ") not found!\n", req->getID());
//   } else {
//     if (m_debug_all || m_debug_addr == req->getAddr()) {
//       MSC_DEBUG("Cube response arrived: addr = %#" PRIx64 ", size = %lu\n",
//                 req->getAddr(), req->size);
//     }
//     m_cube_responses.insert(i->second);
//     m_cube_requests.erase(i);
//   }

//   delete req;
}
