#include "sst_config.h"

#include "sst/core/serialization/element.h"
#include "sst/core/element.h"

#include "macsimComponent.h"

#include <sys/time.h>
using namespace SST;

macsimComponent::macsimComponent(ComponentId_t id, Params_t& params) : Component(id) {
	
	//GET PARAMETERS

	//params.in
	if ( params.find("paramPath") == params.end() ) {
		_abort(event_test, "Couldn't find params.in path\n");
	}
	paramPath = std::string(params["paramPath"]);

	//trace_file_list
	if ( params.find("tracePath") == params.end() ) {
		_abort(event_test, "Couldn't find trace_file_list path\n");
	}
	tracePath = std::string(params["tracePath"]);


	//statistics out directory
	if ( params.find("outputPath") == params.end() ) {
		_abort(event_test, "Couldn't find statistics output directory parameter");
	}
	outputPath = std::string(params["outputPath"]);

	//END GET PARAMETERS
	
	
	//Register with clock
	registerClock("1Ghz",
	              new Clock::Handler<macsimComponent>(this, 
                                                    &macsimComponent::ticReceived));
	
	//Instantiate Macsim Simulator
	macsim = new macsim_c();
	simRunning = false;

}

macsimComponent::macsimComponent() : Component(-1) {} //for serialization only

void macsimComponent::setup() {

	//Ensure that this component must finish before SST kernel can terminate
	//registerExit();
  registerAsPrimarComponent();
  primaryComponentDoNotEndSim();

	printf("Initializing macsim simulation state\n");
	
	//Format for initialization protocol
	char* argv[3];
	argv[0] = new char[ paramPath.size()+1]; strcpy(argv[0],  paramPath.c_str());
	argv[1] = new char[ tracePath.size()+1]; strcpy(argv[1],  tracePath.c_str());
	argv[2] = new char[outputPath.size()+1]; strcpy(argv[2], outputPath.c_str());
		
	//Pass paramaters to simulator if applicable
	macsim->initialize(0, argv);

	//cleanup
	delete argv[0]; delete argv[1]; delete argv[2];

	//return 0;
}

void macsimComponent::finish() {
	printf("Simulation Finished, Finalizing");

  macsim->finalize();

	//return 0;
}

/*******************************************************
 *  ticReceived
 *    return value
 *      true  : indicates the component finished; 
 *              no more clock events to the component
 *      false : component not done yet
 *******************************************************/
bool macsimComponent::ticReceived( Cycle_t ) {

	//Run a cycle of the simulator
	simRunning = macsim->run_a_cycle();

	//Still has more cycles to run
	if (simRunning) {
		return false;
	}

	//Let SST know that this component is done and could be terminated
	else {
		unregisterExit();
		return true;
	}

}

// Element Libarary / Serialization stuff

BOOST_CLASS_EXPORT(macsimComponent)

static Component* create_macsimComponent(SST::ComponentId_t id,
                                         SST::Component::Params_t& params)
{
	return new macsimComponent( id, params );
}

static const ElementInfoComponent components[] = {
	{ 
		"macsimComponent",
		"MACSIM Simulator",
		NULL,
		create_macsimComponent
	},
	{ NULL, NULL, NULL, NULL }
};

extern "C" {
	ElementLibraryInfo macsimComponent_eli = 
	{
		"macsimComponent",
		"MACSIM Simulator",
		components,
	};
}
