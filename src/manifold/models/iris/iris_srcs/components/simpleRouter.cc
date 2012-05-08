#ifndef  SIMPLEROUTER_CC_INC
#define  SIMPLEROUTER_CC_INC

#include	"simpleRouter.h"

extern "C" {
#include "SIM_router.h"
#include "SIM_router_power.h"
#include "SIM_parameter.h"
#include "SIM_array.h"
#include "SIM_misc.h"
#include "SIM_static.h"
#include "SIM_clock.h"
#include "SIM_util.h"
#include "SIM_link.h"
}

#define NETVIS 0
bool printFlag = false;


// memory request state noc string
const char* mem_req_noc_type_name[MAX_NOC_STATE] = {
  "NOC_FILL",
  "NOC_NEW",
  "NOC_LAST",
};


SimpleRouter::SimpleRouter ( macsim_c* simBase )
{
    m_simBase = simBase;
}

SimpleRouter::~SimpleRouter ()
{
	for ( uint i=0; i<ports; i++)
    {
        delete &in_buffers[i];
        delete &decoders[i];
        downstream_credits[i].clear();
        delete &vca;
        delete &swa;
    }
    input_buffer_state.clear();
}

void
SimpleRouter::parse_config(map<string,string>& p)
{
    ports = 3;
    vcs = 4;
    credits = 4;
    rc_method = RING_ROUTING;
    no_nodes = 16;
    grid_size = no_nodes;

    
    map<string,string>::iterator it;
    it = p.find("no_nodes");
    if ( it != p.end())
        no_nodes = atoi((it->second).c_str());
    it = p.find("grid_size");
    if ( it != p.end())
        grid_size= atoi((it->second).c_str());
    it = p.find("no_ports");
    if ( it != p.end())
        ports = atoi((it->second).c_str());
    it = p.find("credits");
    if ( it != p.end())
        credits = atoi((it->second).c_str());
    it = p.find("no_vcs");
    if ( it != p.end())
        vcs = atoi((it->second).c_str());
    it = p.find("rc_method");
    if ( it != p.end())
    {
        if(it->second.compare("RING_ROUTING_UNI") == 0)
            rc_method = RING_ROUTING_UNI;
        if(it->second.compare("TWONODE_ROUTING") == 0)
            rc_method = TWONODE_ROUTING;
        if(it->second.compare("RING_ROUTING") == 0)
            rc_method = RING_ROUTING;
        if(it->second.compare("TORUS_ROUTING") == 0)
            rc_method = TORUS_ROUTING;
        if(it->second.compare("XY_ROUTING") == 0)
            rc_method = XY_ROUTING;
        
        if(it->second.compare("XY_ROUTING_HETERO") == 0)
            rc_method = XY_ROUTING_HETERO;
    }

        //cout << endl << "Routing method: " << it->second << endl;
    numFlits.resize(ports);
    return;
}

void
SimpleRouter::init( void )
{
    in_buffers.resize(ports);
    decoders.resize(ports);
    input_buffer_state.resize(ports*vcs);
    vca.setup( ports, vcs );
    swa.resize(ports,vcs);
    downstream_credits.resize(ports);
    stat_pp_avg_buff.resize(ports);
    stat_pp_packets_out.resize(ports);
    stat_pp_pkt_out_cy.resize(ports);
    stat_pp_avg_lat.resize(ports);

    for(uint i=0; i<ports; i++)
    {
        decoders[i].grid_size = grid_size;
        decoders[i].node_id = node_id;
        decoders[i].address = 0;
        decoders[i].rc_method = rc_method;
        decoders[i].no_nodes = no_nodes;
        decoders[i].init();
        stat_pp_avg_buff[i].resize(vcs);
        stat_pp_packets_out[i].resize(vcs);
        stat_pp_pkt_out_cy[i].resize(vcs);
        stat_pp_avg_lat[i].resize(vcs);
    }

    // useful when debugging the swa and vca... but not needed for operation
    swa.node_ip = node_id; 
    vca.node_ip = node_id; 
    for(uint i=0; i<ports; i++)
    {
        downstream_credits[i].resize(vcs);
        in_buffers[i].resize( vcs, credits);
        decoders[i].resize( vcs );
    }

    for(uint i=0; i<ports; i++)
        for(uint j=0; j<vcs; j++)
        {
            downstream_credits[i][j] = credits;
            input_buffer_state[i*vcs+j].pipe_stage = INVALID;
            input_buffer_state[i*vcs+j].clear_message = true;
            input_buffer_state[i*vcs+j].address = -1 ;
            input_buffer_state[i*vcs+j].input_port = -1;
            input_buffer_state[i*vcs+j].input_channel = -1;
            input_buffer_state[i*vcs+j].output_port = -1;
            input_buffer_state[i*vcs+j].output_channel = -1;
        }

    reset_stats();

    return ;
}

