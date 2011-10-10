/*
 * =====================================================================================
 *
 *       Filename:  networkPacket.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/09/2011 10:19:12 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Mitchelle Rasquinha
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  NETWORKPACKET_H_INC
#define  NETWORKPACKET_H_INC

#include	"../../interfaces/genericHeader.h"
#include	"flit.h"
#include	<math.h>
#ifdef _DEBUG
#include	"../../interfaces/MersenneTwister.h"
#include	"memreq_info.h"
#endif

//class FlitLevelPacket;

using namespace std;
#ifdef irisinterfacenetworkpacket
#error
#endif
class NetworkPacket{
    public:
        NetworkPacket();
        ~NetworkPacket();

	uint src_node;
	uint dst_node;
	uint dst_component_id;
        u_int64_t address;
        message_class mclass;
        uint proc_buffer_id;
        uint pkt_length;
        uint payload_length;    // used to compute no of flits in the flit level packet
        u_int64_t sent_time;
        u_int64_t enter_network_time;

	mem_req_s * req;

	static int Serialize(NetworkPacket p, unsigned char* buf);
	static NetworkPacket Deserialize(const unsigned char* data, int len);

	bool operator==(const NetworkPacket& p);
        string toString ( ) const;

        void to_flit_level_packet(FlitLevelPacket* flp, uint link_width);
        void from_flit_level_packet(FlitLevelPacket* flp);
};
#endif   /* ----- #ifndef NETWORKPACKET_H_INC  ----- */

