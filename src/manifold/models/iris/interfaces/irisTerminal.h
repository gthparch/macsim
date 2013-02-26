/*
 * =====================================================================================
 *
 *       Filename:  IrisTerminal.h
 *
 *    Description:  Implements the class IrisTerminal
 *
 *        Version:  1.0
 *        Created:  02/04/2011 02:26:02 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  IRISTERMINAL_H_INC
#define  IRISTERMINAL_H_INC

#include "genericHeader.h"
#include "irisInterface.h"
#include "kernel/component.h"
#include "../iris_srcs/data_types/networkPacket.h"

class NetworkPacket;

using namespace std;
/*
 * =====================================================================================
 *        Class:  IrisTerminal
 *  Description:  Abstract base classes for all processor type components for
 *  interfacing into manifold. 
 * =====================================================================================
 */
class IrisTerminal:public manifold::kernel::Component
{
    public:

        IrisTerminal():proc_recv(false){}

        /* ==================== Event handler for  the interface to the network ======================================= */
        virtual void handle_new_packet_event( int inputid, NetworkPacket* data) = 0; 
        virtual void handle_issue_pkt_event( int inputid) = 0; 
        virtual void handle_send_credit_event( int inputid, uint64_t data) = 0; 
        virtual void handle_update_credit_event( int inputid, uint64_t data) = 0; 

        /* ====================  Clocked funtions ======================================= */
        virtual void tick (void) = 0;
        virtual void tock (void) = 0;

        virtual void init (void) = 0;
        virtual void parse_config(map<string,string>& ) = 0;
        virtual string print_stats() const = 0;
        /* variables */
        /*  Used enum definitions from genericHeader for port descriptions */
        uint node_id;
        bool proc_recv;

        Component* ni;

    protected:

    private:

}; /* -----  end of class IrisTerminal  ----- */

#endif   /* ----- #ifndef IRISTERMINAL_H_INC  ----- */