void
SimpleRouter::reset_stats ( void )
{
    // stats init
    stat_avg_buff = 0;
    stat_packets_out = 0;
    stat_packets_in = 0;
    stat_flits_out = 0;
    stat_flits_in = 0;
    avg_router_latency = 0;
    stat_last_flit_out_cycle= 0;
    stat_3d_packets_out = 0;
    stat_2d_packets_out = 0;
    stat_3d_flits_out = 0;
    stat_2d_flits_out = 0;

    ib_cycles = 0;
    vca_cycles = 0;
    sa_cycles = 0;
    st_cycles = 0;

    for(uint i=0; i<ports; i++)
        for(uint j=0; j<vcs; j++)
        {
            stat_pp_avg_buff[i][j]=0;
            stat_pp_packets_out[i][j]=0;
            stat_pp_pkt_out_cy[i][j]=0;
            stat_pp_avg_lat[i][j]=0;
        }

    return ;
}		/* -----  end of method SimpleRouter::reset_stats  ----- */

const char *mem_state_copy[] = {
  "MEM_INV",
  "MEM_NEW",
  "MEM_MERGED",
  "MEM_OUT_NEW",
  "MEM_OUT_FILL",
  "MEM_OUT_WB",
  "MEM_FILL_NEW",
  "MEM_FILL_WAIT_DONE",
  "MEM_FILL_WAIT_FILL",
  "MEM_DRAM_START",
  "MEM_DRAM_CMD",
  "MEM_DRAM_DATA",
  "MEM_DRAM_DONE",
  "MEM_NOC_START",
  "MEM_NOC_DONE",
};
void
SimpleRouter::handle_link_arrival( int port, LinkData* data )
{
    switch ( data->type ) {
        case FLIT:	
            {
                //_DBG("FLIT came in inport is %d from source %d", port, data->src);
                /* Stats update */
                stat_flits_in++;
                ib_cycles++;
                
                if ( data->f->type == TAIL ) stat_packets_in++;

                uint inport = port%ports;
                
                numFlits[inport]++; //hack FIXME
                
                //push the flit into the buffer. Init for buffer state done
                //inside do_input_buffering
                in_buffers[inport].change_push_channel( data->vc );
                in_buffers[inport].push(data->f);
                
#if NETVIS
  //track each incoming flit for activity factor
  if(nodeAF.size() <= manifold::kernel::Manifold::NowTicks() / 10)
    nodeAF.push_back(0);
  ++nodeAF[ manifold::kernel::Manifold::NowTicks() / 10 ];

  
#endif
                if ( data->f->type == HEAD )
                {
		((HeadFlit*)data->f)->req->m_noc_cycle = manifold::kernel::Manifold::NowTicks();
                #if 1


                    //for csv stats collection/visualization
#if NETVIS
  if( inport == 0 )
  {
    int m_id = ((HeadFlit*)data->f)->req->m_id;
    if ( nodeLatency.find(m_id) == nodeLatency.end() )
      nodeLatency[m_id] = -1 * manifold::kernel::Manifold::NowTicks();
    else
      nodeLatency[m_id] = manifold::kernel::Manifold::NowTicks() + nodeLatency[m_id];
  }
  
  //track flit_id of tail flit
                    cout << manifold::kernel::Manifold::NowTicks() << ",p," 
                        << ((HeadFlit*)data->f)->req->m_id << "," 
                        << ((HeadFlit*)data->f)->req->m_ptx << ","
                        << mem_state_copy[((HeadFlit*)data->f)->req->m_state] << ","
                        << mem_req_noc_type_name[((HeadFlit*)data->f)->req->m_msg_type] << ","
                        << node_id << "," << ((HeadFlit*)data->f)->dst_node << ","
                        << endl;
         /*           
		    cout << manifold::kernel::Manifold::NowTicks() << ",b," << node_id << ","; 
                    for(uint i=0; i<ports; i++)
                        for(uint j=0; j<vcs; j++)
                            cout << in_buffers[i].get_occupancy(j) << ",";
                    
                    cout << "\n";
                    // */
#endif
                #else 
                  //for debugging only!
                  int stage = -1;
                  static const int stage_state[3][3] = {    //transposed array for column major order
                    {7, 0, -1},                             //no idea why it's not row major order
                    { 6, 1, 2}, 
                    {-1, 4, 3}
                  };
                  
                  
                  stage = stage_state[node_id][((HeadFlit*)data->f)->dst_node];
                  stage += (stage == 1 && ((HeadFlit*)data->f)->req->m_state == 12) ? 4 : 0;
                  
                  cout << manifold::kernel::Manifold::NowTicks() << ", "
                    << ((HeadFlit*)data->f)->req->m_id << ", "
                    << stage << ", "
                    << node_id << ", "
                		<< ((HeadFlit*)data->f)->dst_node << ", "
                    << mem_state_copy[((HeadFlit*)data->f)->req->m_state] << ", \n";
                #endif
                    do_input_buffering(static_cast<HeadFlit*>(data->f), inport, data->vc);
                }
                else
                    decoders[inport].push(data->f, data->vc);

                break;
            }
        case CREDIT:	
            {
                /*  Update credit information for downstream buffer
                 *  corresponding to port and vc */
                uint inport = port%ports;
                downstream_credits[inport][data->vc]++;

		if(0)//node_id == 4)
		{
		  cout << manifold::kernel::Manifold::NowTicks() << ", node:4 credit received" << downstream_credits[inport][data->vc] <<  endl;
		}
                break;
            }

        default:
            cerr << " ERROR: SimpleRouter::handle_link_arrival" << endl;
            break;
    }				/* -----  end switch  ----- */

    delete data;
    return ;
}

