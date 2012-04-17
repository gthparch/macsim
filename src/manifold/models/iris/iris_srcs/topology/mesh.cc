#ifndef  _Mesh_cc_INC
#define  _Mesh_cc_INC

#include  "mesh.h"
#include	"manifold/models/iris/iris_srcs/components/manifoldProcessor.h"
#include  "memory.h"
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>

using namespace std;



//void parse_mapping(string &mapping) {
//    //string mapping = "1 25 65 85 2 6 5 12";
//    istringstream iss(mapping);
//    cout << "printing mapping layout "; //"of string " << mapping << "\n";
//    vector<string> result = split(mapping, ',');
//    vector<string>::iterator it;
//    for(it=result.begin(); it != result.end(); ++it)
//    {
//      cout << *it << " ";
//    }
//    cout << "\n";
//}


Mesh::Mesh(macsim_c* simBase)
{
	m_simBase = simBase;
}

Mesh::~Mesh()
{
    for ( uint i=0 ; i<no_nodes; i++ )
    {
        delete interfaces[i];
        delete routers[i];
    }

}

vector<uint> &Mesh::split(const string &s, char delim, vector<uint> &elems) {
    stringstream ss(s);
    string item;
    while(getline(ss, item, delim)) {
        elems.push_back(atoi(item.c_str()));
    }
    return elems;
}

vector<uint> Mesh::split(const std::string &s, char delim) {
    vector<uint> elems;
    return split(s, delim, elems);
}

void
Mesh::parse_config(std::map<std::string, std::string>& p)
{
    /* Defaults */
    no_nodes = 4;
    grid_size = 4;
    ports = 3;
    vcs = 2;
    credits = 1;

    
    /* Override defaults */
    map<std::string, std::string>:: iterator it;
    it = p.find("no_nodes");
    if ( it != p.end())
        no_nodes = atoi((it->second).c_str());
    
    it = p.find("grid_size");
    if ( it != p.end())
        grid_size = atoi((it->second).c_str());
    
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
    cout << "Mesh network\n";
    char* message_class_type[] = {"INVALID", "PROC", "L1", "L2", "L3", "MC"};
    for(int i=0; i<grid_size; i++)
    {
        for(int j=0; j<grid_size; j++)
        {
//            cout << mapping[i] << "\t"; // << mapping[i*grid_size+j] << ":" 
            int macsim_node_id = mapping[i*grid_size+j];
            int type = m_simBase->m_macsim_terminals.at(macsim_node_id)->mclass;
            cout << message_class_type[type] << ":" << macsim_node_id <<"\t";
        }
        cout << "\n";
    }

    return;
}

std::string
Mesh::print_stats(component_type ty)     
{
    std::stringstream str;
    switch ( ty ) {
        case IRIS_TERMINAL:	
            for( uint i=0; i<terminals.size(); i++)
                str << terminals[i]->print_stats();
            break;

        case IRIS_INTERFACE:	
            break;

        case IRIS_ROUTER:	 
            for( uint i=0; i<routers.size(); i++)
                str << routers[i]->print_stats();
            break;

        default:	
            break;
    }				/* -----  end switch  ----- */

    return str.str();
}

std::string
Mesh::print_stats()     
{
    std::stringstream str;
    for( uint i=0; i<no_nodes; i++)
        str << terminals[i]->print_stats();

    return str.str();
}

void
Mesh::connect_interface_terminal()
{
    /* Connect INTERFACE -> TERMINAL*/
    m_simBase->m_memory->m_iris_node_id = (int*)malloc(sizeof(int)*m_simBase->m_macsim_terminals.size());
    for( uint i=0; i < m_simBase->m_macsim_terminals.size(); i++)
    { 
        m_simBase->m_macsim_terminals.at(i)->ni = static_cast<manifold::kernel::Component*>(interfaces.at(mapping[i]));
        interfaces.at(mapping[i])->terminal = static_cast<manifold::kernel::Component*>(m_simBase->m_macsim_terminals.at(i));
        
        m_simBase->m_macsim_terminals.at(mapping[i])->node_id = i;  //iris node id
        m_simBase->m_memory->m_iris_node_id[i] = mapping[i];        //iris node id
    }
    
    return;
}

