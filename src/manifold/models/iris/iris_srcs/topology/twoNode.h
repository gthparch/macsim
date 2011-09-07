/*
 * =====================================================================================
 *
 *! \brief Filename:  twoNode.h
 *
 *    Description: The class defines functions for a generic k-ary 2D twoNode
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

#ifndef  _twoNode_h_INC
#define  _twoNode_h_INC

#include 	"../interfaces/topology.h"


class TwoNode : public Topology
{
    public:
        TwoNode ();
        ~TwoNode ();

        void parse_config(std::map<std::string,std::string>& p);
        
        void connect_interface_terminal(void);
        void connect_interface_routers(void);
        void connect_routers(void);
        void assign_node_ids(component_type t);

        std::string print_stats(void);
        void set_router_outports( uint n);

    protected:

    private:
        uint ports;
        uint vcs;
        uint credits;
        uint buffer_size;
        uint no_nodes;
        uint links;
        uint grid_size;
        uint no_terminals;

}; /* -----  end of class TwoNode  ----- */

#endif   /* ----- #ifndef _twoNode_h_INC  ----- */
