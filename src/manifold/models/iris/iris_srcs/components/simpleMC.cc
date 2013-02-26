/*
 * =====================================================================================
 *
 *       Filename:  simpleMC.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/09/2011 03:35:53 PM
 *       Revision:  none
 *       Compiler:  g++
 *
 *         Author:  Mitchelle Rasquinha
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  SIMPLEMC_CC_INC
#define  SIMPLEMC_CC_INC

#include	"simpleMC.h"

SimpleMC::SimpleMC ()
{
}

SimpleMC::~SimpleMC ()
{
}

void
SimpleMC::parse_config(std::map<std::string,std::string>& p)
{
    ni_buffer_width = 2;
    no_nodes = 1;
    resp_payload_len = 512;
    max_mc_buffer_size = 10;

    std::map<std::string,std::string>::iterator it;
    it = p.find("int_buff_width");
    if ( it != p.end())
        ni_buffer_width = atoi((it->second).c_str());
    it = p.find("no_nodes");
    if ( it != p.end())
        no_nodes = atoi((it->second).c_str());
    it = p.find("resp_payload_len");
    if ( it != p.end())
        resp_payload_len = atoi((it->second).c_str());
    it = p.find("memory_latency");
    if ( it != p.end())
        memory_latency = atoi((it->second).c_str());
    it = p.find("max_mc_buffer_size");
    if ( it != p.end())
        max_mc_buffer_size = atoi((it->second).c_str());

    return;
}

void
SimpleMC::init()
{
    ni_buffers.resize( ni_buffer_width );
    ni_buffers.insert( ni_buffers.begin(), ni_buffers.size(), true);

    stat_packets_out = 0;
    stat_packets_in = 0;
    return;
}

void
SimpleMC::handle_new_packet_event ( int p, NetworkPacket* data )
{
    proc_recv = false;

    if ( mc_response_buffer.size() < max_mc_buffer_size)
    {
        NetworkPacket* curr_pkt;

        if ( buffered_pkt.size() > 0 )
        {
            curr_pkt = buffered_pkt.front();
            buffered_pkt.pop_front();
            if ( data != NULL)
                buffered_pkt.push_back(data);
        }
        else
            curr_pkt = data;

        if ( curr_pkt )
        {
            //    _DBG("************ MC got new packet ************** %d size:%d", curr_pkt->proc_buffer_id, mc_response_buffer.size());
            stat_packets_in++;

            handle_send_credit_event( (int) SEND_SIG, curr_pkt->proc_buffer_id);
            curr_pkt->sent_time = manifold::kernel::Manifold::NowTicks();
            mc_response_buffer.push_back(curr_pkt);
        }
    }
    else
    {
        NetworkPacket* temp = NULL;
        if ( data != NULL)
            buffered_pkt.push_back(data);
        // need to check here tht event is not called twice in a cycle
        /* not doing this for now.. will get invoked when a buffer empties
.. check issue function
           if ( !proc_recv )
           {
           proc_recv = false;
           manifold::kernel::Manifold::Schedule( 1 , &IrisTerminal::handle_new_packet_event,
           static_cast<IrisTerminal*>(this), (int)SEND_DATA, temp);
           }
         */
    }

    return ;
}

void
SimpleMC::handle_issue_pkt_event ( int port)
{
    bool is_buffer_empty = false;
    uint send_buffer_id = -1;
    for ( uint i=0; i<ni_buffer_width ; i++ )
        if ( ni_buffers[i] )
        {
            is_buffer_empty = true;
            send_buffer_id = i;
            break;
        }

    if ( mc_response_buffer.size()  && is_buffer_empty )
    {
        NetworkPacket* np = mc_response_buffer.front();
        if ( np && manifold::kernel::Manifold::NowTicks()>np->sent_time && (manifold::kernel::Manifold::NowTicks()- np->sent_time) >= memory_latency )
        {
            //        NetworkPacket* np = mc_response_buffer.front();
            mc_response_buffer.pop_front();
            np->dst_node = np->src_node;   // send it back to the node that requested
            np->src_node = this->GetComponentId();
            np->dst_component_id = 0;       // can use this for further splitting at the interface
            np->mclass = MC_RESP;
            np->proc_buffer_id = send_buffer_id;
            np->payload_length = resp_payload_len;

            ni_buffers[send_buffer_id] = false;

//            _DBG(" ******* MC sending new pkt  %d to:%d respbuffsize:%d buffready %lld", send_buffer_id, np->dst_node, mc_response_buffer.size(),stat_packets_out ); 
            manifold::kernel::Manifold::Schedule( 1 , &IrisInterface::handle_new_packet_event,
                                static_cast<IrisInterface*>(ni), (int)SEND_DATA, np);
            //        Send(SEND_DATA,np);

            stat_packets_out++;

            // restart incoming
            // need to check here tht event is not called twice in a cycle
            if ( !proc_recv )
            {
                NetworkPacket* tmp = NULL;
                proc_recv = true;
                manifold::kernel::Manifold::Schedule( 1 , &IrisTerminal::handle_new_packet_event,
                                                      static_cast<IrisTerminal*>(this), (int)SEND_DATA, tmp);
            }
        }
    }

    return ;
}

void
SimpleMC::handle_send_credit_event ( int port, uint64_t data )
{
    /* Send ack to interface */
    manifold::kernel::Manifold::Schedule( 1, &IrisInterface::handle_update_credit_event, 
                        static_cast<IrisInterface*>(ni), data);
    return ;
}

void
SimpleMC::handle_update_credit_event (int p, uint64_t data )
{
    assert( data < ni_buffer_width );
    ni_buffers[data] = true;
    return ;
}

void
SimpleMC::tick ( void )
{
    return ;
}

void
SimpleMC::tock ( void )
{
    handle_issue_pkt_event(SEND_DATA);
    return ;
}

std::string
SimpleMC::print_stats() const
{
    std::stringstream str;
    str << "\n SimpleMC[" << node_id << "] packets_out: " << stat_packets_out
        << "\n SimpleMC[" << node_id << "] packets_in: " << stat_packets_in
        << std::endl;
    return str.str();
} /* ----- end of function GenericPktGen::toString ----- */

#endif   /* ----- #ifndef SIMPLEMC_CC_INC  ----- */