void
SimpleRouter::do_input_buffering(HeadFlit* hf, uint inport, uint invc)
{
    input_buffer_state[inport*vcs+invc].input_port = inport;
    input_buffer_state[inport*vcs+invc].input_channel = invc;
    input_buffer_state[inport*vcs+invc].address = hf->address;
    input_buffer_state[inport*vcs+invc].destination = hf->dst_node;
    input_buffer_state[inport*vcs+invc].src_node= hf->src_node;
    input_buffer_state[inport*vcs+invc].pipe_stage = FULL;
    input_buffer_state[inport*vcs+invc].mclass = hf->mclass;
    input_buffer_state[inport*vcs+invc].pkt_length = hf->pkt_length;
    input_buffer_state[inport*vcs+invc].pkt_arrival_time = manifold::kernel::Manifold::NowTicks();
    input_buffer_state[inport*vcs+invc].clear_message = false;
    input_buffer_state[inport*vcs+invc].sa_head_done = false;
#ifdef _DEBUG_IRIS
    input_buffer_state[inport*vcs+invc].fid= hf->flit_id;
#endif

    //Routing
    decoders[inport].push(hf,invc);
    input_buffer_state[inport*vcs+invc].possible_oports.clear();
    input_buffer_state[inport*vcs+invc].possible_ovcs.clear();
    uint rc_port = decoders[inport].get_output_port(invc);
    input_buffer_state[inport*vcs+invc].possible_oports.push_back(rc_port);
    uint rc_vc = decoders[inport].get_virtual_channel(invc);
    input_buffer_state[inport*vcs+invc].possible_ovcs.push_back(rc_vc);

    assert ( input_buffer_state[inport*vcs+invc].possible_oports.size() != 0);
    assert ( input_buffer_state[inport*vcs+invc].possible_ovcs.size() != 0);


}

void SimpleRouter::do_vc_allocation()
{
    if(!vca.is_empty())
    {
        vca.pick_winner(in_buffers);
        vca_cycles++;
    }

    for( uint i=0; i<ports; i++)
        for ( uint ai=0; ai<vca.current_winners[i].size(); ai++)
        {
            VCA_unit winner = vca.current_winners[i][ai];
            uint ip = winner.in_port;
            uint ic = winner.in_vc;
            uint op = winner.out_port;
            uint oc = winner.out_vc;
//_DBG(" vca winner %d=%d %d", op, oc, vca.current_winners[i]);
//if(printFlag)
  //printf(" vca winner %d=%d %d", op, oc, vca.current_winners[i]);
  
            uint msgid = ip*vcs+ic;
            if ( input_buffer_state[msgid].pipe_stage == VCA_REQUESTED ) // && downstream_credits[op][oc]==credits)
            {
                input_buffer_state[msgid].output_port = op;
                input_buffer_state[msgid].output_channel= oc;
                input_buffer_state[msgid].pipe_stage = VCA_COMPLETE;
                // if requesting multiple outports make sure to cancel them as
                // pkt will no longer be VCA_REQUESTED
            }

        }

    for( uint i=0; i<(ports*vcs); i++)
        if( input_buffer_state[i].pipe_stage == VCA_COMPLETE)
        {
            uint ip = input_buffer_state[i].input_port;
            uint ic = input_buffer_state[i].input_channel;
            uint op = input_buffer_state[i].output_port;
            uint oc = input_buffer_state[i].output_channel;

            if(in_buffers[ip].get_occupancy(ic))
            {
                swa.request(op, oc, ip, ic);
                input_buffer_state[i].pipe_stage = SWA_REQUESTED;
                                //_DBG(" SWA req %d %d op %d %d", ip, ic, op, oc);
                                //if(printFlag)
                                 //printf(" SWA req %d %d op %d %d", ip, ic, op, oc);
            }
        }

    for( uint i=0; i<(ports*vcs); i++)
        if( input_buffer_state[i].pipe_stage == FULL )
            input_buffer_state[i].pipe_stage = IB;

    /* Request VCA */
    for( uint i=0; i<(ports*vcs); i++)
        if( input_buffer_state[i].pipe_stage == IB )
        {
            uint ip = input_buffer_state[i].input_port;
            uint ic = input_buffer_state[i].input_channel;
            uint op = input_buffer_state[i].possible_oports[0];
            uint oc = -1;
            for ( uint ab=0; ab<input_buffer_state[i].possible_ovcs.size();ab++)
            {
                oc = input_buffer_state[i].possible_ovcs[ab];
//		cout << "node:" << node_id << ",vc" << oc << endl;
                //                if( downstream_credits[op][oc] == credits &&
                if ( !vca.is_requested(op, oc, ip, ic) ) 
                {
//_DBG(" Req vca for msg ip%d %d", ip,ic);
                    input_buffer_state[i].pipe_stage = VCA_REQUESTED;
                    vca.request(op,oc,ip,ic);
                    break;      /* allow just one */
                }
            }
        }
}

