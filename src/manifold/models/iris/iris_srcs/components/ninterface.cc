#ifndef  INTERFACE_CC_INC
#define  INTERFACE_CC_INC

#include	"ninterface.h"

/* *********** Arbiter Functions ************ */
SimpleArbiter::SimpleArbiter()
{
}

SimpleArbiter::~SimpleArbiter()
{
}

void
SimpleArbiter::init ( void )
{
    requests.resize(no_channels);
    for ( uint i=0; i<no_channels; i++)
        requests[i] = false;

    return ;
}		/* -----  end of method SimpleArbiter::init  ----- */

bool
SimpleArbiter::is_requested ( uint ch )
{
//    assert( ch < requests.size() );
    return requests[ch];
}

bool
SimpleArbiter::is_empty ( void )
{
    for ( uint i=0 ; i<no_channels; i++ )
        if ( requests[i] )
            return false;

    return true;
}

void
SimpleArbiter::request ( uint ch )
{
    requests[ch] = true;
    return ;
}	

uint
SimpleArbiter::pick_winner ( void )
{
    for ( uint i=last_winner+1; i<requests.size() ; i++ )
        if ( requests[i] )
        {
            last_winner = i;
            requests[i] = false;
            return last_winner;
        }

    for ( uint i=0; i<=last_winner ; i++ )
        if ( requests[i] )
        {
            last_winner = i;
            requests[i] = false;
            return last_winner;
        }

    std::cout << " ERROR: Interface Arbiter was either empty or called and dint have a winner" << std::endl;
    exit (1);
}

/* *********** Network Interface Functions ************ */
NInterface::NInterface (macsim_c* simBase)
{
    m_simBase = simBase;
}

NInterface::~NInterface ()
{
    delete &router_in_buffer;
    delete &router_out_buffer;
}

void
NInterface::parse_config(std::map<std::string,std::string>& p)
{
    no_vcs = 4;
    credits = 1;
    link_width = 128;
    std::map<std::string,std::string>::iterator it;
    it = p.find("credits");
    if ( it != p.end())
        credits = atoi((it->second).c_str());
    it = p.find("no_vcs");
    if ( it != p.end())
        no_vcs = atoi((it->second).c_str());
    it = p.find("link_width");
    if ( it != p.end())
        link_width = atoi((it->second).c_str());

    return;
}

void
NInterface::init()
{

    //char tmp[30];
    //sprintf(tmp,"log_%d.txt",m_simBase->Mytid);
    //m_simBase->log_file = fopen (tmp,"a");


    router_in_buffer = *new GenericBuffer();
    router_out_buffer = *new GenericBuffer();
    router_in_buffer.resize ( no_vcs, 6*credits);
    router_out_buffer.resize ( no_vcs, 6*credits);
    proc_out_buffer.resize ( no_vcs );
    proc_in_buffer.resize(no_vcs);

    downstream_credits.resize( no_vcs );
    proc_credits.resize ( no_vcs );

    proc_ib_flit_index.resize ( no_vcs );
    proc_ob_flit_index.resize ( no_vcs );
    router_ob_packet_complete.resize(no_vcs);

    last_inpkt_winner = 0;

    for ( uint i=0; i<no_vcs; i++)
    {
        proc_ob_flit_index[i] = 0;
        proc_ib_flit_index[i] = 0;
        router_ob_packet_complete[i] = false;
        downstream_credits[i] = credits;
        proc_credits[i]=true;
    }

    arbiter = *new SimpleArbiter();
    arbiter.no_channels = no_vcs;
    arbiter.last_winner = 0;
    arbiter.init();

    /* Init stats */
    stat_packets_out = 0;
    stat_packets_in = 0;
}

