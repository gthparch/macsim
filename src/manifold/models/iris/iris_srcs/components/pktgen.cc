/*
 * =====================================================================================
 *
 *       Filename:  pktgen.cc
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  02/08/2011 10:41:23 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Mitchelle Rasquinha
 *        Company:  Georgia Institute of Technology
 *
 * =====================================================================================
 */

#ifndef  PKTGEN_CC_INC
#define  PKTGEN_CC_INC

#include	"pktgen.h"

PktGen::PktGen ()
{
}

PktGen::~PktGen ()
{
    gsl_rng_free(dst_gen);
    gsl_rng_free(irt_gen);
    gsl_rng_free(pkt_len_gen);
}

void
PktGen::parse_config(std::map<std::string,std::string>& p)
{
    // default
    ni_buffer_width = 2;
    no_nodes = 1;
    mean_irt = 50;
    dst_distribution_type = SIMPLE;

    // find MC_loc in p map and make a local copy
    std::map<std::string,std::string>::iterator it2;
    for ( it2 = p.begin(); it2 != p.end(); it2++)
    {
        string key = it2->first;
        string sim_string = key.substr(0,key.find(":"));
        if ( sim_string.compare("mc_loc") == 0 )
            mc_positions.push_back(atoi((it2->second).c_str()));
    }

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
    it = p.find("mean_irt");
    if ( it != p.end())
        mean_irt = atoi((it->second).c_str());
    it = p.find("dst_distrib");
    if ( it != p.end())
    {
        if ( !it->second.compare("HALF"))
            dst_distribution_type = HALF;
        if ( !it->second.compare("BIT_REVERSAL"))
            dst_distribution_type = BIT_REVERSAL;
        if ( !it->second.compare("USE_MC"))
            dst_distribution_type = USE_MC;
    }

    return;
}

void
PktGen::init()
{

    if ( dst_distribution_type == USE_MC )
    {
        assert ( no_mcs != 0);
        assert ( mc_positions.size() == no_mcs);
    }

    irt = 1;
    sent_time = 0;

    stat_packets_in = 0;
    stat_packets_out = 0;
    stat_last_packet_out_cycle = 0;
    stat_last_packet_in_cycle = 0;
    stat_total_latency = 0;

    gsl_rng_env_setup();
    gsl_rng_default_seed = 2^25;
    T = gsl_rng_default;

    dst_gen = gsl_rng_alloc (T);
    irt_gen = gsl_rng_alloc (T);
    pkt_len_gen = gsl_rng_alloc (T);
    addr_gen = gsl_rng_alloc (T);

    irt =  gsl_ran_gaussian_tail(irt_gen ,0,1000);
    irt = node_id;

    /* Can either set them to true or have the interface send you events */
    ni_buffers.resize(ni_buffer_width);
    ni_buffers.insert( ni_buffers.begin(), ni_buffers.size(), true);

    return;
}

void
PktGen::handle_new_packet_event (int port, NetworkPacket* data)
{

    proc_recv = false;
    /* Can this function be called twice in a cycle??.. make sure that does not
     * happen*/

    stat_packets_in++;
    stat_last_packet_in_cycle = manifold::kernel::Manifold::NowTicks();

    uint lat = (manifold::kernel::Manifold::NowTicks() - data->enter_network_time); //(sent_time part of pkt)
    stat_total_latency += lat;
    //    _DBG(" ********* PKTGEN got new pkt ********* vc %d %d",data->proc_buffer_id, lat);

    //manifold::kernel::Manifold::Schedule( 0, &PktGen::handle_send_credit_event, this,(int)SEND_SIG, data->proc_buffer_id);
    handle_send_credit_event((int) SEND_SIG, data->proc_buffer_id);

    delete data;
    return;
}

std::string
PktGen::print_stats( void ) const
{
    std::stringstream str;
    str << "\n PktGen[" << node_id << "] packets_out: " << stat_packets_out
        << "\n PktGen[" << node_id << "] packets_in: " << stat_packets_in
        << "\n PktGen[" << node_id << "] avg_latency: " << (stat_total_latency+0.0)/stat_packets_in
        << "\n PktGen[" << node_id << "] stat_last_packet_out_cycle: " << stat_last_packet_out_cycle
        << "\n PktGen[" << node_id << "] stat_last_packet_in_cycle: " << stat_last_packet_in_cycle
        << std::endl;
    return str.str();
}

uint 
PktGen::get_bit_rev_dest(uint b)
{
    unsigned char tmp = 0;
    // reverses a byte long number
    // from http://graphics.stanford.edu/~seander/bithacks.html
    tmp = ((b * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
    return tmp%no_nodes;

}

void
PktGen::handle_issue_pkt_event(int port)
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

    if ( is_buffer_empty && manifold::kernel::Manifold::NowTicks() > sent_time && (manifold::kernel::Manifold::NowTicks()-sent_time) > irt )
    {
        stat_packets_out++;
        stat_last_packet_out_cycle = manifold::kernel::Manifold::NowTicks();

        NetworkPacket* np = new NetworkPacket();
        np->src_node = node_id; //this->GetComponentId();
        //uint tmp = (node_id + no_nodes/2 -1)%no_nodes;
        uint tmp = -1;
        switch ( dst_distribution_type )
        {
            case BIT_REVERSAL:
                tmp = get_bit_rev_dest(node_id); 
                break;
            case HALF:
                tmp = (no_nodes-2+node_id)%no_nodes;   
                break;
            case USE_MC:
                tmp = mc_positions.at(gsl_rng_get(dst_gen)%no_mcs);   
                break;
            default:
                tmp = (node_id + 2)%no_nodes;
                break;
        }
        assert ( tmp < no_nodes );
        np->dst_node = tmp == node_id ? (tmp++)%no_nodes : tmp;
        np->dst_component_id = 0;       // can use this for further splitting at the interface
        np->address = 0xff; // gsl_ran_gaussian_tail( addr_gen ,0,2);
        np->mclass = PROC_REQ;
        np->proc_buffer_id = send_buffer_id;
        np->payload_length = 0;
        np->enter_network_time = manifold::kernel::Manifold::NowTicks();


        ni_buffers[send_buffer_id] = false;
        sent_time = manifold::kernel::Manifold::NowTicks();
        irt =  gsl_ran_gaussian_tail(irt_gen ,0,mean_irt);

#ifdef _DEBUG
        _DBG("pktgen pkt out: vc%d dst:%d src:%d", send_buffer_id, np->dst_node, np->src_node);
#endif

        manifold::kernel::Manifold::Schedule( 1 , &IrisInterface::handle_new_packet_event,
                                              static_cast<IrisInterface*>(ni), (int)SEND_DATA, np);
        //        Send(SEND_DATA,np);

    }

    return;
}

void
PktGen::handle_send_credit_event(int port, uint64_t data)
{
    /* Send ack to interface */
    assert( ni != NULL );
    manifold::kernel::Manifold::Schedule( 1, &IrisInterface::handle_update_credit_event, 
                                          static_cast<IrisInterface*>(ni), data);

    return;
}

void
PktGen::handle_update_credit_event(int p, uint64_t credit)
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
PktGen::tock()
{
    handle_issue_pkt_event(SEND_DATA);
    return;
}

void
PktGen::tick ( void )
{
    return ;
}

#endif   /* ----- #ifndef PKTGEN_CC_INC  ----- */

