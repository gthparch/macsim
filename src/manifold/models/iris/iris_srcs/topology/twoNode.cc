/*
 * =====================================================================================
 *
 *       Filename:  twoNode.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/05/2010 12:37:47 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha  
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */
#ifndef  _twoNode_cc_INC
#define  _twoNode_cc_INC

#include        "twoNode.h"

TwoNode::TwoNode()
{

}

TwoNode::~TwoNode()
{
    for ( uint i=0 ; i<no_nodes; i++ )
    {
        delete terminals[i];
        delete interfaces[i];
        delete routers[i];
    }

}

void
TwoNode::parse_config(std::map<std::string, std::string>& p)
{
    /* Defaults */
    grid_size = 2;
    no_nodes = 2;
    
    ports = 5;
    vcs = 2;
    credits = 1;

    no_terminals =2;
    
    /* Override defaults */
    map<std::string, std::string>:: iterator it;
    it = p.find("grid_size");
    if ( it != p.end())
        grid_size = atoi((it->second).c_str());

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

    return;
}

std::string
TwoNode::print_stats()     
{
    std::stringstream str;
    for( uint i=0; i<no_terminals; i++)
        str << terminals[i]->print_stats();

    return str.str();
}

void
TwoNode::connect_interface_terminal()
{
    /* Connect INTERFACE -> TERMINAL*/
    for( uint i=0; i<no_terminals; i++)
    {
        terminals.at(i)->ni = static_cast<manifold::kernel::Component*>(interfaces.at(i));
        interfaces.at(i)->terminal = static_cast<manifold::kernel::Component*>(terminals.at(i));
//        assert ( interfaces.at(i)->terminal != NULL );
//        assert ( terminals.at(i)->ni!= NULL );
    }
    return;
}

void
TwoNode::assign_node_ids( component_type t)
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

void
TwoNode::connect_interface_routers()
{
    int LATENCY = 1;
    /*  Connect for the output links of the router */
    for( uint i=0; i<no_terminals; i++)
    {
        /*  Interface DATA */
        manifold::kernel::Manifold::Connect(interface_ids.at(i), 0, 
                                            router_ids.at(i), 4,
                                            &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));
        manifold::kernel::Manifold::Connect(router_ids.at(i), 0, 
                                            interface_ids.at(i), 2,
                                            &IrisInterface::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

        /*  Interface SIGNAL */
        manifold::kernel::Manifold::Connect(interface_ids.at(i), 1, 
                                            router_ids.at(i), 6,
                                            &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));
        manifold::kernel::Manifold::Connect(router_ids.at(i), 2, 
                                            interface_ids.at(i), 3,
                                            &IrisInterface::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

    }

    return;
}

void
TwoNode::set_router_outports(uint ind)
{
    routers.at(ind)->data_outports.push_back(0);
    routers.at(ind)->data_outports.push_back(1);

    routers.at(ind)->signal_outports.push_back(2);
    routers.at(ind)->signal_outports.push_back(3);
}

void
TwoNode::connect_routers()
{
    int LATENCY = 1;
    /* Router->Router DATA */
    manifold::kernel::Manifold::Connect(router_ids.at(0), 1, 
                                        router_ids.at(1), 5,
                                        &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));
    manifold::kernel::Manifold::Connect(router_ids.at(1), 1, 
                                        router_ids.at(0), 5,
                                        &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

    /* Router->Router SIGNAL */
    manifold::kernel::Manifold::Connect(router_ids.at(0), 3, 
                                        router_ids.at(1), 7,
                                        &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));
    manifold::kernel::Manifold::Connect(router_ids.at(1), 3, 
                                        router_ids.at(0), 7,
                                        &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

    return;
}

#endif   /* ----- #ifndef _twoNode_cc_INC  ----- */