void
NInterface::handle_link_arrival (int port, LinkData* data )
{
    switch ( data->type ) {
        case FLIT:
            {
#ifdef _DEBUG
                _DBG(" Int got flit vc%d ft%d fid:%d",data->vc, data->f->type, data->f->flit_id);
#endif
                if ( data->f->type == HEAD )
                    stat_packets_in++;

                router_in_buffer.change_push_channel(data->vc);
                router_in_buffer.push(data->f);

                if ( data->f->type != TAIL && data->f->pkt_length != 1)
                {
                    LinkData* ld =  new LinkData();
                    ld->type = CREDIT;
                    ld->src = this->GetComponentId();
                    ld->vc = data->vc;

                    Send(1,ld);
                    
                    //Use manifold send here to push the ld obj out
                    //manifold::kernel::Manifold::Schedule(1, &IrisRouter::handle_link_arrival, static_cast<IrisRouter*>(router), (uint)SEND_DATA, ld );
                }
                break;
            }

        case CREDIT:
            {
                downstream_credits[data->vc]++;
                break;
            }

        default:	
            std::cerr << "ERROR: NInterface::handle_link_arrival " << std::endl;
            exit(1);
            break;
    }	
    // delete ld.. if ld contains a flit it is not deleted. see destructor
    delete data;
    return;
}

void
NInterface::handle_new_packet_event (int port, NetworkPacket* data )
{
    //std::cout << "IRIS New packet at " << node_id << " procBufferID " << data->proc_buffer_id << " src: " << data->src_node << " dst: " << data->dst_node << "\n";
    /*  Convert the packet to flits and store in empty buffer */
//    assert ( data->proc_buffer_id < no_vcs);
    data->to_flit_level_packet( &proc_out_buffer[data->proc_buffer_id], link_width);
    proc_out_buffer[data->proc_buffer_id].virtual_channel = data->proc_buffer_id;
    proc_ob_flit_index[data->proc_buffer_id] = 0;

//    assert( proc_out_buffer[data->proc_buffer_id].size() != 0 );
    delete data;

    return ;
}

void
NInterface::handle_issue_pkt_event ( int port, uint64_t data )
{
    return ;
}

void
NInterface::handle_send_credit_event ( int port, uint64_t data )
{
    /* Send ack to terminal
     * */
    manifold::kernel::Manifold::Schedule( 1, &ManifoldProcessor::handle_update_credit_event, 
                                          static_cast<ManifoldProcessor*>(terminal),(int)SEND_SIG, data);
    return ;
}

void
NInterface::handle_update_credit_event ( int data )
{
    if ( data > no_vcs )
    {
        cerr << " ERROR incorrect ready at Interface" << endl;
        exit (1);
    }
    else
        proc_credits [ data ] = true;

    return ;
}

void
NInterface::tick ( void )
{
    return ;
}

