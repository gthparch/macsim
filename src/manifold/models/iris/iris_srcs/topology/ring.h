/*
 * =====================================================================================
 *
 *! \brief Filename:  ring.h
 *
 *    Description: The class defines functions for a generic k-ary 2D ring
 *    with the network being a direct network of size n=k^2.
 *    The links have the following naming convention 
 *    
 *    links going left to right are a links
 *    links going from right to left are b links
 *    links going downwards are a links
 *    links going upwards are b links
 *
 *    Router ports
 *    port 0: Connects to interface
 *    port 1: Connects to direction east
 *    port 2: Connects to direction west
 *    port 3: Connects to direction north
 *    port 4: Connects to direction south
 *
 *              a links
 *              ----> R0 ----> R1 ---->
 *              <---  |^ <---  |^   <---
 *              blinks
 *                    ||       ||
 *                    v|       v| 
 *              ----> R2 ----> R3 ---->
 *                  a  |^  b     
 *                     ||   
 *                  l  ||  l
 *                  i  V|  i
 *                  n      n
 *                  k      k
 *                  s      s
 *
 *        Version:  1.0
 *        Created:  05/05/2010 12:01:12 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha  
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  _ring_h_INC
#define  _ring_h_INC

#include 	"../interfaces/topology.h"
#include 	<vector>
#include 	<assert.h>

extern std::vector<ManifoldProcessor*> g_macsim_terminals;
class Ring : public Topology
{
    private:
	Ring(); //Do not use

    public:
        Ring(macsim_c* simBase);
        ~Ring ();

        void parse_config(std::map<std::string,std::string>& p);
        
        //void connect_interface_terminal(std::vector<ManifoldProcessor*> &m_term);
        void connect_interface_terminal(void);
        void connect_interface_routers(void);
        void connect_routers(void);
        void assign_node_ids( component_type t);

        std::string print_stats(void);
        void set_router_outports( uint n);

        uint no_nodes;
        vector<uint> mapping;
        
    protected:
            vector<uint> &split(const string &s, char delim, vector<uint> &elems);
            vector<uint> split(const std::string &s, char delim);

    private:
        uint ports;
        uint vcs;
        uint credits;
        uint buffer_size;
        uint links;
	
	macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */

}; /* -----  end of class Ring  ----- */

#endif   /* ----- #ifndef _ring_h_INC  ----- */
