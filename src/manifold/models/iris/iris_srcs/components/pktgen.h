/*
 * =====================================================================================
 *
 *       Filename:  pktgen.h
 *
 *    Description:  This file contains the description for class
 *    PktGen. It models a simple stochastic packet generator that 
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

#ifndef  PKTGEN_H_INC
#define  PKTGEN_H_INC

#include	"../../interfaces/irisTerminal.h"
#include	"../../interfaces/irisInterface.h"
#include	"kernel/clock.h"
#include	"kernel/manifold.h"
#include	"../data_types/networkPacket.h"
#include        <gsl/gsl_rng.h>
#include        <gsl/gsl_randist.h>

//extern class Clock* master_clock;
class PktGen: public IrisTerminal
{
    public:
        /* ====================  LIFECYCLE  ======================================= */
        PktGen ();
        //PktGen (uint nid):IrisTerminal(nid){}
        PktGen (uint nid, uint ni_buf_wid, uint nnodes, uint m_irt):
	    IrisTerminal(nid), ni_buffer_width(ni_buf_wid), no_nodes(nnodes), mean_irt(m_irt),
	    dst_distribution_type(SIMPLE) {}
        ~PktGen ();

        /* ====================  Event handlers  ======================================= */
        void handle_issue_pkt_event( int inputid ); 
        void handle_send_credit_event( int inputid, uint64_t data ); 
        void handle_new_packet_event( int inputid, NetworkPacket* data);
        void handle_update_credit_event( int inputid, uint64_t data);

        /* ====================  Clocked funtions  ======================================= */
        void tick (void);
        void tock (void);

        /* ====================  Setup functions  ======================================= */
        void init ( void );
        void parse_config(std::map<std::string,std::string>& params);
        std::string print_stats( void ) const;

    protected:

    private:
        uint ni_buffer_width;
        uint no_nodes;
        uint irt;
        uint64_t sent_time;
        uint no_mcs;
        uint mean_irt;
        DEST_DISTRIBUTION_TYPE dst_distribution_type;
        std::vector<int> mc_positions;

        std::vector<bool> ni_buffers;        /* Credits for the buffers in the interface */

        /* Stat variables */
        uint64_t stat_packets_in;
        uint64_t stat_packets_out;
        uint64_t stat_last_packet_out_cycle;
        uint64_t stat_last_packet_in_cycle;
        uint64_t stat_total_latency;

        /* Statistical generators for destination,addr and irt */
        const gsl_rng_type *T;
        gsl_rng* irt_gen;
        gsl_rng* dst_gen;
        gsl_rng* pkt_len_gen;
        gsl_rng* addr_gen;

        uint get_bit_rev_dest(uint d);
}; /* -----  end of class PktGen  ----- */

#endif   /* ----- #ifndef PKTGEN_H_INC  ----- */

