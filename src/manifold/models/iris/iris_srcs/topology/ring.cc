#ifndef  _ring_cc_INC
#define  _ring_cc_INC

#include        "ring.h"
#include	"../components/manifoldProcessor.h"
#include  "memory.h"
Ring::Ring(macsim_c* simBase)
{
	m_simBase = simBase;
}

Ring::~Ring()
{
    for ( uint i=0 ; i<no_nodes; i++ )
    {
        delete interfaces[i];
        delete routers[i];
    }

}

vector<uint> &Ring::split(const string &s, char delim, vector<uint> &elems) {
    stringstream ss(s);
    string item;
    while(getline(ss, item, delim)) {
        elems.push_back(atoi(item.c_str()));
    }
    return elems;
}

vector<uint> Ring::split(const std::string &s, char delim) {
    vector<uint> elems;
    return split(s, delim, elems);
}
void
Ring::parse_config(std::map<std::string, std::string>& p)
{
    /* Defaults */
    no_nodes = 3;
    
    ports = 3;
    vcs = 2;
    credits = 1;

    
    /* Override defaults */
    map<std::string, std::string>:: iterator it;
    it = p.find("no_nodes");
    if ( it != p.end())
        no_nodes = atoi((it->second).c_str());

    it = p.find("no_ports");
    if ( it != p.end())
        ports = atoi((it->second).c_str());

    it = p.find("credits");
    if ( it != p.end())
        credits = atoi((it->second).c_str());

    it = p.find("no_vcs");
    if ( it != p.end())
        vcs = atoi((it->second).c_str());
    
    it = p.find("mapping");
    if ( it != p.end())
        mapping = split(it->second, ',');

    /*print asci map of network */
    cout << "Ring network\n";
    char* message_class_type[] = {"INVALID", "PROC", "L1", "L2", "L3", "MC"};
    int j;
    for( j=0; j<no_nodes/2; j++)
    {
        int macsim_node_id = mapping[j];
        int type = m_simBase->m_macsim_terminals.at(macsim_node_id)->mclass;
        cout << message_class_type[type] << ":" << macsim_node_id;
        if (j+1 < no_nodes/2 ) cout <<" <--> ";
    }
    cout << " <-v\n^\n|-> ";
    for( int i=no_nodes-1; i>= j; i--)
    {
        int macsim_node_id = mapping[i];
        int type = m_simBase->m_macsim_terminals.at(macsim_node_id)->mclass;
        cout << message_class_type[type] << ":" << macsim_node_id;
        if (i-1 >= j ) cout <<" <--> ";
    }
    cout << "<-^\n";

    return;
}

std::string
Ring::print_stats()     
{
    std::stringstream str;
    for( uint i=0; i<no_nodes; i++)
        str << terminals[i]->print_stats();

    return str.str();
}

void
Ring::connect_interface_terminal()//std::vector <ManifoldProcessor*> &g_macsim_terminals)
{

    /* Connect INTERFACE -> TERMINAL*/
    m_simBase->m_memory->m_iris_node_id = (int*)malloc(sizeof(int)*m_simBase->m_macsim_terminals.size());
    for( uint i=0; i < m_simBase->m_macsim_terminals.size(); i++)
    { 
        m_simBase->m_macsim_terminals.at(i)->ni = static_cast<manifold::kernel::Component*>(interfaces.at(mapping[i]));
        interfaces.at(mapping[i])->terminal = static_cast<manifold::kernel::Component*>(m_simBase->m_macsim_terminals.at(i));
        m_simBase->m_macsim_terminals.at(mapping[i])->node_id = i;
        m_simBase->m_memory->m_iris_node_id[i] = mapping[i];
	      //handled in init_network in main.cc
	      //interfaces.at(mapping[i])->node_id = mapping[i];
        //assert ( interfaces.at(i)->terminal != NULL );
        //assert ( m_simBase->m_macsim_terminals.at(i)->ni!= NULL );
    }
    return;
}

void
Ring::connect_interface_routers()
{
    int LATENCY = 1;
    /*  Connect for the output links of the router */
    for( uint i=0; i<no_nodes; i++)
    {
        /*  Interface DATA */
        manifold::kernel::Manifold::Connect(interface_ids.at(i), 0, 
                                            router_ids.at(i), 2*ports,
                                            &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));
        manifold::kernel::Manifold::Connect(router_ids.at(i), 0, 
                                            interface_ids.at(i), 2,
                                            &IrisInterface::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

        /*  Interface SIGNAL */
        manifold::kernel::Manifold::Connect(interface_ids.at(i), 1, 
                                            router_ids.at(i), 3*ports,
                                            &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));
        manifold::kernel::Manifold::Connect(router_ids.at(i), ports, 
                                            interface_ids.at(i), 3,
                                            &IrisInterface::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));
	
	/* assign node id to router */
	//routers.at(i)->node_id = interfaces.at(i)->node_id; //handled in init_network
	//assert ( routers.at(i)->node_id == i );

    }

    return;
}

