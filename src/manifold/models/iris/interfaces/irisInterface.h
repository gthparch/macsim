/*
 * =====================================================================================
 *
 *       Filename:  IrisInterface.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/04/2011 02:26:28 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  IRISINTERFACE_H_INC
#define  IRISINTERFACE_H_INC

//#define irisinterfacenetworkpacket
#include        "genericHeader.h"
#include        "kernel/component.h"
#include        "../iris_srcs/data_types/networkPacket.h"
#include        "../iris_srcs/data_types/linkData.h"
//#include	"manifoldTile.h"
class NetworkPacket;


/*
 * =====================================================================================
 *        Class:  IrisInterface
 *  Description:  
 * =====================================================================================
 */
class IrisInterface: public manifold::kernel::Component 
{
 public:
        IrisInterface(){}
        IrisInterface(uint nid ):node_id(nid){}

        /* ====================  Event handlers at the processor-NI interface    ======================================= */
        virtual void handle_issue_pkt_event( int inputId, uint64_t data ) = 0;
        virtual void handle_send_credit_event( int inputId, uint64_t data ) = 0;
        virtual void handle_update_credit_event( int inputId, uint64_t data ) = 0;
        /* Using schedule */
        virtual void handle_link_arrival ( int inputId, LinkData* data ) = 0;
        virtual void handle_new_packet_event( int inputId, NetworkPacket* data ) = 0;
        virtual void handle_update_credit_event( int data ) = 0;

        /* ====================  Event handlers at the interface-router interface    ======================================= */

        /* ====================  Clocked funtions ======================================= */
        virtual void tick (void) = 0;
        virtual void tock (void) = 0;

        virtual void init (void) = 0;
        virtual void parse_config(map<string,string>&) = 0;

        /* ====================== variables  */
        /*  Used enum definitions from genericHeader for port descriptions */
        uint node_id;
        manifold::kernel::Component* terminal;

    protected:

    private:



}; /* -----  end of class IrisInterface  ----- */

#endif   /* ----- #ifndef IRISINTERFACE_H_INC  ----- */
