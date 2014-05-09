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

using namespace boost;
using namespace SST;
using namespace SST::MacSim;

macsimComponent::macsimComponent(ComponentId_t id, Params& params) : Component(id)
{
  dbg = new Output();

  int debugLevel = params.find_integer("debugLevel", 0);
  if (debugLevel < 0 || debugLevel > 8) _abort(macsimComponent, "Debugging level must be betwee 0 and 8. \n");

  string prefix = "[" + getName() + "] ";
  dbg->init(prefix, debugLevel, 0, (Output::output_location_t)params.find_integer("printLocation", Output::NONE));
  MSC_DEBUG("------- Initializing -------\n");

  if (params.find("paramFile") == params.end()) _abort(macsimComponent, "Couldn't find params.in file\n");
  if (params.find("traceFile") == params.end()) _abort(macsimComponent, "Couldn't find trace_file_list file\n");
  if (params.find("outputDir") == params.end()) _abort(macsimComponent, "Couldn't find statistics output directory parameter");

  paramFile = string(params["paramFile"]);
  traceFile = string(params["traceFile"]);
  outputDir = string(params["outputDir"]);

  if (params.find("commandLine") != params.end()) 
    commandLine = string(params["commandLine"]);

  icache_link = dynamic_cast<Interfaces::SimpleMem*>(loadModuleWithComponent("memHierarchy.memInterface", this, params));
  if (!icache_link) _abort(macsimComponent, "Unable to load Module as memory\n");
  icache_link->initialize("icache_link", new Interfaces::SimpleMem::Handler<macsimComponent>(this, &macsimComponent::handleIcacheEvent));

  dcache_link = dynamic_cast<Interfaces::SimpleMem*>(loadModuleWithComponent("memHierarchy.memInterface", this, params));
  if (!dcache_link) _abort(macsimComponent, "Unable to load Module as memory\n");
  dcache_link->initialize("dcache_link", new Interfaces::SimpleMem::Handler<macsimComponent>(this, &macsimComponent::handleDcacheEvent));

  cubeConnected = params.find_integer("cubeConnected", 0);
  if (cubeConnected) {
    cube_link = dynamic_cast<Interfaces::SimpleMem*>(loadModuleWithComponent("memHierarchy.memInterface", this, params));
    if (!cube_link) _abort(macsimComponent, "Unable to load Module as memory\n");
    cube_link->initialize("cube_link", new Interfaces::SimpleMem::Handler<macsimComponent>(this, &macsimComponent::handleCubeEvent));
  } else {
    cube_link = NULL;
  }

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
    if (cubeConnected)
      cube_link->sendInitData(new Interfaces::StringEvent("SST::MemHierarchy::MemEvent"));
  }
}

#include <boost/tokenizer.hpp>
int countTokens(string _commandLine)
{
  int count = 0;
  char_separator<char> sep(" ");
  tokenizer<char_separator<char>> tokens(_commandLine, sep);
  for (auto I = tokens.begin(), E = tokens.end(); I != E; I++) count++;
  return count;
}

