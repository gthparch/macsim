/*!
 * =====================================================================================
 *
 *       Filename:  genericbuffer.h
 *
 *    Description:  implements a buffer component. Can support multiple virtual
 *    channels. Used by the router and network interface
 *
 *        Version:  1.0
 *        Created:  02/20/2010 11:48:27 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Mitchelle Rasquinha (), mitchelle.rasquinha@gatech.edu
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  _genericbuffer_h_INC
#define  _genericbuffer_h_INC

#include	"../data_types/flit.h"
#include        "../../interfaces/genericHeader.h"
#include	<deque>
#include	<vector>
#include	<assert.h>

class GenericBuffer
{
    public:
        GenericBuffer ();                             /* constructor */
        ~GenericBuffer ();                             /* desstructor */
        void push( Flit* f );
        Flit* pull();
        Flit* peek();
        uint get_occupancy( uint ch ) const;
        void resize ( uint vcs, uint buffer_size );
        void change_pull_channel( uint ch );
        void change_push_channel( uint ch );
        uint get_pull_channel() const;
        uint get_push_channel() const;
        bool is_channel_full( uint ch ) const;
        bool is_empty( uint ch ) const;

        std::string toString() const;

        std::vector < std::deque<Flit*> > buffers;
        std::vector < int > next_port;

    protected:

    private:
        uint vcs;
        uint buffer_size;
        uint pull_channel;
        uint push_channel;

}; /* -----  end of class GenericBuffer  ----- */

#endif   /* ----- #ifndef _genericbuffer_h_INC  ----- */

