
#ifndef  _genericvcallocator_cc_INC
#define  _genericvcallocator_cc_INC

#include "genericVcAllocator.h"

GenericVcAllocator::GenericVcAllocator()
{
    name = "genericVcAllocator";
}

GenericVcAllocator::~GenericVcAllocator()
{

}

void
GenericVcAllocator::setup( uint p, uint v )
{
    ports = p;
    vcs = v;
    current_winners.resize(ports);
    ovc_tokens.resize(ports);
    requestors.resize(ports);
    last_winner.resize(ports);
    requestors_vc.resize(ports);

    for ( uint i=0; i<ports; i++ )
    {
        ovc_tokens[i].resize( vcs);
        requestors[i].resize(ports*vcs);
        requestors_vc[i].resize(ports*vcs);
        last_winner[i].resize(vcs);
    }

    for ( uint i=0; i<ports; i++ )
        for ( uint j=0; j<ports*vcs; j++ )
        {
            requestors[i][j] = false;
            requestors_vc[i][j] = 99;
        }

    for ( uint i=0; i<ports; i++ )
        for ( uint j=0; j<vcs; j++ )
            ovc_tokens[i][j] = j;

}

bool
GenericVcAllocator::request( uint op , uint ovc, uint ip, uint invc )
{ 
    if ( !requestors[op][ip+ports*invc] )
    {
        requestors.at(op).at(ip+ports*invc) = true;
        requestors_vc.at(op).at(ip+ports*invc) = ovc;
    }

    return requestors.at(op).at(ip+ports*invc);
}

bool
GenericVcAllocator::is_empty()
{
    for ( uint i=0; i<ports; i++ )
        if (!is_empty(i))
            return false;

    return true;
}

bool
GenericVcAllocator::is_empty(uint i)
{
    for ( uint j=0; j<ports*vcs; j++)
        if ( requestors[i][j] )
            return false;

    return true;
}

int
GenericVcAllocator::no_requestors(uint op)
{
    uint no = 0;
    for ( uint j=0; j<ports*vcs; j++)
        if ( requestors[op][j] )
            no++;
    return no;
}


void
GenericVcAllocator::pick_winner(std::vector <GenericBuffer> &in_buffers)
{
    if( !is_empty())
    {
        for ( uint i=0; i<ports; i++)
            for ( uint j=0; j<vcs; j++)
            {
                // are there tokens for this port/vc pair
                std::vector<uint>::iterator it = ovc_tokens[i].begin();
                it = find (ovc_tokens[i].begin(), ovc_tokens[i].end(), j);
                if ( it != ovc_tokens[i].end())
                {
                    /* 
                    printf("\nIter for port %d vc %d tokens:-",i,j);
                    for ( uint ss=0; ss<ovc_tokens.size(); ss++)    
                        printf(" %d|",ovc_tokens[i][ss]); 
                    printf(" reqs:-");
                    for ( uint ss=0; ss<ports*vcs; ss++)    
                        std::cout << " " <<requestors[i][ss] << "-" << requestors_vc[i][ss];
                     * */
                    uint st_loc = last_winner[i][j]+1;
                    bool found = false;
                    for ( uint jj = st_loc; jj<ports*vcs; jj++) 
                        if ( requestors[i][jj] && requestors_vc[i][jj] == j )
                        {
                            VCA_unit tmp;
                            std::vector<uint>::iterator it2 = ovc_tokens[i].begin();
                            it2 = find (ovc_tokens[i].begin(), ovc_tokens[i].end(), j);
                            tmp.out_vc = *it2;
                            ovc_tokens[i].erase(it);

                            tmp.out_port = i;
                            tmp.in_port = (int)(jj%ports);
                            tmp.in_vc= (int)(jj/ports);
                            current_winners[i].push_back(tmp);
                            last_winner[i][j] = jj;
                            found = true;
			    /*
			    Flit* f = in_buffers[jj%ports].buffers[jj/ports].front();
			    if( f->type == HEAD )
			    {
			    	cout << "vca winner:" << ((HeadFlit*)f)->req->m_id <<  ",src:" << node_ip << ",dst:" << ((HeadFlit*)f)->dst_node <<  endl;
			    }
			   // */
                            break;
                        }

                    if ( !found )
                        for ( uint jj =0; jj<st_loc; jj++) 
                            if ( requestors[i][jj] && requestors_vc[i][jj]==j)
                            {
                                VCA_unit tmp;
                                tmp.out_vc = *it;
                                ovc_tokens[i].erase(it);

                                tmp.out_port = i;
                                tmp.in_port = (int)(jj%ports);
                                tmp.in_vc= (int)(jj/ports);
                                current_winners[i].push_back(tmp);
                                last_winner[i][j] = jj;
                                break;
                            }

                }
            }
    }
    return;
}

void
GenericVcAllocator::clear_winner(uint op, uint ovc, uint ip, uint ivc)
{
    requestors[op][ip+ports*ivc] = false;
    for ( uint i=0; i<current_winners[op].size(); i++)
    {
        VCA_unit tmp = current_winners[op][i];
        if ( tmp.in_port == ip &&
             tmp.in_vc == ivc )
            current_winners[op].erase(current_winners[op].begin()+i);
    }
    ovc_tokens[op].push_back(ovc);
}

bool
GenericVcAllocator::is_requested( uint op, uint ovc, uint ip, uint ivc )
{
    return requestors.at(op).at(ip+ports*ivc);
}

#endif   /* ----- #ifndef _genericvcallocator_cc_INC  ----- */

