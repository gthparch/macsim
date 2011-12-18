/*
 * =====================================================================================
 *
 *       Filename:  flit.h
 *
 *    Description:  Classes for network packets
 *
 *        Version:  1.0
 *        Created:  02/12/2011 04:53:48 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Mitchelle Rasquinha
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */
#ifndef  FLIT_H_INC
#define  FLIT_H_INC
#include	"memreq_info.h"
#include	"../../interfaces/genericHeader.h"
#include	<deque>

using namespace std;

enum flit_type {UNK, HEAD, BODY, TAIL };

/*
 * =====================================================================================
 *        Class:  Phit
 *  Description: Subdividing a phit from a flit. But for now 1Phit = 1Flit 
 * =====================================================================================
 */

class Phit
{
    public:
        Phit();
        ~Phit();
        flit_type ft;

    protected:

    private:

};

/*
 * =====================================================================================
 *        Class:  Flit
 *  Description:  This object defines a single flow control unit. 
 *  FROM TEXT: Flow control is a synchronization protocol for transmitting and
 *  recieving a unit of information. This unit of flow control refers to that
 *  portion of the message whoose transfer must be syncronized. A packet is
 *  divided into flits and flow control between routers is at the flit level. It
 *  can be multicycle depending on the physical channel width.
 * =====================================================================================
 */
class Flit
{
    public:
        Flit ();
        ~Flit ();

        flit_type type;
        uint virtual_channel;
        uint no_phits;
        uint pkt_length;
        string toString() const;

#ifdef _DEBUG_IRIS
        uint flit_id;
#endif

    protected:

    private:

}; /* -----  end of class Flit  ----- */

/*
 * =====================================================================================
 *        Class:  HeadFlit
 *  Description:  All pkt stat variables are passed within the head flit for
 *  simulation purposes. Not passing it in tail as some pkt types may not need
 *  more than one flit. 
 * =====================================================================================
 */

class HeadFlit: public Flit
{
    public:
        HeadFlit ();  
        ~HeadFlit ();  
        uint src_node;
        uint dst_node;
        uint dst_component_id;
        u_int64_t address;
        message_class mclass;

        /* stats */
        u_int64_t packet_originated_time;

        void populate_head_flit(void);
        string toString() const;

	mem_req_s *req;

    protected:

    private:

}; /* -----  end of class HeadFlit  ----- */

/*
 * =====================================================================================
 *        Class:  BodyFlit
 *  Description:  flits of type body 
 * =====================================================================================
 */
class BodyFlit : public Flit
{
    public:
        BodyFlit (); 
        ~BodyFlit (); 

        string toString() const;
        void populate_body_flit();

    protected:

    private:

}; /* -----  end of class BodyFlit  ----- */

/*
 * =====================================================================================
 *        Class:  TailFlit
 *  Description:  flits of type TAIL. Has the pkt sent time. 
 * =====================================================================================
 */
class TailFlit : public Flit
{
    public:
        TailFlit ();
        ~TailFlit ();

        void populate_tail_flit();
        u_int64_t sent_time;
        u_int64_t enter_network_time;
        string toString() const;

    protected:

    private:

}; /* -----  end of class TailFlit  ----- */

/*
 * =====================================================================================
 *        Class:  FlitLevelPacket
 *  Description:  This is the desription of the lower link layer protocol
 *    class. It translates from the higher message protocol definition of the
 *    packet to flits. Phits are further handled for the physical layer within
 *    this definition. 
 * =====================================================================================
 */
class FlitLevelPacket
{
    public:
        FlitLevelPacket ();
        ~FlitLevelPacket ();

        uint src_node;
        uint dst_node;
        uint dst_component_id;
        u_int64_t address;
        message_class mc;
        uint pkt_length;
        uint virtual_channel;
        uint64_t enter_network_time;

        void add ( Flit* ptr);  /* Adds an incoming flit to the pkt. */
        Flit* get_next_flit();  /* This will pop the flit from the queue as well. */
        uint size();            /* Return the length of the pkt in flits. */
        bool valid_packet();    /* Checks the length of the pkt against the flits vector size. */
        string toString () const;

	mem_req_s *req;
        deque<Flit*> flits;

    protected:

    private:

}; /* -----  end of class FlitLevelPacket  ----- */

#endif   /* ----- #ifndef FLIT_H_INC  ----- */

