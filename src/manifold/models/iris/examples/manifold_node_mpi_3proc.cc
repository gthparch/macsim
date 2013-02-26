/*
 * =====================================================================================
 *
 *       Filename:  manifold_node.cc
 *
 *    Description:  This is a single node using the manifold kernel
 *			Proc(PktGen)->NI->Router->NI->MC
 *			Runs in a single LP
 *
 *        Version:  1.0
 *        Created:  04/20/2010 02:59:08 AM
 *       Revision:  none
 *       Compiler:  mpicxx
 *
 *         Author:  Mitchelle Rasquinha (), mitchelle.rasquinha@gatech.edu
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  _simmc2mesh_cc_INC
#define  _simmc2mesh_cc_INC

#include        "genericHeader.h"
#include        "irisTerminal.h"
#include        "irisInterface.h"
#include        "irisRouter.h"
#include        "topology.h"

#include        "kernel/clock.h"
#include        "kernel/manifold.h"
#include        "kernel/component.h"
#include        "kernel/messenger.h"

#include        "../iris_srcs/components/pktgen.h"
#include        "../iris_srcs/components/simpleRouter.h"
#include        "../iris_srcs/components/simpleMC.h"
#include        "../iris_srcs/components/ninterface.h"

#include        <execinfo.h>
#include        <signal.h>

#include	"mpi.h"
#define         TERMINAL_PROC 0
#define         ROUTER0_PROC 1
#define         ROUTER1_PROC 1
#define         TOT_PROCS 2


void 
sigsegv_handler(int sig)
{
    void *array[10];
    size_t size;

    //    get void*'s for all entries on the stack
    size = backtrace(array, 10);

    //print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, 4);
    MPI_Abort(MPI_COMM_WORLD,1);
    exit(1);
}

void
dump_configuration ( void )
{

    return ;
}


int Mytid; //task id
FILE* log_file;

manifold::kernel::Clock* master_clock = new manifold::kernel::Clock(1);  //clock has to be global or static.
int
main ( int argc, char *argv[] )
{
//    int tmp; printf(" For debug ENTER NO"); scanf("%d", &tmp);
    srandom(time(NULL));
    manifold::kernel::Manifold::Init(argc,argv);
    signal(SIGSEGV, sigsegv_handler);
    if( TOT_PROCS != manifold::kernel::TheMessenger.get_node_size()) {
        cerr << "ERROR: Incorrect no of procs \"-np <no_procs>\" for mpirun!" << endl;
        return 1;
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &Mytid);

    ullint sim_start_time = time(NULL);
    unsigned long long int max_simtime;
    if(argc<2)
    {
        cout << "Error: Requires config file for input parameters\n";
        return 1;
    }

    if ( Mytid == 0)
    {
        cerr << "\n-----------------------------------------------------------------------------------\n";
        cerr << "** IRIS - Cycle Accurate Network Simulator for On-Chip Networks **\n";
        cerr << "-- Computer Architecture and Systems Lab                                         --\n"
            << "-- Georgia Institute of Technology                                               --\n"
            << "-----------------------------------------------------------------------------------\n";
        cerr << "Cmd line: ";
        for( int i=0; i<argc; i++)
            cerr << argv[i] << " ";
        cerr << endl;
    }

    /* Config arguments */
    uint no_nodes =1;

    /* Main components of the node.. will be moved inside topology */
    IrisTerminal* terminal = NULL;
    IrisInterface* ninterface0 = NULL;
    IrisInterface* ninterface1 = NULL;
    IrisRouter* router0 = NULL;
    IrisRouter* router1 = NULL;
    IrisTerminal* mc = NULL;

    string top_string;
    map<string,string> params;
    ifstream fd(argv[1]);
    string data;
    while(!fd.eof())
    {
        getline(fd,data);
        string simknob = data.substr(0,data.find("#"));

        if ( simknob.find('-') == 0 )
        {
            string key,value;
            istringstream iss( simknob, istringstream::in);
            iss >> key;
            string sim_string = key.substr(0,key.find(":"));
            if ( sim_string.compare("-iris") == 0 )
            {
                iss >> value;
                if (! key.compare("-iris:noc_topology"))
                    top_string = value;
                if (! key.compare("-iris:no_nodes"))
                    params.insert(pair<string,string>("no_nodes",value));
                if (! key.compare("-iris:no_mcs"))
                    params.insert(pair<string,string>("no_mcs",value));
                if (! key.compare("-iris:no_ports"))
                    params.insert(pair<string,string>("no_ports",value));
                if (! key.compare("-iris:router_vcs"))
                    params.insert(pair<string,string>("no_vcs",value));
                if (! key.compare("-iris:credits"))
                    params.insert(pair<string,string>("credits",value));
                if (! key.compare("-iris:int_buff_width"))
                    params.insert(pair<string,string>("int_buff_width",value));
                if (! key.compare("-iris:rc_method"))
                    params.insert(pair<string,string>("rc_method",value));
                if (! key.compare("-iris:mean_irt"))
                    params.insert(pair<string,string>("mean_irt",value));
                if (! key.compare("-iris:link_width"))
                    params.insert(pair<string,string>("link_width",value));
                if (! key.compare("-iris:max_simtime"))
                    max_simtime = atoi(value.c_str());
                if (! key.compare("-iris:self_assign_dest_id"))
                    params.insert(pair<string,string>("self_assign_dest_id",value));
            }
            else if ( sim_string.compare("-mc") == 0 )
            {
                iss >> value;
                if (! key.compare("-mc:resp_payload_len"))
                    params.insert(pair<string,string>("resp_payload_len",value));
                if (! key.compare("-mc:memory_latency"))
                    params.insert(pair<string,string>("memory_latency",value));
                if (! key.compare("-mc:max_mc_buffer_size"))
                    params.insert(pair<string,string>("max_mc_buffer_size",value));
            }
            else
                cerr << " config line unk" << endl;

        }
    }

    topology_type noc_topology = TWO_NODE;
    if ( top_string.compare("TWONODE") == 0)
        noc_topology = TWO_NODE;
    if ( top_string.compare("RING") == 0)
        noc_topology = RING;

    /* Create the components */
    manifold::kernel::CompId_t terminal_id = manifold::kernel::Component::Create<PktGen>(TERMINAL_PROC);
    manifold::kernel::CompId_t ninterface0_id = manifold::kernel::Component::Create<NInterface>(TERMINAL_PROC);
    manifold::kernel::CompId_t router0_id = manifold::kernel::Component::Create<SimpleRouter>(ROUTER0_PROC);
    manifold::kernel::CompId_t mc_id = manifold::kernel::Component::Create<SimpleMC>(TERMINAL_PROC);
    manifold::kernel::CompId_t ninterface1_id = manifold::kernel::Component::Create<NInterface>(TERMINAL_PROC);
    manifold::kernel::CompId_t router1_id = manifold::kernel::Component::Create<SimpleRouter>(ROUTER1_PROC);

    uint LATENCY = 1;
    /* Register clocks and pass config parameters */
    if ( Mytid == TERMINAL_PROC )
    {
        terminal = manifold::kernel::Component::GetComponent<IrisTerminal>(terminal_id);
        ninterface0 = manifold::kernel::Component::GetComponent<IrisInterface>(ninterface0_id);
        ninterface1 = manifold::kernel::Component::GetComponent<IrisInterface>(ninterface1_id);
        mc = manifold::kernel::Component::GetComponent<IrisTerminal>(mc_id);

        assert ( ninterface0 != NULL );
        assert ( terminal != NULL );
        assert ( ninterface1 != NULL );
        assert ( mc != NULL );

        /* Register the clock for clocked components */
        manifold::kernel::Clock::Register<IrisTerminal>(terminal, &IrisTerminal::tick, &IrisTerminal::tock);
        manifold::kernel::Clock::Register<IrisInterface>(ninterface0, &IrisInterface::tick, &IrisInterface::tock);
        manifold::kernel::Clock::Register<IrisInterface>(ninterface1, &IrisInterface::tick, &IrisInterface::tock);
        manifold::kernel::Clock::Register<IrisTerminal>(mc, &IrisTerminal::tick, &IrisTerminal::tock);

        /* Pass config knobs */
        ninterface0->parse_config(params);
        ninterface1->parse_config(params);
        terminal->parse_config(params);
        mc->parse_config(params);
    }
    if ( Mytid == ROUTER0_PROC )
    {
        router0 = manifold::kernel::Component::GetComponent<IrisRouter>(router0_id);
        assert ( router0 != NULL );
        manifold::kernel::Clock::Register<IrisRouter>(router0, &IrisRouter::tick, &IrisRouter::tock);
        router0->parse_config(params);
    }
    if ( Mytid == ROUTER1_PROC)
    {
        router1 = manifold::kernel::Component::GetComponent<IrisRouter>(router1_id);
        assert ( router1 != NULL );
        manifold::kernel::Clock::Register<IrisRouter>(router1, &IrisRouter::tick, &IrisRouter::tock);
        router1->parse_config(params);
    }
    if ( Mytid != TERMINAL_PROC && Mytid != ROUTER0_PROC && Mytid != ROUTER1_PROC ) 
    {
        _sim_exit_now ( "Unk process %d", Mytid);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if ( Mytid == TERMINAL_PROC)
    {
        std::cout << " ninterface0 id : " << ninterface0_id << std::endl;
        std::cout << " ninterface1 id : " << ninterface1_id << std::endl;
        std::cout << " proc id : " << terminal_id << std::endl;
        std::cout << " mc id : " << mc_id << std::endl;

        std::cout << " router0 id : " << router0_id << std::endl;
        std::cout << " router1 id : " << router1_id << std::endl;
    }

    Topology* tp = Topology::get_topology(noc_topology);
    tp->parse_config(params);

    params.clear();
    cout << " Done creating & reg clk for components on " << Mytid << endl; 

    if ( Mytid == TERMINAL_PROC)
    {
        /*  Set node ids */
        ninterface0->node_id = 0;
        ninterface1->node_id = 1;
        terminal->node_id = 0;
        mc->node_id = 1;

        tp->terminals.push_back(terminal);
        tp->terminals.push_back(mc);
        tp->interfaces.push_back(ninterface0);
        tp->interfaces.push_back(ninterface1);

        tp->connect_interface_terminal();
        // Connect interfaces and terminals
        tp->connect_interface_terminal();
    }

    MPI_Barrier(MPI_COMM_WORLD);   // insert in order
    if ( Mytid == ROUTER0_PROC)
    {
        router0->node_id = 0;
        tp->routers.push_back(router0);
    }

    MPI_Barrier(MPI_COMM_WORLD);   // insert in order
    if ( Mytid == ROUTER1_PROC)
    {
        router1->node_id = 1;
        tp->routers.push_back(router1);
    }

    // Component ids exist here so insert all comp ids
    tp->terminal_ids.push_back(terminal_id);
    tp->terminal_ids.push_back(mc_id);
    tp->interface_ids.push_back(ninterface0_id);
    tp->interface_ids.push_back(ninterface1_id);
    tp->router_ids.push_back(router0_id);
    tp->router_ids.push_back(router1_id);

    if ( Mytid == ROUTER0_PROC)
        tp->set_router_outports(0);

    if ( Mytid == ROUTER1_PROC)
        tp->set_router_outports(1);

    tp->connect_interface_routers();

    tp->connect_routers();


    /* Init the components */
    if ( Mytid == TERMINAL_PROC )
    {
        terminal->init();
        mc->init();
        ninterface0->init();
        ninterface1->init();
    }

    if ( Mytid == ROUTER0_PROC )
        router0->init();

    if ( Mytid == ROUTER1_PROC )
        router1->init();

    MPI_Barrier(MPI_COMM_WORLD);   // All components have finished Init
    std::cerr << "\n############# Start Timing Simulation for " << max_simtime << " cycles Proc:"<< Mytid;
    manifold::kernel::Manifold::StopAt(max_simtime);
    manifold::kernel::Manifold::Run();


    if ( Mytid == TERMINAL_PROC )
    {
        cerr << tp->print_stats();
        ullint sim_time_ms = (time(NULL) - sim_start_time);
        cerr << "\n\n************** Simulation Stats ***************\n";
        cerr << " Simulation Time: " << sim_time_ms << endl;
    }

    MPI_Finalize();
    exit(0);
}				/* ----------  end of function main  ---------- */


#endif   /* ----- #ifndef _simmc2mesh_cc_INC  ----- */
