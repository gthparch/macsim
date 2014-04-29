#include <sys/time.h>

#include <sst_config.h>
#include <sst/core/serialization/element.h>
#include <sst/core/element.h>
#include <sst/core/simulation.h>
#include <sst/core/params.h>
#include <sst/core/debug.h>

#include <sst/core/interfaces/stringEvent.h>
#include <sst/core/interfaces/simpleMem.h>

#include "src/global_defs.h"
#include "src/uop.h"
#include "src/frontend.h"

#include "macsimComponent.h"

#define MSC_DEBUG(fmt, args...) dbg->debug(CALL_INFO,INFO,0,fmt,##args)

using namespace SST;
using namespace SST::MacSim;

macsimComponent::macsimComponent(ComponentId_t id, Params& params) : Component(id)
{
  dbg = new Output();

  int debugLevel = params.find_integer("debugLevel", 0);
  if (debugLevel < 0 || debugLevel > 8) _abort(macsimComponent, "Debugging level must be betwee 0 and 8. \n");

  std::string prefix = "[" + getName() + "] ";
  dbg->init(prefix, debugLevel, 0, (Output::output_location_t)params.find_integer("printLocation", Output::NONE));
  MSC_DEBUG("------- Initializing -------\n");

  if (params.find("paramFile") == params.end()) _abort(macsimComponent, "Couldn't find params.in file\n");
  if (params.find("traceFile") == params.end()) _abort(macsimComponent, "Couldn't find trace_file_list file\n");
  if (params.find("outputDir") == params.end()) _abort(macsimComponent, "Couldn't find statistics output directory parameter");

  paramFile = std::string(params["paramFile"]);
  traceFile = std::string(params["traceFile"]);
  outputDir = std::string(params["outputDir"]);

  icache_link = dynamic_cast<Interfaces::SimpleMem*>(loadModuleWithComponent("memHierarchy.memInterface", this, params));
  if (!icache_link) _abort(macsimComponent, "Unable to load Module as memory\n");
  icache_link->initialize("icache_link", new Interfaces::SimpleMem::Handler<macsimComponent>(this, &macsimComponent::icacheHandleEvent));

  dcache_link = dynamic_cast<Interfaces::SimpleMem*>(loadModuleWithComponent("memHierarchy.memInterface", this, params));
  if (!dcache_link) _abort(macsimComponent, "Unable to load Module as memory\n");
  dcache_link->initialize("dcache_link", new Interfaces::SimpleMem::Handler<macsimComponent>(this, &macsimComponent::dcacheHandleEvent));

  string clockFreq = params.find_string("clockFreq", "1 GHz");  //Hertz
  registerClock(clockFreq, new Clock::Handler<macsimComponent>(this, &macsimComponent::ticReceived));

  registerAsPrimaryComponent();
  primaryComponentDoNotEndSim();

  macsim = new macsim_c();
  simRunning = false;
}

macsimComponent::macsimComponent() : Component(-1) {}  //for serialization only 

void macsimComponent::init(unsigned int phase)
{
  if (!phase) {
    icache_link->sendInitData(new Interfaces::StringEvent("SST::MemHierarchy::MemEvent"));
    dcache_link->sendInitData(new Interfaces::StringEvent("SST::MemHierarchy::MemEvent"));
  }
}

void macsimComponent::setup() 
{
  MSC_DEBUG("------- Setting up -------\n");

  // Format for initialization protocol
  char* argv[3];
  argv[0] = new char[ paramFile.size()+1 ]; strcpy(argv[0], paramFile.c_str());
  argv[1] = new char[ traceFile.size()+1 ]; strcpy(argv[1], traceFile.c_str());
  argv[2] = new char[ outputDir.size()+1 ]; strcpy(argv[2], outputDir.c_str());

  // Pass paramaters to simulator if applicable
  macsim->initialize(0, argv);

  // Cleanup
  delete argv[0]; delete argv[1]; delete argv[2];

  CallbackSendInstReq* sir = 
    new Callback3<macsimComponent, void, frontend_s*, uint64_t, int>(this, &macsimComponent::sendInstReq);
  CallbackSendDataReq* sdr =
    new Callback1<macsimComponent, void, uop_c*>(this, &macsimComponent::sendDataReq);
  CallbackStrobeInstRespQ* sirq = 
    new Callback1<macsimComponent, bool, frontend_s*>(this, &macsimComponent::strobeInstRespQ);
  CallbackStrobeDataRespQ* sdrq =
    new Callback1<macsimComponent, bool, uop_c*>(this, &macsimComponent::strobeDataRespQ);
  
  macsim->registerCallback(sir, sdr, sirq, sdrq);

#ifdef HAVE_LIBDRAMSIM
  macsim->setMacsimComponent(this);
#endif //HAVE_LIBDRAMSIM
}

