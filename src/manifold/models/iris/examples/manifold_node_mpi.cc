/*
 * =====================================================================================
 *
 *       Filename:  manifold_node.cc
 *
 *    Description:  This is a simple frontenf for using IRIS with pktgen and
 *    routers for diff topologies
 *    Can run either single threaded or on two logical processes using MPI.
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
#include	<typeinfo>

#include	"mpi.h"
#define         TERMINAL_PROC 0
#define         ROUTER_PROC 1
#define         TOT_PROCS 2

extern Topology* get_topology( topology_type t);

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
dump_configuration ( std::map< std::string, std::string>& p)
{
    std::cerr << " Knob\tValue\n";
    for ( std::map< std::string, std::string >::const_iterator iter = p.begin();
          iter != p.end(); ++iter )
        std::cerr<< iter->first << '\t' << iter->second << '\n';
    std::cerr << "\n";
    return ;
}

Topology* tp;
uint no_nodes =2;
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

    system(" rm -f log_0.txt");
    system(" rm -f log_1.txt");
    /* Config arguments */
    uint grid_size=2;
    uint no_mcs = 0;
    std::string top_string;
    std::vector<uint> mc_positions;

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
                {
                    params.insert(pair<string,string>("no_nodes",value));
                    no_nodes = atoi(value.c_str());
                }
                if (! key.compare("-iris:grid_size"))
                {
                    params.insert(pair<string,string>("grid_size",value));
                    grid_size= atoi(value.c_str());
                }
                if (! key.compare("-iris:no_mcs"))
                {
                    params.insert(pair<string,string>("no_mcs",value));
                    no_mcs = atoi ( value.c_str());
                }
                if (! key.compare("-iris:no_ports"))
                    params.insert(pair<string,string>("no_ports",value));
                if (! key.compare("-iris:no_vcs"))
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
            else if (! key.compare("-MC_LOC") )
            {
                // make sure grid size is specified before
                uint mc_xpos, mc_ypos;
                iss >> mc_xpos;
                iss >> mc_ypos;
                uint pos = mc_xpos*grid_size+mc_ypos;
                assert( pos < no_nodes);
                mc_positions.push_back(pos);
            }
            else
                cerr << " config line unk" << endl;

        }
    }

    assert ( no_mcs == mc_positions.size());
    if( Mytid == TERMINAL_PROC)
        dump_configuration(params);

    topology_type noc_topology = TWO_NODE;
    if ( top_string.compare("TWONODE") == 0)
        noc_topology = TWO_NODE;
    if ( top_string.compare("RING") == 0)
        noc_topology = RING;

    // TODO: move get_topology to static.. 
    //    static Topology* tp = get_topology(noc_topology);
    tp = get_topology(noc_topology);
    tp->parse_config(params);

    /* Create the components */
    std::vector<uint>::iterator itr;
    for ( uint i=0; i< no_nodes; i++)
    {
        itr = find ( mc_positions.begin(), mc_positions.end(),i);
        if ( itr != mc_positions.end())
            tp->terminal_ids.push_back( manifold::kernel::Component::Create<SimpleMC>(TERMINAL_PROC) );
        else
            tp->terminal_ids.push_back( manifold::kernel::Component::Create<PktGen>(TERMINAL_PROC) );

        tp->interface_ids.push_back( manifold::kernel::Component::Create<NInterface>(TERMINAL_PROC) );
        tp->router_ids.push_back( manifold::kernel::Component::Create<SimpleRouter>(ROUTER_PROC) );
    }

    uint LATENCY = 1;
    /* Register clocks and pass config parameters */
    if ( Mytid == TERMINAL_PROC )
    {
        for ( uint i=0; i< no_nodes; i++)
        {
            IrisTerminal* terminal = manifold::kernel::Component::GetComponent<IrisTerminal>(tp->terminal_ids.at(i));
            IrisInterface* interface = manifold::kernel::Component::GetComponent<IrisInterface>(tp->interface_ids.at(i));

            assert ( interface != NULL );
            assert ( terminal != NULL );

            /* Register the clock for clocked components */
            manifold::kernel::Clock::Register<IrisTerminal>(terminal, &IrisTerminal::tick, &IrisTerminal::tock);
            manifold::kernel::Clock::Register<IrisInterface>(interface, &IrisInterface::tick, &IrisInterface::tock);

            /* Pass config knobs */
            interface->parse_config(params);
            terminal->parse_config(params);

            tp->terminals.push_back(terminal);
            tp->interfaces.push_back(interface);
        }

        tp->assign_node_ids(IRIS_TERMINAL);
        tp->assign_node_ids(IRIS_INTERFACE);

        for ( uint i=0; i< no_nodes; i++)
        {
            IrisTerminal* terminal = tp->terminals.at(i);
            IrisInterface* interface = tp->interfaces.at(i);

            interface->init();
            terminal->init();

#ifdef _DEBUG
            std::cerr << " Terminal " << i <<" id : " << tp->terminal_ids.at(i) 
                <<" Nid: "<< terminal->node_id << " " <<typeid(terminal).name()<< std::endl;
            std::cerr << " Interface" << i <<" id : " << tp->interface_ids.at(i)
                <<" Nid: "<< interface->node_id << std::endl;
#endif
        }

        tp->connect_interface_terminal();
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if ( Mytid == ROUTER_PROC )
    {
        for ( uint i=0; i< no_nodes; i++)
        {
            IrisRouter* rr = manifold::kernel::Component::GetComponent<IrisRouter>(tp->router_ids.at(i));
            assert ( rr != NULL );
            manifold::kernel::Clock::Register<IrisRouter>(rr, &IrisRouter::tick, &IrisRouter::tock);
            rr->parse_config(params);
            tp->routers.push_back(rr);
        }

        tp->assign_node_ids(IRIS_ROUTER);


        for ( uint i=0; i< no_nodes; i++)
        {
            IrisRouter* rr;
            rr = tp->routers.at(i);
            rr->init();
            tp->set_router_outports(i);
#ifdef _DEBUG
            std::cerr << " Router" << i <<" id : " << tp->router_ids.at(i)
                <<" Nid: "<< rr->node_id << std::endl;
#endif
        }
    }

    if ( Mytid != TERMINAL_PROC && Mytid != ROUTER_PROC ) 
    {
        _sim_exit_now ( "Unk process %d", Mytid);
    }

    params.clear();
    MPI_Barrier(MPI_COMM_WORLD);


    tp->connect_interface_routers();
    tp->connect_routers();

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