void
Ring::set_router_outports(uint ind)
{
    for ( uint i=0; i<ports; i++)
    {
        routers.at(ind)->data_outports.push_back(i);
        routers.at(ind)->signal_outports.push_back(ports+i);
    }

    return;
}

void
Ring::connect_routers()
{
    int LATENCY = 1;

    // Configure east - west links for the routers.. in order first WEST then
    // EAST
    for ( uint i=1; i<no_nodes; i++)
    {

#ifdef _DBG_TOP
        int tmp4 = router_ids.at(i);
        int tmp6 = router_ids.at(i-1);
        printf ( "\n Connect %d p%d -> %d p%d", tmp4,1, tmp6, 2*ports+2);
        printf ( "\n Connect %d p%d -> %d p%d", tmp4,ports+1, tmp6, 3*ports+2);

        printf ( "\n Connect %d p%d -> %d p%d", tmp6,2, tmp4, 2*ports+1);
        printf ( "\n Connect %d p%d -> %d p%d", tmp6,ports+2, tmp4, 3*ports+1);
#endif

        // going west  <-
        /* Router->Router DATA */
        
        manifold::kernel::Manifold::Connect(router_ids.at(i), 1, 
                                            router_ids.at(i-1), 2*ports+2,
                                            &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

        /* Router->Router SIGNAL */
        manifold::kernel::Manifold::Connect(router_ids.at(i), ports+1, 
                                            router_ids.at(i-1), 3*ports+2,
                                            &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

        // going east  ->
        /* Router->Router DATA */
        manifold::kernel::Manifold::Connect(router_ids.at(i-1), 2, 
                                            router_ids.at(i), 2*ports+1,
                                            &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

        /* Router->Router SIGNAL */
        manifold::kernel::Manifold::Connect(router_ids.at(i-1), ports+2, 
                                            router_ids.at(i), 3*ports+1,
                                            &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));
    }

#ifdef _DBG_TOP
        int tmp4 = router_ids.at(0);
        int tmp6 = router_ids.at(no_nodes-1);
        printf ( "\n Connect %d p%d -> %d p%d", tmp4,1, tmp6, 2*ports+2);
        printf ( "\n Connect %d p%d -> %d p%d", tmp4,ports+1, tmp6, 3*ports+2);

        printf ( "\n Connect %d p%d -> %d p%d", tmp6,2, tmp4, 2*ports+1);
        printf ( "\n Connect %d p%d -> %d p%d", tmp6,ports+2, tmp4, 3*ports+1);
#endif

    // router 0 and end router
    // going west <-
    manifold::kernel::Manifold::Connect(router_ids.at(0), 1, 
                                        router_ids.at(no_nodes-1), 2*ports+2,
                                        &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

    manifold::kernel::Manifold::Connect(router_ids.at(0), ports+1, 
                                        router_ids.at(no_nodes-1), 3*ports+2,
                                        &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));
    
    // going east ->
    manifold::kernel::Manifold::Connect(router_ids.at(no_nodes-1), 2, 
                                        router_ids.at(0), 2*ports+1,
                                        &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));
    manifold::kernel::Manifold::Connect(router_ids.at(no_nodes-1), ports+2, 
                                        router_ids.at(0), 3*ports+1,
                                        &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));


    return;
}

void
Ring::assign_node_ids( component_type t)
{
    switch ( t)
    {
        case IRIS_TERMINAL:
            {
                for ( uint i=0; i<no_nodes; i++)
                    terminals.at(i)->node_id = i;
                break;
            }
        case IRIS_INTERFACE:
            {
                for ( uint i=0; i<no_nodes; i++)
                    interfaces.at(i)->node_id = i;
                break;
            }
        case IRIS_ROUTER:
            {
                for ( uint i=0; i<no_nodes; i++)
                    routers.at(i)->node_id = i;
                break;
            }
        default:
            {
                _sim_exit_now(" ERROR: assign_node_ids %d", t);
                break;
            }
    }
}
#endif   /* ----- #ifndef _ring_cc_INC  ----- */

