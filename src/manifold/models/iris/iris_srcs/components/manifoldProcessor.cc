/*
 * =====================================================================================
 *
 *       Filename:  manifoldProcessor.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/16/2011 01:18:00 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Si Li
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  MANIFOLDPROCESSOR_CC_INC
#define  MANIFOLDPROCESSOR_CC_INC

#include	"manifoldProcessor.h"

ManifoldProcessor::ManifoldProcessor (macsim_c* simBase)
{
    m_simBase = simBase; //
}

ManifoldProcessor::~ManifoldProcessor ()
{
}

void
ManifoldProcessor::parse_config( std::map<std::string,std::string> & p)
{
    // default
    ni_buffer_width = 2;
    no_nodes = 1;
#if 1
    std::map<std::string,std::string>::iterator it;
    it = p.find("int_buff_width");
    if ( it != p.end())
        ni_buffer_width = atoi((it->second).c_str());
    it = p.find("no_nodes");
    if ( it != p.end())
        no_nodes = atoi((it->second).c_str());
    it = p.find("no_mcs");
    if ( it != p.end())
        no_mcs = atoi((it->second).c_str());
#endif
    return;
}

void
ManifoldProcessor::init()
{

    char tmp[30];
    sprintf(tmp,"log_%d.txt", m_simBase->Mytid);
    m_simBase->log_file = fopen (tmp, "a");

    sent_time = 0;

    stat_packets_in = 0;
    stat_packets_out = 0;
    stat_last_packet_out_cycle = 0;
    stat_last_packet_in_cycle = 0;
    stat_total_latency = 0;

    /* Can either set them to true or have the interface send you events */
    ni_buffers.resize(ni_buffer_width);
    ni_buffers.insert( ni_buffers.begin(), ni_buffers.size(), true);

    return;
}

void
ManifoldProcessor::handle_issue_pkt_event ( int nothing)
{
}

void
ManifoldProcessor::handle_issue_pkt_event ( int nothing, uint64_t data)
{
}

void
ManifoldProcessor::handle_new_packet_event (int port, NetworkPacket* data)
{

    proc_recv = false;

    stat_packets_in++;
    stat_last_packet_in_cycle = manifold::kernel::Manifold::NowTicks();

    uint lat = (manifold::kernel::Manifold::NowTicks() - data->enter_network_time); //(sent_time part of pkt)
    stat_total_latency += lat;
    _DBG(" ********* MANIFOLDPROCESSOR got new pkt ********* vc %d %d",data->proc_buffer_id, lat);
if(manifold::kernel::Manifold::NowTicks() == 135)
    cout << "manifold proc got its first packet back!" << endl;
    
    //manifold::kernel::Manifold::Schedule( 0, &ManifoldProcessor::handle_send_credit_event, this,(int)SEND_SIG, data->proc_buffer_id);
    handle_send_credit_event((int) SEND_SIG, data->proc_buffer_id);

    data->req->m_state = MEM_NOC_DONE;
    receive_queue.push(data->req);
    delete data;
    return;
}

std::string
ManifoldProcessor::print_stats( void ) const
{
    std::stringstream str;
    str << "\n ManifoldProcessor[" << node_id << "] packets_out: " << stat_packets_out
        << "\n ManifoldProcessor[" << node_id << "] packets_in: " << stat_packets_in
        << "\n ManifoldProcessor[" << node_id << "] avg_latency: " << (stat_total_latency+0.0)/stat_packets_in
        << "\n ManifoldProcessor[" << node_id << "] stat_last_packet_out_cycle: " << stat_last_packet_out_cycle
        << "\n ManifoldProcessor[" << node_id << "] stat_last_packet_in_cycle: " << stat_last_packet_in_cycle
        << std::endl;
    return str.str();
}

//only works for 8 bits
uint 
ManifoldProcessor::get_bit_rev_dest(uint b)
{
    unsigned char tmp = 0;
    // reverses a byte long number
    // from http://graphics.stanford.edu/~seander/bithacks.html
    tmp = ((b * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
    return tmp%no_nodes;

}

bool
ManifoldProcessor::send_packet(mem_req_s *req)
{
    bool is_buffer_empty = false;
    uint send_buffer_id = -1;
    
    uint i = (req->m_noc_type == PROC_REQ) ? 0 : ni_buffer_width/2;
    for ( uint count=0; count < ni_buffer_width/2 ; count++,i++ )
    {
        if ( ni_buffers[i] )
        {
            is_buffer_empty = true;
            send_buffer_id = i;
            break;
        }
    }
	
    if ( is_buffer_empty )
    {
    
        stat_packets_out++;
        stat_last_packet_out_cycle = manifold::kernel::Manifold::NowTicks();

        NetworkPacket* np = new NetworkPacket();
        np->src_node = node_id; 
        np->dst_node = req->m_msg_dst;
		    np->req = req;

        np->dst_component_id = 3;       // can use this for further splitting at the interface
        np->address = req->m_addr; 
        np->mclass = (message_class)req->m_noc_type;	
        np->proc_buffer_id = send_buffer_id;
        np->payload_length = 128;//req->m_size*8; FIXME: use m_size when macsim is updated
        np->enter_network_time = manifold::kernel::Manifold::NowTicks();

        ni_buffers[send_buffer_id] = false;
        sent_time = manifold::kernel::Manifold::NowTicks();

//        _DBG("pktgen pkt out: %d nextattempt %lld ", send_buffer_id, irt);

        manifold::kernel::Manifold::Schedule( 1 , &IrisInterface::handle_new_packet_event,
                                              static_cast<IrisInterface*>(ni), (int)SEND_DATA, np);
        //        Send(SEND_DATA,np);
        return true;
    } else
    	return false;
}

void
ManifoldProcessor::handle_send_credit_event(int port, uint64_t data)
{
    /* Send ack to interface */
    assert( ni != NULL );
    manifold::kernel::Manifold::Schedule( 1, &IrisInterface::handle_update_credit_event, 
                                          static_cast<IrisInterface*>(ni), data);

    return;
}

void
ManifoldProcessor::handle_update_credit_event(int p, uint64_t credit)
{
    assert( credit < ni_buffer_width );
    ni_buffers[credit] = true;

    /* 
    printf("\nT:%lld N:%d got credit for vc %d credits:",manifold::kernel::Manifold::NowTicks(), node_id, credit);
    for ( uint i=0; i<ni_buffer_width; i++)
    {
        if ( ni_buffers[i] )
            printf(" 1");
        else
            printf(" 0");
    }
     * */
    return;
}

void
ManifoldProcessor::tock()
{
    //handle_issue_pkt_event(SEND_DATA);
    return;
}

void
ManifoldProcessor::tick ( void )
{
    return ;
}

mem_req_s* 
ManifoldProcessor::check_queue()
{
    if(receive_queue.empty() )
	return NULL;
    
    mem_req_s *ret = receive_queue.front();
    receive_queue.pop();

    return ret;
}

#endif   /* ----- #ifndef MANIFOLDPROCESSOR_CC_INC  ----- */

