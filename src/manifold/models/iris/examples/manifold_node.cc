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
#include        "manifoldTerminal.h"
#include        "irisInterface.h"
#include        "irisRouter.h"

#include        "kernel/clock.h"
#include        "kernel/manifold.h"
#include        "kernel/component.h"

#include        "../iris_srcs/components/pktgen.h"
#include        "../iris_srcs/components/simpleRouter.h"
#include        "../iris_srcs/components/simpleMC.h"
#include        "../iris_srcs/components/ninterface.h"

extern void interface_simiris(ullint);




void
dump_configuration ( void )
{

    return ;
}


int Mytid; //task id

manifold::kernel::Clock* master_clock = new manifold::kernel::Clock(1);  //clock has to be global or static.
int
main ( int argc, char *argv[] )
{
    manifold::kernel::Manifold::Init(argc,argv);
    /* 
       if( 1 != TheMessenger.get_node_size()) {
       cerr << "ERROR: Must specify \"-np <no_procs>\" for mpirun!" << endl;
       return 1;
       }
     * */

    //MPI_Comm_rank(MPI_COMM_WORLD, &Mytid);

    ullint sim_start_time = time(NULL);
    unsigned long long int max_simtime;
    if(argc<2)
    {
        cout << "Error: Requires config file for input parameters\n";
        return 1;
    }

    cerr << "\n-----------------------------------------------------------------------------------\n";
    cerr << "** IRIS - Cycle Accurate Network Simulator for On-Chip Networks **\n";
    cerr << "-- Computer Architecture and Systems Lab                                         --\n"
        << "-- Georgia Institute of Technology                                               --\n"
        << "-----------------------------------------------------------------------------------\n";
    cerr << "Cmd line: ";
    for( int i=0; i<argc; i++)
        cerr << argv[i] << " ";
    cerr << endl;

    /* Config arguments */
    uint no_nodes =1;

    /* Main components of the node.. will be moved inside topology */
    ManifoldTerminal* terminal;
    IrisInterface* ninterface0;
    IrisInterface* ninterface1;
    IrisRouter* router0;
    IrisRouter* router1;
    ManifoldTerminal* mc;

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
            }
            else
                cerr << " config line unk" << endl;

        }
    }

    /* Create the components */
    manifold::kernel::CompId_t terminal_id = manifold::kernel::Component::Create<PktGen>(0);
    manifold::kernel::CompId_t ninterface0_id = manifold::kernel::Component::Create<NInterface>(0);
    manifold::kernel::CompId_t router0_id = manifold::kernel::Component::Create<SimpleRouter>(0);
    manifold::kernel::CompId_t mc_id = manifold::kernel::Component::Create<SimpleMC>(0);
    manifold::kernel::CompId_t ninterface1_id = manifold::kernel::Component::Create<NInterface>(0);
    manifold::kernel::CompId_t router1_id = manifold::kernel::Component::Create<SimpleRouter>(0);

    /* Save the component pointers and component ids */
    terminal = manifold::kernel::Component::GetComponent<ManifoldTerminal>(terminal_id);
    ninterface0 = manifold::kernel::Component::GetComponent<IrisInterface>(ninterface0_id);
    router0 = manifold::kernel::Component::GetComponent<IrisRouter>(router0_id);
    router1 = manifold::kernel::Component::GetComponent<IrisRouter>(router1_id);
    ninterface1 = manifold::kernel::Component::GetComponent<IrisInterface>(ninterface1_id);
    mc = manifold::kernel::Component::GetComponent<ManifoldTerminal>(mc_id);

    std::cout << " ninterface0 id : " << ninterface0->GetComponentId() << std::endl;
    std::cout << " ninterface1 id : " << ninterface1->GetComponentId() << std::endl;
    std::cout << " router0 id : " << router0->GetComponentId() << std::endl;
    std::cout << " router1 id : " << router1->GetComponentId() << std::endl;
    if (Mytid == 0) 
    {
        assert ( ninterface0 != NULL );
        assert ( terminal != NULL );
        assert ( ninterface1 != NULL );
        assert ( mc != NULL );
        assert ( router0 != NULL );
    }

    cout << " Done creating components " << endl; 

    /* Register the clock for clocked components */
    manifold::kernel::Clock::Register<ManifoldTerminal>(terminal, &ManifoldTerminal::tick, &ManifoldTerminal::tock);
    manifold::kernel::Clock::Register<IrisInterface>(ninterface0, &IrisInterface::tick, &IrisInterface::tock);
    manifold::kernel::Clock::Register<IrisInterface>(ninterface1, &IrisInterface::tick, &IrisInterface::tock);
    manifold::kernel::Clock::Register<IrisRouter>(router0, &IrisRouter::tick, &IrisRouter::tock);
    manifold::kernel::Clock::Register<IrisRouter>(router1, &IrisRouter::tick, &IrisRouter::tock);
    manifold::kernel::Clock::Register<ManifoldTerminal>(mc, &ManifoldTerminal::tick, &ManifoldTerminal::tock);

    cout << " Done regestering clock objects" << endl; 

    /* Assign node ids */
    //    ninterface0->node_id = ninterface0_id;
    //    ninterface1->node_id = ninterface1_id;
    terminal->node_id = terminal_id;
    mc->node_id = mc_id;
    router0->node_id = router0_id;
    router1->node_id = router1_id;

    /* Pass config knobs */
    ninterface0->parse_config(&params);
    ninterface1->parse_config(&params);
    terminal->parse_config(&params);
    mc->parse_config(&params);
    router0->parse_config(&params);
    router1->parse_config(&params);

    params.clear();

    cout << "*********** manifold::kernel::Component addresses ****"<< endl;
    cout << " Int 0 ID is " << ninterface0_id << endl;
    cout << " Int 1 ID is " << ninterface1_id << endl;
    cout << " Proc ID is " << terminal_id<< endl;
    cout << " MC ID is " << mc_id<< endl;
    cout << " Rot 0  ID is " << router0_id<< endl;
    cout << " rot 1 ID is " << router1_id<< endl;
    /* Connect INTERFACE -> PROCESSOR */
    uint LATENCY = 1;

    ManifoldTile* tile0 = new ManifoldTile();
    ManifoldTile* tile1 = new ManifoldTile();
    tile0->tile_id = 0;
    tile1->tile_id = 1;
    tile0->interface = ninterface0;
    tile0->terminal = terminal;
    tile0->router = router0;
    tile1->interface = ninterface1;
    tile1->terminal = mc;
    tile1->router = router1;

    terminal->tile = tile0;
    ninterface0->tile = tile0;
    router0->tile = tile0;

    mc->tile = tile1;
    ninterface1->tile = tile1;
    router1->tile = tile1;

    /* Connect INTERFACE -> ROUTER */
    ninterface0->router = static_cast<manifold::kernel::Component*>(router0);
    ninterface1->router = router1;

    router0->input_connections.resize(2);
    router1->input_connections.resize(2);
    router0->output_connections.resize(2);
    router1->output_connections.resize(2);

    router0->input_connections[0] = ninterface0_id;
    router0->input_connections[1] = router1_id;
    router1->input_connections[0] = ninterface1_id;
    router1->input_connections[1] = router0_id;

    router0->output_connections[0] = ninterface0_id;
    router0->output_connections[1] = router1_id;
    router1->output_connections[0] = ninterface1_id;
    router1->output_connections[1] = router0_id;
    std::cout << "*** ninterface0 id : " << ninterface0->GetComponentId() << std::endl;
    std::cout << "** ninterface1 id : " << ninterface1->GetComponentId() << std::endl;

    /* this is for send
       router->input_ports_c.push_back(2);
       router->input_ports_c.push_back(3);

       router->output_ports_c.push_back(0);
       router->output_ports_c.push_back(1);
     */

    // none of the pointer connections are needed with connect
    router0->output_link_connections.push_back(ninterface0);
    router0->output_link_connections.push_back(router1);
    router1->output_link_connections.push_back(ninterface1);
    router1->output_link_connections.push_back(router0);

    /*  Connect for the output links of the router 
        manifold::kernel::Manifold::Connect(router_id, 0, 
        ninterface0_id, 0,
        &IrisInterface::handle_link_arrival , static_cast<Ticks_t>(LATENCY));
        manifold::kernel::Manifold::Connect(router_id, 1, 
        ninterface1_id, 0,
        &IrisInterface::handle_link_arrival , static_cast<Ticks_t>(LATENCY));

        manifold::kernel::Manifold::Connect(ninterface0_id, 1, 
        router_id, 2,
        &IrisInterface::handle_link_arrival , static_cast<Ticks_t>(LATENCY));
        manifold::kernel::Manifold::Connect(ninterface1_id, 1, 
        router_id, 3,
        &IrisInterface::handle_link_arrival , static_cast<Ticks_t>(LATENCY));
     *  */
    /* 
       ------------ DATA -----------------
       | SEND_DATA --- handle_new_pkt_event_int ------> RECV_DATA |
       | RECV_DATA <-- handle_new_plt_event_proc ------ SEND_DATA |
       Proc | SEND_SIG ---- handle_update_credit_event_int -> RECV_SIG |  Interface
       | RECV_SIG <-- handle_update_credit_event_proc -- SEND_SIG |

    // Terminal sends new packet to interface
    manifold::kernel::Manifold::Connect(terminal_id, SEND_DATA, 
    ninterface0_id, RECV_DATA,
    &IrisInterface::handle_new_packet_event, static_cast<Ticks_t>(LATENCY));
    // Interface sends new packet to terminal 
    manifold::kernel::Manifold::Connect(ninterface0_id, SEND_DATA, 
    terminal_id, RECV_DATA,
    &PktGen::handle_new_packet_event, static_cast<Ticks_t>(LATENCY));
    // Proc send a credit to the interface 
    manifold::kernel::Manifold::Connect(ninterface0_id, SEND_SIG, 
    terminal_id, RECV_SIG,
    &PktGen::handle_update_credit_event, static_cast<Ticks_t>(LATENCY));
    // Interface sends a credit to the proc 
    manifold::kernel::Manifold::Connect(terminal_id, SEND_SIG, 
    ninterface0_id, RECV_SIG,
    &IrisInterface::handle_update_credit_event, static_cast<Ticks_t>(LATENCY));
     * */

    std::cout << "^^^^^^^^^^^^ ninterface0 id : " << ninterface0->GetComponentId() << std::endl;
    std::cout << "** ninterface1 id : " << ninterface1->GetComponentId() << std::endl;
    /* Init the components */
    terminal->init();
    mc->init();
    ninterface0->init();
    ninterface1->init();
    router0->init();
    router1->init();

    manifold::kernel::Manifold::StopAt(max_simtime);
    manifold::kernel::Manifold::Run();

    // print stats
    cerr << terminal->print_stats();
    cerr << mc->print_stats();
    //MPI_Finalize();

    ullint sim_time_ms = (time(NULL) - sim_start_time);
    cerr << "\n\n************** Simulation Stats ***************\n";
    cerr << " Simulation Time: " << sim_time_ms << endl;
    return 0;
}				/* ----------  end of function main  ---------- */


#endif   /* ----- #ifndef _simmc2mesh_cc_INC  ----- */
