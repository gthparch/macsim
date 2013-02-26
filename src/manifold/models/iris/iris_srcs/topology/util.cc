
#include	"../../interfaces/topology.h"
#include	"../components/simpleRouter.h"
#include	"twoNode.h"
#include	"ring.h"

Topology* 
get_topology(topology_type ch, macsim_c* simBase)
{
    if ( ch == TWO_NODE)
        return new TwoNode();
    if ( ch == RING)
        return new Ring(simBase);

    return NULL;
}

void
dump_state_at_deadlock( macsim_c* simBase )
{
    fprintf(stderr, "\n***** DEADLOCK ****\n");
    cout << " PIPESTAGES: " << "0=INVALID, 1=EMPTY, 2=IB, 3=FULL, 4=ROUTED, 5=VCA_REQUESTED, 6=SWA_REQUESTED, 7=SW_ALLOCATED, 8=SW_TRAVERSAL, 9=REQ_OUTVC_ARB, 10=VCA_COMPLETE " << endl;
    cout << "number of nodes in network: " << simBase->no_nodes << endl;
    
    for ( uint i=0; i<simBase->no_nodes; i++)
    {
        SimpleRouter* rr = static_cast<SimpleRouter*>(simBase->m_iris_network->routers.at(i));
        fprintf( stderr, "\n\n------ Router %d", i);
        for ( uint ii=0; ii<rr->input_buffer_state.size(); ii++)
        {
            InputBufferState ib = rr->input_buffer_state.at(ii);
            fprintf(stderr,"\n Msg%d ip%d ic%d op%d oc%d ps%d",ii,ib.input_port, ib.input_channel, ib.output_port, ib.output_channel, ib.pipe_stage); 

        }
    }
    fprintf(stderr,"\n\n");
    _sim_exit_now(" **** Deadlock Proc %d ", simBase->Mytid);
}
