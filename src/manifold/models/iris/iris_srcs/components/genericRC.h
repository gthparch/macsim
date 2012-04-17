/*!
 * =====================================================================================
 *
 *       Filename:  genericrc.h
 *
 *    Description:  All routing algos are implemented here for the time being.
 *    Need to re arrange
 *
 *        Version:  1.0
 *        Created:  02/19/2010 11:54:57 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha (), mitchelle.rasquinha@gatech.edu
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */
#ifndef  _genericrc_h_INC
#define  _genericrc_h_INC

#include	"../../interfaces/genericHeader.h"
#include        "manifold/models/iris/iris_srcs/data_types/flit.h"
class HeadFlit;

class GenericRC
{
    public:
        GenericRC ();
        ~GenericRC(){}
        void push( Flit* f, uint vc );
        uint get_output_port ( uint channel);
        uint speculate_port ( Flit* f, uint ch );
        uint speculate_channel ( Flit* f, uint ch );
        uint get_virtual_channel ( uint ch );
        void resize ( uint ch );
        uint no_adaptive_ports( uint ch );
        uint no_adaptive_vcs( uint ch );
        bool is_empty();
        std::string toString() const;
        void init(void);
        
        uint node_id;
        uint address;
        uint no_nodes;
        uint grid_size;
        ROUTING_SCHEME rc_method;
        std::vector < uint > grid_xloc;
        std::vector < uint > grid_yloc;

    protected:

    private:
        std::string name;
        uint vcs;
        uint route_x_y( uint addr );
        void route_x_y_hetero(  HeadFlit* hf );
        void route_twonode( HeadFlit* hf );
        void route_torus( HeadFlit* hf );
        void route_ring( HeadFlit* hf );
        void route_ring_uni( HeadFlit* hf );

        std::vector < uint > possible_out_ports;
        std::vector < uint > possible_out_vcs;

        bool do_request_reply_network;
        /*
         * =====================================================================================
         *        Class:  Address
         *  Description:  
         * =====================================================================================
         */
        class Address
        {
            public:
                bool route_valid;
                unsigned int channel;
                unsigned int out_port;
                uint last_adaptive_port;
                uint last_vc;
                std::vector < uint > possible_out_ports;
                std::vector < uint > possible_out_vcs;

            protected:

            private:

        }; /* -----  end of class Address  ----- */
        std::vector<Address> addresses;

}; /* -----  end of class GenericRC  ----- */

#endif   /* ----- #ifndef _genericrc_h_INC  ----- */

