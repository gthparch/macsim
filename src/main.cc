#include <stdio.h>

#include "macsim.h"

int main(int argc, char** argv)
{
	macsim_c* sim;

	//Instantiate
	sim = new macsim_c();

	//Initialize Simulation State
	sim->initialize(argc, argv);

	//Run simulation
	//report("run core (single threads)");
	while (sim->run_a_cycle());
	
	//Finialize Simulation State
	sim->finalize();

	return 0;
}