void
Mesh::connect_interface_routers()
{
    int LATENCY = 1;
    /*  Connect for the output links of the router */
    for( uint i=0; i<no_nodes; i++)
    {
        uint iid = interface_ids.at(i);
        uint rid = router_ids.at(i);
#ifdef _DBG_TOP
            printf ( "\n Connect %d p%d -> %d p%d", iid,0, rid, 2*ports);
            printf ( "\n Connect %d p%d -> %d p%d", rid,0, iid, 2);

            printf ( "\n Connect %d p%d -> %d p%d", iid,1, rid, 3*ports);
            printf ( "\n Connect %d p%d -> %d p%d", rid,ports, iid, 3);
#endif
            // going west  <-
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

        
    }

    return;
}

void
Mesh::set_router_outports(uint ind)
{
    for ( uint i=0; i<ports; i++)
    {
        routers.at(ind)->data_outports.push_back(i);
        routers.at(ind)->signal_outports.push_back(ports+i);
    }

    return;
}

void
Mesh::connect_routers()
{
    int LATENCY = 1;

    // Configure east - west links for the routers.. in order first WEST then
    for ( uint i=0; i<grid_size; i++)
        for ( uint j=1; j<grid_size; j++)
        {
            uint rno = router_ids.at(i*grid_size + j);
            uint rno2 = router_ids.at(i*grid_size + j-1);

#ifdef _DBG_TOP
            printf ( "\n Connect %d p%d -> %d p%d", rno,1, rno2, 2*ports+2);
            printf ( "\n Connect %d p%d -> %d p%d", rno,ports+1, rno2, 3*ports+2);

            printf ( "\n Connect %d p%d -> %d p%d", rno2,2, rno, 2*ports+1);
            printf ( "\n Connect %d p%d -> %d p%d", rno2,ports+2, rno, 3*ports+1);
#endif
            // going west  <-
            /* Router->Router DATA */
            manifold::kernel::Manifold::Connect(rno, 1, 
                                                rno2, 2*ports+2,
                                                &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

            /* Router->Router SIGNAL */
            manifold::kernel::Manifold::Connect(rno, ports+1, 
                                                rno2, 3*ports+2,
                                                &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

            // going east  ->
            /* Router->Router DATA */
            manifold::kernel::Manifold::Connect(rno2, 2, 
                                                rno, 2*ports+1,
                                                &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

            /* Router->Router SIGNAL */
            manifold::kernel::Manifold::Connect(rno2, ports+2, 
                                                rno, 3*ports+1,
                                                &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));
            
        uint west_id = ( 0 == i % grid_size ) ? -1 : i - 1;
        uint east_id = ( 0 == (i+1) % grid_size ) ? -1 : i + 1;
        // going north
        uint north_id = (i < grid_size) ? -1 : i - grid_size;
                                            
        // going south
        uint south_id = (i + grid_size >= no_nodes) ? -1 : i + grid_size;
        }

    //connect north south
    for ( uint i=1; i<grid_size; i++)
        for ( uint j=0; j<grid_size; j++)
        {
            uint rno = router_ids.at(i*grid_size + j);
            uint up_rno =router_ids.at(i*grid_size + j - grid_size);

#ifdef _DBG_TOP
            printf ( "\n Connect %d p%d -> %d p%d", rno,3, up_rno, 2*ports+4);
            printf ( "\n Connect %d p%d -> %d p%d", rno,ports+3, up_rno, 3*ports+4);

            printf ( "\n Connect %d p%d -> %d p%d", up_rno,4, rno, 2*ports+3);
            printf ( "\n Connect %d p%d -> %d p%d", up_rno,ports+4, rno, 3*ports+3);
#endif


            // going north ^
            // Router->Router DATA 
            manifold::kernel::Manifold::Connect(rno, 3, 
                                                up_rno, 2*ports+4,
                                                &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

            // Router->Router SIGNAL 
            manifold::kernel::Manifold::Connect(rno, ports+3, 
                                                up_rno, 3*ports+4,
                                                &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

            // going south->
            // Router->Router DATA 
            manifold::kernel::Manifold::Connect(up_rno, 4, 
                                                rno, 2*ports+3,
                                                &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

            // Router->Router SIGNAL 
            manifold::kernel::Manifold::Connect(up_rno, ports+4, 
                                                rno, 3*ports+3,
                                                &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));
        }

    // connect edge nodes East-west
    for ( uint i=0; i<grid_size; i++)
    {
        uint rno = router_ids.at(i*grid_size);
        uint end_rno = router_ids.at(i*grid_size+grid_size-1);

#ifdef _DBG_TOP
        printf ( "\n Connect %d p%d -> %d p%d", rno,1, end_rno, 2*ports+2);
        printf ( "\n Connect %d p%d -> %d p%d", rno,ports+1, end_rno, 3*ports+2);

        printf ( "\n Connect %d p%d -> %d p%d", end_rno,2, rno, 2*ports+1);
        printf ( "\n Connect %d p%d -> %d p%d", end_rno,ports+2, rno, 3*ports+1);
#endif

        // router 0 and end router of every row
        // going west <-
        manifold::kernel::Manifold::Connect(rno, 1, 
                                            (end_rno), 2*ports+2,
                                            &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

        manifold::kernel::Manifold::Connect((rno), ports+1, 
                                            (end_rno), 3*ports+2,
                                            &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

        // going east ->
        manifold::kernel::Manifold::Connect(end_rno, 2, 
                                            rno, 2*ports+1,
                                            &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));
        manifold::kernel::Manifold::Connect(end_rno, ports+2, 
                                            rno, 3*ports+1,
                                            &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

     }


    // connect edge nodes north-south
    for ( uint i=0; i<grid_size; i++)
    {
        uint rno = router_ids.at(i);
        uint end_rno = router_ids.at(grid_size*(grid_size-1)+i);

#ifdef _DBG_TOP
        printf ( "\n Connect %d p%d -> %d p%d", rno,3, end_rno, 2*ports+4);
        printf ( "\n Connect %d p%d -> %d p%d", rno,ports+3, end_rno, 3*ports+4);

        printf ( "\n Connect %d p%d -> %d p%d", end_rno,4, rno, 2*ports+3);
        printf ( "\n Connect %d p%d -> %d p%d", end_rno,ports+4, rno, 3*ports+3);
#endif

        // router 0 and end router of every row
        // going west <-
        manifold::kernel::Manifold::Connect((rno), 3, 
                                            (end_rno), 2*ports+4,
                                            &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

        manifold::kernel::Manifold::Connect((rno), ports+3, 
                                            (end_rno), 3*ports+4,
                                            &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

        // going east ->
        manifold::kernel::Manifold::Connect(end_rno, 4, 
                                            rno, 2*ports+3,
                                            &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));
        manifold::kernel::Manifold::Connect(end_rno, ports+4, 
                                            rno, 3*ports+3,
                                            &IrisRouter::handle_link_arrival , static_cast<manifold::kernel::Ticks_t>(LATENCY));

     }

    return;
}


void
Mesh::assign_node_ids( component_type t)
{
    switch ( t)
    {
        case IRIS_TERMINAL:
            {
                for ( uint i=0; i<terminals.size(); i++)
                    terminals.at(i)->node_id = i;
                break;
            }
        case IRIS_INTERFACE:
            {
                for ( uint i=0; i<interfaces.size(); i++)
                    interfaces.at(i)->node_id = i;
                break;
            }
        case IRIS_ROUTER:
            {
                for ( uint i=0; i<routers.size(); i++)
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
#endif   /* ----- #ifndef _Mesh_cc_INC  ----- */