void
NInterface::tock ( void )
{
    
    /* Handle events on the ROUTER side */
    // Move flits from the pkt buffer to the router_out_buffer
    for ( uint i=0 ; i<no_vcs ; i++ )
        if ( proc_out_buffer[i].size()>0 && proc_ob_flit_index[i] < proc_out_buffer[i].pkt_length )
        {
            router_out_buffer.change_push_channel(i);
            Flit* f = proc_out_buffer[i].get_next_flit();
            f->virtual_channel = i;
            router_out_buffer.push(f);
            proc_ob_flit_index[i]++;

            if ( proc_ob_flit_index[i] == proc_out_buffer[i].pkt_length )
            {
                proc_ob_flit_index[i] = 0;
                router_ob_packet_complete[i] = true;
            }
        }

    //Request the arbiter for all completed packets. Do not request if
    //downstream credits are empty.
    for ( uint i=0; i<no_vcs ; i++ )
        if( downstream_credits[i]>0 && router_out_buffer.get_occupancy(i)>0
            && router_ob_packet_complete[i] )
        {
            router_out_buffer.change_pull_channel(i);
            Flit* f= router_out_buffer.peek();
            if((f->type != HEAD)||( f->type == HEAD  && downstream_credits[i] == credits))
            {
              
                if ( !arbiter.is_requested(i) )
                    arbiter.request(i);
            }
        }

    // Pick a winner for the arbiter
    if ( !arbiter.is_empty() )
    {
        uint winner = arbiter.pick_winner();
//        assert ( winner < no_vcs );

        // send out a flit as credits were already checked earlier this cycle
        router_out_buffer.change_pull_channel(winner);
        Flit* f = router_out_buffer.pull();

        downstream_credits[winner]--;

        //if this is the last flit signal the terminal to update credit
        if ( f->type == TAIL || f->pkt_length == 1 )
        {
            if( f->type == TAIL )
                static_cast<TailFlit*>(f)->enter_network_time = manifold::kernel::Manifold::NowTicks();
            router_ob_packet_complete[winner] = false;
            handle_send_credit_event( (int) SEND_SIG, winner);
            
        }

        LinkData* ld =  new LinkData();
        ld->type = FLIT;
        ld->src = this->GetComponentId();
        ld->f = f;
        ld->vc = winner;

        Send(0, ld);

#ifdef _DEBUG
        _DBG(" SEND FLIT %d Flit is %s", winner, f->toString().c_str() );std::cout.flush();
#endif        
        //Use manifold send here to push the ld obj out
        //manifold::kernel::Manifold::Schedule(1, &IrisRouter::handle_link_arrival, static_cast<IrisRouter*>(router), (uint)SEND_DATA, ld );

        // Give every vc a fair chance to use the link
        //        arbiter.clear_winner();
    }

    /* Handle events on the PROCESSOR side */
    // Move completed packets to the terminal
    bool found = false;
    // send a new packet to the terminal.
    // In order to make sure it gets one packet in a cycle you check the
    // proc_recv flag
    if ( !static_cast<ManifoldProcessor*>(terminal)->proc_recv )
    {
        for ( uint i=last_inpkt_winner+1; i<no_vcs ; i++)
            if ( proc_credits[i] && proc_ib_flit_index[i]!=0 && 
                 proc_ib_flit_index[i] == proc_in_buffer[i].pkt_length)
            {
                last_inpkt_winner= i;
                found = true;
                break;
            }
        if ( !found )
            for ( uint i=0; i<=last_inpkt_winner; i++)
                if ( proc_credits[i] && proc_ib_flit_index[i]!=0 && 
                     proc_ib_flit_index[i] == proc_in_buffer[i].pkt_length)
                {
                    last_inpkt_winner= i;
                    found = true;
                    break;
                }

        if ( found )
        {
            proc_credits[last_inpkt_winner] = false;
            NetworkPacket* np = new NetworkPacket();
            np->from_flit_level_packet(&proc_in_buffer[last_inpkt_winner]);
            proc_ib_flit_index[last_inpkt_winner] = 0;
            static_cast<ManifoldProcessor*>(terminal)->proc_recv = true;

            //            _DBG(" Int sending pkt to proc %d", last_inpkt_winner); 
            manifold::kernel::Manifold::Schedule(1, &ManifoldProcessor::handle_new_packet_event, static_cast<ManifoldProcessor*>(terminal),(uint)SEND_DATA, np);

            /*  Send the tail credit back. All other flit credits were sent
             *  as soon as flit was got in link-arrival */
            LinkData* ld =  new LinkData();
            ld->type = CREDIT;
            ld->src = this->GetComponentId();
            ld->vc = last_inpkt_winner;

            //_DBG(" SEND CREDIT vc%d", ld->vc);
            Send(1,ld);
            //Use manifold send here to push the ld obj out
            //manifold::kernel::Manifold::Schedule(1, &IrisRouter::handle_link_arrival, static_cast<IrisRouter*>(router), (uint)SEND_DATA, ld );
        }
    }

    // push flits coming in to the in buffer
    for ( uint i=0; i<no_vcs ; i++ )
    {
        if( router_in_buffer.get_occupancy(i) > 0 && (proc_ib_flit_index.at(i) < (proc_in_buffer.at(i)).pkt_length || proc_ib_flit_index.at(i) == 0) )
        {
            router_in_buffer.change_pull_channel(i);
            Flit* ptr = router_in_buffer.pull();
            proc_in_buffer[i].add(ptr); 
            proc_ib_flit_index[i]++;
        }
    }

    return ;
}

#endif   /* ----- #ifndef INTERFACE_CC_INC  ----- */