void 
SimpleRouter::do_switch_traversal()
{
    /* Switch traversal */
    for( uint i=0; i<ports*vcs; i++)
        if( input_buffer_state[i].pipe_stage == SW_TRAVERSAL)
        {
            uint op = input_buffer_state[i].output_port;
            uint oc = input_buffer_state[i].output_channel;
            uint ip = input_buffer_state[i].input_port;
            uint ic= input_buffer_state[i].input_channel;
            if( in_buffers[ip].get_occupancy(ic)> 0
                && downstream_credits[op][oc]>0 )
            {
                in_buffers[ip].change_pull_channel(ic);
                Flit* f = in_buffers[ip].pull();
                f->virtual_channel = oc;

                //If the tail is going out then its cleared later
                swa.request(op, oc, ip, ic);
                input_buffer_state[i].pipe_stage = SWA_REQUESTED;

                if( f->type == TAIL || f->pkt_length == 1)
                {
                    /* Update packet stats */
                    uint lat = manifold::kernel::Manifold::NowTicks()-
                        input_buffer_state[i].pkt_arrival_time;
                    avg_router_latency += lat;
                    stat_packets_out++;
                    stat_pp_packets_out[op][oc]++;
                    stat_pp_pkt_out_cy[op][oc] = manifold::kernel::Manifold::NowTicks();
                    stat_pp_avg_lat[op][oc] += lat;

                    if  ( op != 5 && op != 0)  stat_2d_packets_out++; else if ( op == 5) stat_3d_packets_out++;
//                    printf("Time%lld node:%d stat_3d_packets_out %lld stat_2d_packets_out:%lld op%d addr:%llx\n",
//                           manifold::kernel::Manifold::NowTicks(), node_id, stat_3d_packets_out,stat_2d_packets_out,op, input_buffer_state[i].address);
                    
                    input_buffer_state[i].clear_message = true;
                    input_buffer_state[i].pipe_stage = EMPTY;
                    input_buffer_state[i].input_port = -1;
                    input_buffer_state[i].input_channel = -1;
                    input_buffer_state[i].output_port = -1;
                    input_buffer_state[i].output_channel = -1;
                    vca.clear_winner(op, oc, ip, ic);
                    swa.clear_requestor(op, ip,oc);
                    input_buffer_state[i].possible_oports.clear();
                    input_buffer_state[i].possible_ovcs.clear();
                }

                stat_flits_out++;
                st_cycles++;
                if  ( op != 5 && op != 0)  stat_2d_flits_out++; else if ( op == 5) stat_3d_flits_out++;

                LinkData* ld = new LinkData();
                ld->type = FLIT;
                ld->src = this->GetComponentId();
                f->virtual_channel = oc;
                ld->f = f;
                ld->vc = oc;

                //Not using send then identify between router and interface
                //Use manifold send here to push the ld obj out

                Send( data_outports.at(op), ld);    //schedule cannot be used here as the component is not on the same LP

                /*! \brief Send a credit back and update buffer state for the
                 * downstream router buffer */
                downstream_credits[op][oc]--;
#if NETVIS  //out going flit
  if(nodeAF.size() <= manifold::kernel::Manifold::NowTicks() / 10)
    nodeAF.push_back(0);
  ++nodeAF[ manifold::kernel::Manifold::NowTicks() / 10 ];

#endif
//                if (numFlits[ip]-- < MAX_NUM_FLITS)
                {
                    LinkData* ldc = new LinkData();
                    ldc->type = CREDIT;
                    ldc->src = this->GetComponentId();
                    ldc->vc = ic;
    //_DBG(" FLIT out %d=%d", op,oc);

                    Send(signal_outports.at(ip),ldc);   /* Int not on same LP */
                    stat_last_flit_out_cycle= manifold::kernel::Manifold::NowTicks();
                }

#if 0 //NETVIS
                    cout << manifold::kernel::Manifold::NowTicks() << ",b," << node_id << ","; 
                    for(uint i=0; i<ports; i++)
                        for(uint j=0; j<vcs; j++)
                            cout << in_buffers[i].get_occupancy(j) << ",";
                    
                    cout << "\n";
#endif
            }
            else
            {
                input_buffer_state[i].pipe_stage = VCA_COMPLETE;
                swa.clear_requestor(op, ip,oc);
            }
        }
}

