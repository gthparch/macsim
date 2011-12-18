/*
 * =====================================================================================
 *
 *       Filename:  genericVcAllocator.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/21/2010 03:22:20 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha (), mitchelle.rasquinha@gatech.edu
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  _genericvcallocator_h_INC
#define  _genericvcallocator_h_INC

#include	"../../interfaces/genericHeader.h"
#include	"genericBuffer.h"
#include	"manifold/models/iris/iris_srcs/data_types/flit.h"
#include	<algorithm>


struct VCA_unit
{
    uint out_vc;
    uint out_port;
    uint in_port;
    uint in_vc;
};
typedef struct VCA_unit VCA_unit;

/*!
 * =====================================================================================
 *        Class:  GenericVcAllocator
 *  Description:  \brief This class allocates an output virtual channel to all
 *  requesting input messages. Only head flits pass through VCA.
 * =====================================================================================
 */
class GenericVcAllocator
{
    public:
        /* ====================  LIFECYCLE     ======================================= */
        GenericVcAllocator ();                             /* \fn constructor */
        ~GenericVcAllocator ();                            /* \fn destructor */
        void setup(uint p, uint v);                        /* \fn Init. Set vcs and ports */
        bool request(uint out_port, uint out_vc, uint in_port, uint in_vc);            /* \fn Request output port allocation.
                                                            \param in_port: Requesting in_port
                                                            \param in_vc: Requesting in_vc.*/
        bool is_empty( void );                                   /* \fn Checks if the vca unit has requests */
        bool is_empty( uint i );                                   /* \fn Checks if the vca unit has requests for the ith row ( i=out_port*ports+out_vc) */
        void pick_winner( std::vector <GenericBuffer> &in_buffers );
        void clear_winner( uint op, uint ovc, uint ip, uint ivc);                                   /* \fn Clear winner */
        std::string toString() const;        
        bool is_requested ( uint op, uint ovc, uint ip, uint ivc);
        int no_requestors( uint op);
        std::vector < std::vector< VCA_unit > > current_winners;
        uint address;
        uint node_ip;
        std::vector < std::vector <uint> >  last_winner;

    private:
        uint ports;
        uint vcs;
        std::string name;
        std::vector < std::vector < uint > > ovc_tokens;
        std::vector < std::vector < bool> > requestors;
        std::vector < std::vector < uint> > requestors_vc;
}; /* -----  end of class GenericVcAllocator  ----- */

#endif   /* ----- #ifndef _genericvcallocator_h_INC  ----- */

