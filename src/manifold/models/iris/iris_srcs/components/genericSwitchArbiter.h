/*
 * =====================================================================================
 *
 *       Filename:  myFullyVirtualArbiter.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  04/27/2010 01:52:11 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha (), mitchelle.rasquinha@gatech.edu
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  _GENERICSWARBITER_h_INC
#define  _GENERICSWARBITER_h_INC

#include	"../../interfaces/genericHeader.h"
#include	<vector>
#include	<fstream>


class SA_unit
{
    public:
        SA_unit(){};
        uint port;
        uint ch;
        uint64_t in_time;
        uint64_t win_cycle;
};

class GenericSwitchArbiter
{
    public:
        GenericSwitchArbiter ();       
        ~GenericSwitchArbiter();
        void resize(uint p, uint v);
        bool is_requested(uint outp, uint inp, uint ovc);
        void clear_requestor(uint outp, uint inp, uint ovc);
        void request(uint p, uint op, uint inp, uint iv);
        SA_unit pick_winner( uint p);
        SA_unit do_round_robin_arbitration( uint p);
        SA_unit do_priority_round_robin_arbitration( uint p);
        SA_unit do_fcfs_arbitration( uint p);
        void request(uint oport, uint inport, message_class m);
        bool is_empty();
        std::string toString() const;
        uint address;
        std::string name;
        uint node_ip;

    protected:

    private:
        uint ports;
        uint vcs;
        std::vector < std::vector <bool> > requested;
        std::vector < std::vector <bool> > priority_reqs;
        std::vector < std::vector<SA_unit> > requesting_inputs;
        std::vector < SA_unit > last_winner;
        std::vector < uint> last_port_winner;

};

#endif 