void 
SimpleRouter::do_switch_allocation()
{
    /* Switch Allocation */
    if(node_id == 4)
{ 
//cout << "node:4 print0" << endl;
}

    for( uint i=0; i<ports*vcs; i++)
        if( input_buffer_state[i].pipe_stage == SWA_REQUESTED)
            if ( !swa.is_empty())
            {
                sa_cycles++;    // stat
                
                uint op = -1, oc = -1;
                SA_unit sa_winner;
                uint ip = input_buffer_state[i].input_port;
                uint ic = input_buffer_state[i].input_channel;
                op = input_buffer_state[i].output_port;
                oc= input_buffer_state[i].output_channel;
                sa_winner = swa.pick_winner(op);


                bool alloc_done = false;
                if(input_buffer_state[i].sa_head_done)
                {
                    if( sa_winner.port == ip && sa_winner.ch == ic
                        && in_buffers[ip].get_occupancy(ic) > 0
                        && downstream_credits[op][oc]>0 )
                    {
                        input_buffer_state[i].pipe_stage = SW_TRAVERSAL;
                        alloc_done = true;
                    }
                }
                else
                {
                    if( sa_winner.port == ip && sa_winner.ch == ic
                        //                        && in_buffers[ip].get_occupancy(ic) > 0
                        && downstream_credits[op][oc]== credits )
                    {
		if(node_id == 4)
		{
		  //cout << "node:4 print1-3" << endl;
		}
                        input_buffer_state[i].sa_head_done = true;
                        input_buffer_state[i].pipe_stage = SW_TRAVERSAL;
                        alloc_done = true;
			//*
	/*	        Flit* f = in_buffers[ip].buffers[ic].front();
    	                if( f != NULL && f->type == HEAD )
                        {
                            //cout << "swa winner:" << ((HeadFlit*)f)->req->m_id <<  ",src:" << node_id << ",dst:" << ((HeadFlit*)f)->dst_node <<  endl;
                        }
			for(int i=0; i<ports; i++)
			{
			    for(int j=0; j<vcs; j++)
			    {
				for (auto I = in_buffers[i].buffers[j].begin(), E = in_buffers[i].buffers[j].end(); I != E; ++I) {
					if( (*I) != NULL && (*I)->type == HEAD )
					{
					    HeadFlit* H = (HeadFlit*)(*I);
					    cout << "node:" << node_id << ",m_id:" << H->req->m_id << "," << H->req->m_noc_cycle << endl;
					}
				}
			    }
			}

	*/
#if 0                       
                        cout << "IRIS packet " << ((HeadFlit*)f)->req->m_id << " @ node "
		                    << node_id << " in oport " << data_outports.at(op)
		                    << "\n";
#endif
                    }

                }
                if ( !alloc_done )
                {
                    input_buffer_state[i].pipe_stage = VCA_COMPLETE;
                    swa.clear_requestor(op, ip,oc);
                }

            }else{

		if(node_id == 4)
		{
		  //cout << "node:4 print5 swa empty" << endl;
		}
	}
}
void
SimpleRouter::tick ( void )
{
    return ;
}

extern void dump_state_at_deadlock(macsim_c* simBase);
void
SimpleRouter::tock ( void )
{
    /* 
     * This can be used to check if a message does not move for a large
     * interval. ( eg. interval of arrival_time + 100 )
     *
     for ( uint i=0; i<input_buffer_state.size(); i++)
     if (input_buffer_state[i].pipe_stage != EMPTY
     && input_buffer_state[i].pipe_stage != INVALID
     && input_buffer_state[i].pkt_arrival_time+1000 < manifold::kernel::Manifold::NowTicks())
     {
     fprintf(stderr,"\n\nDeadlock at Router %d node %d Msg id %d Fid%d", GetComponentId(), node_id, i, input_buffer_state[i].fid);
     dump_state_at_deadlock(m_simBase);
     }
      /**/
    for(uint i=0; i<ports; i++)
        for(uint j=0; j<vcs; j++)
        {
            stat_pp_avg_buff[i][j] += in_buffers[i].get_occupancy(j);
            stat_avg_buff += in_buffers[i].get_occupancy(j);
        }
    //FIXME
    //if(printFlag && node_id == 11)
      //cout << "packet 0 in node 11\n";
    
    do_switch_traversal();
    do_switch_allocation();
    do_vc_allocation();

    return ;
}

