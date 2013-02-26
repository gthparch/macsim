/*
 * =====================================================================================
 *
 *       Filename:  ninterface.h
 *
 *    Description:  This file contains the description for class
 *    NInterface . It models a simple network interface for
 *    handling interaction between the processor and a router within the
 *    network.
 *    It is modelled as a component within manifold
 *
 *
 *        Version:  1.0
 *        Created:  02/08/2011 10:50:32 AM
 *       Revision:  none
 *       Compiler:  g++/mpicxx
 *
 *         Author:  Mitchelle Rasquinha
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */


#ifndef  INTERFACE_H_INC
#define  INTERFACE_H_INC

#include	"../../interfaces/genericHeader.h"
#include	"../../interfaces/irisRouter.h"
#include	"../../interfaces/irisInterface.h"
#include	"manifoldProcessor.h"
#include	"../data_types/linkData.h"
#include	"genericBuffer.h"

//extern class Clock* master_clock;
class FlitLevelPacket;

class SimpleArbiter
{
    public:
        SimpleArbiter ();
        ~SimpleArbiter();
        void init();
        bool is_requested(uint ch);
        bool is_empty();
        void request( uint ch);
        uint pick_winner();

        uint no_channels;
        uint last_winner;
    protected:

    private:
        std::vector < bool > requests;

}; 

class NInterface: public IrisInterface
{
    private: 
	NInterface(); //Do not implement

    public:
        /* ====================  LIFECYCLE     ======================================= */
        NInterface (macsim_c* simBase); 
        NInterface (uint nid):IrisInterface(nid){} 
        ~NInterface (); 

        /* ====================  Event handlers     ======================================= */
        void handle_issue_pkt_event( int inputid, uint64_t data); 
        void handle_send_credit_event( int inputid, uint64_t data); 
        void handle_update_credit_event( int inputid, uint64_t data){}

        void handle_new_packet_event( int port, NetworkPacket* data); 
        void handle_link_arrival( int port, LinkData* data); 
        void handle_update_credit_event( int data); 

        /* ====================  Clocked funtions  ======================================= */
        void tick (void);
        void tock (void);

        void init();
        void parse_config(std::map<std::string,std::string>& p);

    protected:

    private:
        uint no_vcs;
        uint credits;
        uint last_inpkt_winner;
        uint link_width;
        uint ports;

        std::vector < int > downstream_credits;
        std::vector < bool > proc_credits;
        std::vector < uint > proc_ob_flit_index;
        std::vector < uint > proc_ib_flit_index;
        GenericBuffer router_in_buffer;
        GenericBuffer router_out_buffer;
        std::vector<FlitLevelPacket> proc_in_buffer;
        std::vector<FlitLevelPacket> proc_out_buffer;
        std::vector<bool> router_ob_packet_complete;

        SimpleArbiter arbiter;

        uint64_t stat_packets_in;
        uint64_t stat_packets_out;

	macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */
}; /* -----  end of class NInterface  ----- */

#endif   /* ----- #ifndef INTERFACE_H_INC  ----- */