void macsimComponent::finish() 
{
  MSC_DEBUG("------- Finishing simulation -------\n");
  macsim->finalize();
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
  timestamp++;

	// Run a cycle of the simulator
	simRunning = macsim->run_a_cycle();

	// Still has more cycles to run
	if (simRunning) {
		return false;
	}
	// Let SST know that this component is done and could be terminated
	else {
    primaryComponentOKToEndSim();
		return true;
	}
}

////////////////////////////////////////
//
// I-Cache related routines go here
//
////////////////////////////////////////
void macsimComponent::sendInstReq(frontend_s* fetch_data, uint64_t fetch_addr, int fetch_size)
{
  uint64_t addr = fetch_addr & 0x3FFFFFFF;
  int      size = fetch_size;

  SimpleMem::Request *req = new SimpleMem::Request(SimpleMem::Request::Read, addr, size);
  icache_link->sendRequest(req);
  MSC_DEBUG("I$ request sent: addr = 0x%lx, size = %d\n", addr, size);

  icache_requests.insert(std::make_pair(req->id, fetch_data));
}

bool macsimComponent::strobeInstRespQ(frontend_s* fetch_data)
{
  auto I = icache_responses.find(fetch_data);
  if (I == icache_responses.end()) 
    return false;
  else {
    icache_responses.erase(I);
    return true;
  }
}

// incoming events are scanned and deleted
void macsimComponent::icacheHandleEvent(Interfaces::SimpleMem::Request *req)
{
  auto i = icache_requests.find(req->id);
  if (icache_requests.end() == i) {
    // No matching request
  } else {
    MSC_DEBUG("I$ response arrived\n");
    icache_responses.insert(std::make_pair(i->second, req->id));
    icache_requests.erase(i);
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

void macsimComponent::sendDataReq(uop_c* uop)
{
  uint64_t addr = uop->m_vaddr & 0x3FFFFFFF;
  int size      = uop->m_mem_size;
  Mem_Type type = uop->m_mem_type;
  bool doWrite  = isStore(type);

  SimpleMem::Request *req = new SimpleMem::Request(doWrite ? SimpleMem::Request::Write : SimpleMem::Request::Read, addr, size);
  dcache_link->sendRequest(req);
  MSC_DEBUG("D$ request sent: addr = 0x%lx (orig addr = 0x%lx), %s, size = %d\n", addr, uop->m_vaddr, doWrite ? "write" : "read", size);

  dcache_requests.insert(std::make_pair(req->id, uop));
}

bool macsimComponent::strobeDataRespQ(uop_c* uop)
{
  auto I = dcache_responses.find(uop);
  if (I == dcache_responses.end()) 
    return false;
  else {
    dcache_responses.erase(I);
    return true;
  }
}

// incoming events are scanned and deleted
void macsimComponent::dcacheHandleEvent(Interfaces::SimpleMem::Request *req)
{
  auto i = dcache_requests.find(req->id);
  if (dcache_requests.end() == i) {
    // No matching request
  } else {
    MSC_DEBUG("D$ response arrived: addr = 0x%llx (orig addr = 0x%llx), size = %d\n", i->second->m_vaddr & 0x3FFFFFFF, i->second->m_vaddr, i->second->m_mem_size);
    dcache_responses.insert(std::make_pair(i->second, req->id));
    dcache_requests.erase(i);
  }

  delete req;
}