void macsimComponent::setup() 
{
  MSC_DEBUG("------- Setting up -------\n");

  // Build arguments
  char** argv = new char*[1+3+countTokens(commandLine)];

  int argc = 0;
  argv[argc] = new char[                  4 ]; strcpy(argv[argc],             "sst"); argc++;
  argv[argc] = new char[ paramFile.size()+1 ]; strcpy(argv[argc], paramFile.c_str()); argc++;
  argv[argc] = new char[ traceFile.size()+1 ]; strcpy(argv[argc], traceFile.c_str()); argc++;
  argv[argc] = new char[ outputDir.size()+1 ]; strcpy(argv[argc], outputDir.c_str()); argc++;

  char_separator<char> sep(" ");
  tokenizer<char_separator<char>> tokens(commandLine, sep);
  for (auto I = tokens.begin(), E = tokens.end(); I != E; I++) {
    string command = *I;
    argv[argc] = new char[ command.size()+1 ]; strcpy(argv[argc], command.c_str()); argc++;
  }

  // Pass paramaters to simulator if applicable
  macsim->initialize(argc, argv);

  // Cleanup
  for (int trav = 0; trav < argc; trav++) delete argv[trav];
  delete argv;

  CallbackSendInstReq* sir = 
    new Callback3<macsimComponent, void, uint64_t, uint64_t, int>(this, &macsimComponent::sendInstReq);
  CallbackSendDataReq* sdr =
    new Callback4<macsimComponent, void, uint64_t, uint64_t, int, int>(this, &macsimComponent::sendDataReq);
  CallbackStrobeInstRespQ* sirq = 
    new Callback1<macsimComponent, bool, uint64_t>(this, &macsimComponent::strobeInstRespQ);
  CallbackStrobeDataRespQ* sdrq =
    new Callback1<macsimComponent, bool, uint64_t>(this, &macsimComponent::strobeDataRespQ);

  macsim->registerCallback(sir, sdr, sirq, sdrq);

  if (cubeConnected) {
    CallbackSendCubeReq* scr = 
      new Callback4<macsimComponent, void, uint64_t, uint64_t, int, int>(this, &macsimComponent::sendCubeReq);
    CallbackStrobeCubeRespQ* scrq =
      new Callback1<macsimComponent, bool, uint64_t>(this, &macsimComponent::strobeCubeRespQ);

    macsim->registerCallback(scr, scrq);
  }

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
void macsimComponent::sendInstReq(uint64_t key, uint64_t addr, int size)
{
  SimpleMem::Request *req = 
    new SimpleMem::Request(SimpleMem::Request::Read, addr & 0x3FFFFFFF, size);
  icache_link->sendRequest(req);
  MSC_DEBUG("I$ request sent: addr = %#" PRIx64 " (orig addr = %#" PRIx64 ", size = %d\n", 
      addr & 0x3FFFFFFF, addr, size);
  icache_requests.insert(make_pair(req->id, key));
}

bool macsimComponent::strobeInstRespQ(uint64_t key)
{
  auto I = icache_responses.find(key);
  if (I == icache_responses.end()) 
    return false;
  else {
    icache_responses.erase(I);
    return true;
  }
}

// incoming events are scanned and deleted
void macsimComponent::handleIcacheEvent(Interfaces::SimpleMem::Request *req)
{
  auto i = icache_requests.find(req->id);
  if (icache_requests.end() == i) {
    // No matching request
    _abort(macsimComponent, "Event (%#" PRIx64 ") not found!\n", req->id);
  } else {
    MSC_DEBUG("I$ response arrived\n");
    icache_responses.insert(i->second);
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

void macsimComponent::sendDataReq(uint64_t key, uint64_t addr, int size, int type)
{
  bool doWrite  = isStore((Mem_Type)type);
  SimpleMem::Request *req = 
    new SimpleMem::Request(doWrite ? SimpleMem::Request::Write : SimpleMem::Request::Read, addr & 0x3FFFFFFF, size);
  dcache_link->sendRequest(req);
  MSC_DEBUG("D$ request sent: addr = %#" PRIx64 " (orig addr = %#" PRIx64 "), %s, size = %d\n", 
      addr & 0x3FFFFFFF, addr, doWrite ? "write" : "read", size);
  dcache_requests.insert(make_pair(req->id, key));
}

bool macsimComponent::strobeDataRespQ(uint64_t key)
{
  auto I = dcache_responses.find(key);
  if (I == dcache_responses.end()) 
    return false;
  else {
    dcache_responses.erase(I);
    return true;
  }
}

// incoming events are scanned and deleted
void macsimComponent::handleDcacheEvent(Interfaces::SimpleMem::Request *req)
{
  auto i = dcache_requests.find(req->id);
  if (dcache_requests.end() == i) {
    // No matching request
  } else {
    MSC_DEBUG("D$ response arrived: addr = %#" PRIx64 ", size = %#" PRIx64 "\n", req->addr, req->size);
    dcache_responses.insert(i->second);
    dcache_requests.erase(i);
  }

  delete req;
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
  cube_link->sendRequest(req);
  MSC_DEBUG("Cube request sent: addr = %#" PRIx64 "(orig addr = %#" PRIx64 "), %s %s, size = %d\n", 
      addr & 0x3FFFFFFF, addr, (type == -1) ? "instruction" : "data",  doWrite ? "write" : "read", size);
  cube_requests.insert(make_pair(req->id, key));
}

bool macsimComponent::strobeCubeRespQ(uint64_t key)
{
  auto I = cube_responses.find(key);
  if (I == cube_responses.end()) 
    return false;
  else {
    cube_responses.erase(I);
    return true;
  }
}

// incoming events are scanned and deleted
void macsimComponent::handleCubeEvent(Interfaces::SimpleMem::Request *req)
{
  auto i = cube_requests.find(req->id);
  if (cube_requests.end() == i) {
    // No matching request
    _abort(macsimComponent, "Event (%#" PRIx64 ") not found!\n", req->id);
  } else {
    MSC_DEBUG("Cube response arrived\n");
    cube_responses.insert(i->second);
    cube_requests.erase(i);
  }

  delete req;
}