std::string
SimpleRouter::print_stats ( void ) const
{
    std::stringstream str;
    str 
        << "\n SimpleRouter[" << node_id << "] packets_in: " << stat_packets_in
        << "\n SimpleRouter[" << node_id << "] packets_out: " << stat_packets_out
        << "\n SimpleRouter[" << node_id << "] flits_in: " << stat_flits_in
        << "\n SimpleRouter[" << node_id << "] flits_out: " << stat_flits_out
        << "\n SimpleRouter[" << node_id << "] avg_router_latency: " << (avg_router_latency+0.0)/stat_packets_out
        << "\n SimpleRouter[" << node_id << "] last_pkt_out_cy: " << stat_last_flit_out_cycle
//        << "\n SimpleRouter[" << node_id << "] stat_3d_packets_out: " << stat_3d_packets_out 
//        << "\n SimpleRouter[" << node_id << "] stat_2d_packets_out: " << stat_2d_packets_out 
//        << "\n SimpleRouter[" << node_id << "] stat_3d_flits_out: " << stat_3d_flits_out 
//        << "\n SimpleRouter[" << node_id << "] stat_2d_flits_out: " << stat_2d_flits_out 
//        << "\n SimpleRouter[" << node_id << "] inj_nodes_packets_out: " << stat_packets_out - ( stat_2d_packets_out + stat_3d_packets_out )
//        << "\n SimpleRouter[" << node_id << "] local_remote_ratio: " << stat_3d_packets_out*1.0/stat_2d_packets_out 
//        << "\n SimpleRouter[" << node_id << "] stat_3d_flits_out: " << stat_3d_flits_out
//        << "\n SimpleRouter[" << node_id << "] stat_2d_flits_out: " << stat_2d_flits_out
        << "\n SimpleRouter[" << node_id << "] ib_cycles: " << ib_cycles
        << "\n SimpleRouter[" << node_id << "] sa_cycles: " << sa_cycles 
        << "\n SimpleRouter[" << node_id << "] vca_cycles: " << vca_cycles
        << "\n SimpleRouter[" << node_id << "] st_cycles: " << st_cycles
        ;
    str << "\n SimpleRouter[" << node_id << "] per_port_avg_buff: ";
    
    for ( uint i=0; i<ports; i++)
        for ( uint j=0; j<vcs; j++)
            str << stat_pp_avg_buff[i][j]*1.0/manifold::kernel::Manifold::NowTicks() << " ";

    str << "\n SimpleRouter[" << node_id << "] per_port_pkts_out: ";
    for ( uint i=0; i<ports; i++)
        for ( uint j=0; j<vcs; j++)
            str << stat_pp_packets_out[i][j] << " ";

    str << "\n SimpleRouter[" << node_id << "] per_port_avg_lat: ";
    for ( uint i=0; i<ports; i++)
        for ( uint j=0; j<vcs; j++)
            if ( stat_pp_packets_out[i][j] ) 
                str  << (stat_pp_avg_lat[i][j]+0.0)/stat_pp_packets_out[i][j] << " ";
            else
                str  << "0 ";

    str << "\n SimpleRouter[" << node_id << "] per_port_last_pkt_out_cy: ";
    for ( uint i=0; i<ports; i++)
        for ( uint j=0; j<vcs; j++)
            str << stat_pp_pkt_out_cy[i][j] << " ";

    if ( ( stat_packets_in - stat_packets_out ) > ports*vcs ) str << "\n ERROR pkts_in-pkts_out > buffering available";
    str << std::endl;
    
    cout << str.str();
    return str.str();
}

std::string
SimpleRouter::print_csv_stats ( void ) const
{
    std::stringstream str;
    str << node_id << ","
        << stat_packets_out << "," << stat_flits_in << "," << stat_flits_out << "," 
        << ib_cycles << "," << sa_cycles << "," << vca_cycles << "," << st_cycles << ",";

    double buff = 0.0;
    double lat = 0.0;
    for ( uint i=0; i<ports; i++)
        for ( uint j=0; j<vcs; j++)
            buff += stat_pp_avg_buff[i][j]*1.0/manifold::kernel::Manifold::NowTicks();
    buff /= ports*vcs;
    
    for ( uint i=0; i<ports; i++)
        for ( uint j=0; j<vcs; j++)
            if ( stat_pp_packets_out[i][j] ) 
                lat += (stat_pp_avg_lat[i][j]+0.0)/stat_pp_packets_out[i][j];
    lat /= ports*vcs;
    
    str << buff << "," << (avg_router_latency+0.0)/stat_packets_out << "," << lat;
    
    for ( uint i=0; i<ports; i++)
        for ( uint j=0; j<vcs; j++)
            str << stat_pp_avg_buff[i][j]*1.0/manifold::kernel::Manifold::NowTicks() << ",";
    
    for ( uint i=0; i<ports; i++)
        for ( uint j=0; j<vcs; j++)
            if ( stat_pp_packets_out[i][j] ) 
                str << (stat_pp_avg_lat[i][j]+0.0)/stat_pp_packets_out[i][j] << ",";
    
    str << endl;

    //cout << str.str();
    return str.str();
}


