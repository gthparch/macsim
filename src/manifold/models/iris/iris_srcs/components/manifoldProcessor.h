/*
 * =====================================================================================
 *
 *       Filename:  manifoldProcessor.h
 *
 *    Description:  This file contains the description for class
 *    ManifoldProcessor. It models a simple stochastic packet generator that 
 *    can be used as an end terminal nodes to IRIS.
 *    It uses gsl ( GNU scientific library ) for computing
 *    1. packet injection time
 *    2. destinaltion node id
 *    It is modelled as a component within manifold
 *
 *        Version:  1.0
 *        Created:  02/07/2011 05:00:39 PM
 *       Revision:  none
 *       Compiler:  g++/ mpicxx
 *
 *         Author:  Mitchelle Rasquinha
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  MANIFOLDPROCESSOR_H_INC
#define  MANIFOLDPROCESSOR_H_INC

#include	"../../interfaces/irisTerminal.h"
#include	"../../interfaces/irisInterface.h"
#include	"kernel/clock.h"
#include	"kernel/manifold.h"
#include	"../data_types/networkPacket.h"
#include	"../../../../../global_defs.h"

//class IrisTerminal;
class NetworkPacket;

class ManifoldProcessor: public IrisTerminal
{
   private:
	ManifoldProcessor (); //Do not implement

   public:
        /* ====================  LIFECYCLE  ======================================= */
        ManifoldProcessor (macsim_c* simBase);
        ~ManifoldProcessor ();

        /* ====================  Event handlers  ======================================= */
        bool send_packet( mem_req_s *req ); 
	void handle_issue_pkt_event( int nothing);//, uint64_t data);
	void handle_issue_pkt_event( int nothing, uint64_t data);
        void handle_send_credit_event( int inputid, uint64_t data ); 
        void handle_new_packet_event( int inputid, NetworkPacket* data);
        void handle_update_credit_event( int inputid, uint64_t data);

        /* ====================  Clocked funtions  ======================================= */
        void tick (void);
        void tock (void);

        /* ====================  Setup functions  ======================================= */
        void init ( void );
        void parse_config( std::map<std::string,std::string> & params);
        std::string print_stats( void ) const;

	manifold::kernel::Component *ni;
	mem_req_s* check_queue();
	uint node_id;
 	std::queue <mem_req_s*> receive_queue;
 	
 	message_class mclass;
 	bool ptx;
 	
    protected:

    private:
        uint ni_buffer_width;
        uint no_nodes;
        uint64_t sent_time;
        uint no_mcs;
	
        std::vector<bool> ni_buffers;        /* Credits for the buffers in the interface */

        /* Stat variables */
        uint64_t stat_packets_in;
        uint64_t stat_packets_out;
        uint64_t stat_last_packet_out_cycle;
        uint64_t stat_last_packet_in_cycle;
        uint64_t stat_total_latency;

        uint get_bit_rev_dest(uint d);
	
	macsim_c* m_simBase;         /**< macsim_c base class for simulation globals */

}; /* -----  end of class ManifoldProcessor  ----- */

#endif   /* ----- #ifndef MANIFOLDPROCESSOR_H_INC  ----- */

