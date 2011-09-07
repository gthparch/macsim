/*
 * =====================================================================================
 *
 *       Filename:  simpleMC.h
 *
 *    Description:  This models a simple MC model that accepts a request packet
 *    and sends out a response packet with a fixed latency of DRAM_MEM_LATENCY
 *
 *        Version:  1.0
 *        Created:  02/09/2011 03:36:04 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Mitchelle Rasquinha
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  SIMPLEMC_H_INC
#define  SIMPLEMC_H_INC

#include	"../../interfaces/irisTerminal.h"
#include	"../data_types/networkPacket.h"
#include	<deque>

#define DRAM_MEM_LATENCY 100

class SimpleMC: public IrisTerminal
{
    public:
        /* ====================  LIFECYCLE     ======================================= */
        SimpleMC (); 
        SimpleMC (uint nid):IrisTerminal(nid){}; 
        ~SimpleMC (); 

        /* ====================  Event handlers     ======================================= */
        void handle_issue_pkt_event( int inputid); 
        void handle_send_credit_event( int inputid, uint64_t data); 
        void handle_new_packet_event(int inputid, NetworkPacket* data); 
        void handle_update_credit_event( int p, uint64_t data);

        /* ====================  Clocked funtions  ======================================= */
        void tick (void);
        void tock (void);

        void init (void);
        void parse_config(std::map<std::string,std::string>& params);
        std::string print_stats() const;

    protected:

    private:
        uint ni_buffer_width;
        uint no_nodes;
        std::vector<bool> ni_buffers;        /* Credits for the buffers in the interface */
        uint resp_payload_len;
        uint memory_latency;
        uint max_mc_buffer_size;

        std::deque<NetworkPacket*> mc_response_buffer;
        std::deque<NetworkPacket*> buffered_pkt;

        // stats
        uint64_t stat_packets_in;
        uint64_t stat_packets_out;

}; /* -----  end of class SimpleMC  ----- */

#endif   /* ----- #ifndef SIMPLEMC_H_INC  ----- */