#if 0 
void alt_energy_stat()
{
    /////////////////energy//////////////////////////
    
    SIM_router_power_t router_power;
    SIM_router_info_t router_info;
    SIM_router_power_t *router = &router_power;
    SIM_router_info_t *info = &router_info;
    
    double Pbuf = 0, Pxbar = 0, Pvc_arbiter = 0, Psw_arbiter = 0, Pclock = 0, Ptotal = 0;
	double Pbuf_static = 0, Pxbar_static = 0, Pvc_arbiter_static = 0, Psw_arbiter_static = 0, Pclock_static = 0;
	double Pbuf_dyn = 0, Pxbar_dyn = 0, Pvc_arbiter_dyn = 0, Psw_arbiter_dyn = 0, Pclock_dyn = 0;
	
    int total_cycles = manifold::kernel::Manifold::NowTicks();
    double e_in_buf_rw = ib_cycles / total_cycles;
    double e_cbuf_fin = st_cycles /total_cycles;
    double e_fin = sa_cycles / total_cycles;
    double e_vca = vca_cycles /total_cycles;
    
    double Eavg = 0, Eatomic, Estruct, Estatic = 0;
    double freq = 1.3e9;
    int vc_allocator_enabled = 1;
    int max_avg = 0;
    char path[80] = "testing";
    
    double ret = SIM_router_init(info, router, NULL);
    
    info->n_total_in = ports;
    info->n_total_out = ports;
    info->n_switch_out = ports;
    
	if(info->sw_out_arb_model || info->sw_out_arb_model){
		Psw_arbiter_dyn = Eavg * freq - Pbuf_dyn - Pxbar_dyn;
		Psw_arbiter_static = router->I_sw_arbiter_static * Vdd * SCALE_S;
		Psw_arbiter = Psw_arbiter_dyn + Psw_arbiter_static;
	}

//    ret = SIM_router_stat_energy(&GLOB(info), &GLOB(router_power), 10, path, AVG_ENERGY, 1050.0, 1, PARM(Freq));
    uint path_len = SIM_strlen(path);
    int next_depth = 0;
    
    if (info->in_buf) {
		Eavg += SIM_array_stat_energy(&info->in_buf_info, &router->in_buf, e_in_buf_rw, e_in_buf_rw, next_depth, SIM_strcat(path, "input buffer"), max_avg); 
		SIM_res_path(path, path_len); //resets path string
	}
	
	Pbuf_dyn = Eavg * freq;
	Pbuf_static = router->I_buf_static * Vdd * SCALE_S;
	Pbuf = Pbuf_dyn + Pbuf_static;

	/* main crossbar */
	if (info->crossbar_model) {
		Eavg += SIM_crossbar_stat_energy(&router->crossbar, next_depth, SIM_strcat(path, "crossbar"), max_avg, e_cbuf_fin);
		SIM_res_path(path, path_len);
	}
	
    Pxbar_dyn = (Eavg * freq - Pbuf_dyn);
	Pxbar_static = router->I_crossbar_static * Vdd * SCALE_S;
	Pxbar = Pxbar_dyn + Pxbar_static;

	/* switch allocation (arbiter energy only) */
	/* input (local) arbiter for switch allocation*/
	if (info->sw_in_arb_model) {
		/* assume # of active input arbiters is (info->in_n_switch * info->n_in * e_fin) 
		 * assume (info->n_v_channel*info->n_v_class)/2 vcs are making request at each arbiter */

		Eavg += SIM_arbiter_stat_energy(&router->sw_in_arb, &info->sw_in_arb_queue_info, (info->n_v_channel*info->n_v_class)/2, next_depth, SIM_strcat(path, "switch allocator input arbiter"), max_avg) * info->in_n_switch * info->n_in * e_fin;
		SIM_res_path(path, path_len);
    }
    
    /* output (global) arbiter for switch allocation*/
	if (info->sw_out_arb_model) {
		/* assume # of active output arbiters is (info->n_switch_out * (e_cbuf_fin/info->n_switch_out)) 
		 * assume (info->n_in)/2 request at each output arbiter */

		Eavg += SIM_arbiter_stat_energy(&router->sw_out_arb, &info->sw_out_arb_queue_info, info->n_in / 2, next_depth, SIM_strcat(path, "switch allocator output arbiter"), max_avg) * info->n_switch_out * (e_cbuf_fin / info->n_switch_out);

		SIM_res_path(path, path_len); 
	}

	if(info->sw_out_arb_model || info->sw_out_arb_model){
		Psw_arbiter_dyn = Eavg * freq - Pbuf_dyn - Pxbar_dyn;
		Psw_arbiter_static = router->I_sw_arbiter_static * Vdd * SCALE_S;
		Psw_arbiter = Psw_arbiter_dyn + Psw_arbiter_static;
	}
	
	/* virtual channel allocation (arbiter energy only) */
	/* HACKs:
	 *   - assume 1 header flit in every 5 flits for now, hence * 0.2  */

	if(info->vc_allocator_type == ONE_STAGE_ARB && info->vc_out_arb_model  ){
		/* one stage arbitration (vc allocation)*/
		/* # of active arbiters */
		double nActiveArbs = e_fin * info->n_in * 0.2 / 2; //flit_rate * n_in * 0.2 / 2

		/* assume for each active arbiter, there is 2 requests on average (should use expected value from simulation) */	
		Eavg += SIM_arbiter_stat_energy(&router->vc_out_arb, &info->vc_out_arb_queue_info,
				1, next_depth,
				SIM_strcat(path, "vc allocation arbiter"),
				max_avg) * nActiveArbs;

		SIM_res_path(path, path_len);
	}
	else if(info->vc_allocator_type == TWO_STAGE_ARB && info->vc_in_arb_model && info->vc_out_arb_model){
		/* first stage arbitration (vc allocation)*/
		if (info->vc_in_arb_model) {
			// # of active stage-1 arbiters (# of new header flits)
			double nActiveArbs = e_fin * info->n_in * 0.2;


			/* assume an active arbiter has n_v_channel/2 requests on average (should use expected value from simulation) */
			Eavg += SIM_arbiter_stat_energy(&router->vc_in_arb, &info->vc_in_arb_queue_info, info->n_v_channel/2, next_depth, 
					SIM_strcat(path, "vc allocation arbiter (stage 1)"),
					max_avg) * nActiveArbs; 

			SIM_res_path(path, path_len);
		}

		/* second stage arbitration (vc allocation)*/
		if (info->vc_out_arb_model) {
			/* # of active stage-2 arbiters */
			double nActiveArbs = e_fin * info->n_in * 0.2 / 2; //flit_rate * n_in * 0.2 / 2

			/* assume for each active arbiter, there is 2 requests on average (should use expected value from simulation) */
			Eavg += SIM_arbiter_stat_energy(&router->vc_out_arb, &info->vc_out_arb_queue_info,
					2, next_depth, 
					SIM_strcat(path, "vc allocation arbiter (stage 2)"),
					max_avg) * nActiveArbs;

			SIM_res_path(path, path_len);
		}
	}
	else if(info->vc_allocator_type == VC_SELECT && info->n_v_channel > 1 && info->n_in > 1){
		double n_read = e_fin * info->n_in * 0.2;
		double n_write = e_fin * info->n_in * 0.2;
		Eavg += SIM_array_stat_energy(&info->vc_select_buf_info, &router->vc_select_buf, n_read , n_write, next_depth, SIM_strcat(path, "vc selection"), max_avg);
		SIM_res_path(path, path_len);

	}
	else{
		vc_allocator_enabled = 0; //set to 0 means no vc allocator is used
	}

	if(info->n_v_channel > 1 && vc_allocator_enabled){
		Pvc_arbiter_dyn = Eavg * freq - Pbuf_dyn - Pxbar_dyn - Psw_arbiter_dyn; 
		Pvc_arbiter_static = router->I_vc_arbiter_static * Vdd * SCALE_S;
		Pvc_arbiter = Pvc_arbiter_dyn + Pvc_arbiter_static;
	}

	/*router clock power (supported for 90nm and below) */
	if(PARM(TECH_POINT) <=90){
		Eavg += SIM_total_clockEnergy(info, router);
		Pclock_dyn = Eavg * freq - Pbuf_dyn - Pxbar_dyn - Pvc_arbiter_dyn - Psw_arbiter_dyn;
		Pclock_static = router->I_clock_static * Vdd * SCALE_S;
		Pclock = Pclock_dyn + Pclock_static;
	}

	/* static power */
	Estatic = router->I_static * Vdd * Period * SCALE_S;
	SIM_print_stat_energy(SIM_strcat(path, "static energy"), Estatic, next_depth);
	SIM_res_path(path, path_len);
	Eavg += Estatic;
	Ptotal = Eavg * freq;
	
	//try using a more general activity factor
//	double activity_factor = stat_packets_in / total_cycles;
//	
//	Eavg = SIM_router_stat_energy(info, router, 0, path, AVG_ENERGY, activity_factor, 1, PARM(Freq));
	cout << "Node:" << node_id << "Packets: " << stat_packets_in << " Total Power:" << Eavg * freq << "\n";
	
	m_simBase->total_energy += Eavg * total_cycles;
	m_simBase->avg_power += Eavg * freq;

//    double SIM_router_stat_energy(SIM_router_info_t *info, SIM_router_power_t *router, int print_depth, char *path, int max_avg, double e_fin, int plot_flag, double freq)

//    Eavg += sa_cycles * SIM_arbiter_init(&router->sw_in_arb, info->sw_in_arb_model, info->sw_in_arb_ff_model, info->n_v_channel*info->n_v_class, 0, &info->sw_in_arb_queue_info);
//    
//    Eavg += vca_cycles * SIM_arbiter_init(&router->sw_in_arb, info->sw_in_arb_model, info->sw_in_arb_ff_model, info->n_v_channel*info->n_v_class, 0, &info->sw_in_arb_queue_info);
//    
//    Eavg += st_cycles * SIM_crossbar_stat_energy(&router->crossbar, next_depth, SIM_strcat(path, "crossbar"), max_avg, e_cbuf_fin);
//    
//    Eavg += ib_cycles * 
    
} /* ----- end of function GenericPktGen::toString ----- */
#endif


void SimpleRouter::power_stats()
{
    SIM_router_power_t router_power;
    SIM_router_info_t router_info;
    SIM_router_power_t *router = &router_power;
    SIM_router_info_t *info = &router_info;
    double Pdynamic, Pleakage, Plink;
    char path[80] = "router_power";
    double freq = 1.3e9;
    double link_len = .005;     //unit meter
    uint data_width = 128;
    int total_cycles = manifold::kernel::Manifold::NowTicks();
    double activity_factor = (double)stat_packets_in / total_cycles;
    
    double ret = SIM_router_init(info, router, NULL);
    
    info->n_total_in = ports;
    info->n_total_out = ports;
    info->n_switch_out = ports;
    
    //link power
    Pdynamic = 0.5 * activity_factor * LinkDynamicEnergyPerBitPerMeter(link_len, Vdd) * freq * link_len * (double)data_width;
	Pleakage = LinkLeakagePowerPerMeter(link_len, Vdd) * link_len * data_width;
	Plink = (Pdynamic + Pleakage) * PARM(in_port);
	
    double Eavg = SIM_router_stat_energy(info, router, 0, path, AVG_ENERGY, activity_factor, 0, freq);
	cout << "Node:" << node_id << " Packets: " << stat_packets_in 
	    << " Router Power:" << Eavg * freq 
	    << " Link Power:" << Plink << "\n";
	
#ifdef POWER_EI
	m_simBase->total_energy += Eavg * total_cycles;
	m_simBase->avg_power += Eavg * freq + Plink;
	m_simBase->total_packets += stat_packets_in;
#endif
}
#endif   /* ----- #ifndef SIMPLEROUTER_CC_INC  ----- */

