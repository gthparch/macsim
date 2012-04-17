#ifndef  _genericaddressdecoder_cc_INC
#define  _genericaddressdecoder_cc_INC

#include	"genericRC.h"

GenericRC::GenericRC()
{
    srand(time(NULL));
    rc_method = RING_ROUTING;
    do_request_reply_network = false;
    
    
}

void GenericRC::init()
{
    if( rc_method == XY_ROUTING || rc_method == XY_ROUTING_HETERO)
    {
        grid_xloc.resize(no_nodes);
        grid_yloc.resize(no_nodes);
        for( uint i = 0; i < no_nodes; i++ )
        {
            grid_xloc[i] = i % grid_size;
            grid_yloc[i] = i / grid_size;
        }
        
//        possible_out_vcs.push_back(0);
    }
}
uint
GenericRC::route_x_y(uint dest)
{
    uint oport = -1;
    uint myx=-1, destx=-1, myy =-1, desty=-1;
    myx = grid_xloc[node_id];
    myy = grid_yloc[node_id];
    destx = grid_xloc[ dest ];
    desty = grid_yloc[ dest ];
    if ( myx == destx && myy == desty )
        oport = 0;
    else if ( myx ==  destx )
    {
        if( desty < myy )
            oport = 3;
        else
            oport = 4;
    }
    else
    {
        if( destx < myx )
            oport = 1;
        else
            oport = 2;
    }

    return oport;
}

//want spinal routers to be flexible: prefer y traversal, but can use x if adjacent router is also spinal
//FIXME only does X then Y for now, as GPU nodes only have lateral links - consider using MECS eventually?
void
GenericRC::route_x_y_hetero( HeadFlit* hf )

{
    if ( hf->req->m_id == 458 )
		{
			cout << "node id " << node_id << "\n";
            cout << "routing: \n";
		}
    uint dest = hf->dst_node;
    uint myx=-1, destx=-1, myy =-1, desty=-1;
    myx = grid_xloc[node_id];
    myy = grid_yloc[node_id];
    destx = grid_xloc[ dest ];
    desty = grid_yloc[ dest ];
    if ( myx == destx && myy == desty )
        possible_out_ports.push_back(0);
    else if ( myx < 3 || myx > 4 )//case is gpu
    {
        if( destx < myx )
            possible_out_ports.push_back(1);
        else
            possible_out_ports.push_back(2);
        
    } //case is mc/l3
    else if ( myy !=  desty )   
    {
        if( desty < myy )
        {
            possible_out_ports.push_back(3);
//            possible_out_vcs.push_back(5);     //alternative to 3
        }
        else
        {
            possible_out_ports.push_back(4);
//            possible_out_vcs.push_back(6);
        }
    }else
    {
        if( destx < myx )
            possible_out_ports.push_back(1);
        else
            possible_out_ports.push_back(2);
    }

    return;
}

void
GenericRC::route_torus(HeadFlit* hf)
{
    uint myx = (int)(node_id%grid_size);
    uint destx = (int)(hf->dst_node%grid_size);
    uint myy = (int)(node_id/grid_size);
    uint desty = (int)(hf->dst_node/grid_size);

    if ( myx == destx  && myy == desty )
    {
        possible_out_ports.push_back(0);
        if ( hf->mclass == MC_RESP)
            possible_out_vcs.push_back(1);
        else
            possible_out_vcs.push_back(0);
        return;
    } 
    else if ( myx == destx ) /*  reached row but not col */
    {
        /*  Decide the port based on hops around the ring */
        if ( desty > myy )
        {
            if ((desty-myy)>grid_size/2)
                possible_out_ports.push_back(3);

            else
                possible_out_ports.push_back(4);
        }
        else
        {
            if ((myy - desty )>grid_size/2)
                possible_out_ports.push_back(4);
            else
                possible_out_ports.push_back(3);
        }

        /*  Decide the vc  */
        possible_out_vcs.resize(1);
        if(  possible_out_ports[0] == 3)
        {
            desty = (grid_size-desty)%grid_size;
            myy= (grid_size-myy)%grid_size;
        }

        if ( desty > myy )
        {
            if ( hf->mclass == MC_RESP)
                possible_out_vcs[0] = 3;
            else
                possible_out_vcs[0] = 2;
        }
        else
        {
            if ( hf->mclass == MC_RESP)
                possible_out_vcs[0] = 1;
            else
                possible_out_vcs[0] = 0;
        }

        return;
    } 
    /*  both row and col dont match do x first. Y port is 
     *  adaptive in this case and can only be used with the adaptive vc */
    else 
    {
        if ( destx > myx )
        {
            if ((destx - myx)>grid_size/2)
                possible_out_ports.push_back(1);
            else
                possible_out_ports.push_back(2);
        }
        else
        {
            if ((myx - destx )>grid_size/2)
                possible_out_ports.push_back(2);
            else
                possible_out_ports.push_back(1);
        }

        /*  Decide the vc  */
        possible_out_vcs.resize(1);
        if(  possible_out_ports[0] == 1)
        {
            destx = (grid_size-destx)%grid_size;
            myx= (grid_size-myx)%grid_size;
        }

        if ( destx > myx )
        {
            if ( hf->mclass == MC_RESP)
                possible_out_vcs[0] = 3;
            else
                possible_out_vcs[0] = 2;
        }
        else
        {
            if ( hf->mclass == MC_RESP)
                possible_out_vcs[0] = 1;
            else
                possible_out_vcs[0] = 0;
        }

        return;
    }

    cout << "ERROR: dint return yet " << endl;

    return;
}

// this routes unidirectional
void
GenericRC::route_ring_uni(HeadFlit* hf)
{

    grid_size = no_nodes;
    uint myx = node_id;
    uint destx = hf->dst_node;

    if ( myx == destx )
    {
        possible_out_ports.push_back(0);
        if ( hf->mclass== PROC_REQ)
            possible_out_vcs.push_back(0);
        else
        {
            possible_out_vcs.push_back(1);
            // should be able to add 0-4 vcs here but make sure vca can
            // handle multiple selections
        }

        return;
    } 
    else
    {
        possible_out_ports.push_back(2);

        /*  Decide the vc  */
        possible_out_vcs.resize(1);
	possible_out_vcs[0] = rand() % 4;
	return;

        if ( destx > myx )
        {
            if ( hf->mclass == PROC_REQ )
                possible_out_vcs[0] = 2;
            else
                possible_out_vcs[0] = 3;
        }   
        else
        {
            if ( hf->mclass == PROC_REQ)
                possible_out_vcs[0] = 0;
            else
                possible_out_vcs[0] = 1;
        }

        return;
    } 
    cout << "ERROR: dint return yet " << endl;

    return;
}

void
GenericRC::route_ring(HeadFlit* hf)
{

    grid_size = no_nodes;
    uint myx = node_id;
    uint destx = hf->dst_node;

    if ( myx == destx )
    {
        possible_out_ports.push_back(0);
        if ( hf->mclass== PROC_REQ)
            possible_out_vcs.push_back(0);
        else
        {
            possible_out_vcs.push_back(1);
            // should be able to add 0-4 vcs here but make sure vca can
            // handle multiple selections
        }

        return;
    } 
    else
    {
        if ( destx > myx)
        {
            if ( (destx - myx) > grid_size/2) 
                possible_out_ports.push_back(1);
            else
                possible_out_ports.push_back(2);
        }
        else
        {
            if ( (myx - destx) > grid_size/2) 
                possible_out_ports.push_back(2);
            else
                possible_out_ports.push_back(1);
        }

        /*  Decide the vc  */
        possible_out_vcs.resize(1);
//        possible_out_vcs[0] = rand() % 4;
//		return;
        if(  possible_out_ports[0] == 1)
        {
            destx = (grid_size-destx)%grid_size;
            myx= (grid_size-myx)%grid_size;
        }

        if ( destx > myx )
        {
            if ( hf->mclass == PROC_REQ )
	        {
                possible_out_vcs[0] = 2;//rand() % 2;
            }
	        else
	        {
                possible_out_vcs[0] = 3;// + rand() % 3;
	        }
        }//*   
        else
        {
            if ( hf->mclass == PROC_REQ)
                possible_out_vcs[0] = 0;
            else
                possible_out_vcs[0] = 1;
        }
        //*/
        return;
    } 
    cout << "ERROR: dint return yet " << endl;

    return;
}


void
GenericRC::route_twonode(HeadFlit* hf)
{
    if ( node_id == 0 )
    {
        if ( hf->dst_node == 1)
            possible_out_ports.push_back(1);
        else
            possible_out_ports.push_back(0);
    }

    if ( node_id == 1 )
    {
        if ( hf->dst_node == 0)
            possible_out_ports.push_back(1);
        else
            possible_out_ports.push_back(0);
    }

    if ( hf->mclass == PROC_REQ )
        possible_out_vcs.push_back(0);
    else
        possible_out_vcs.push_back(1);

    return;
}

void
GenericRC::push (Flit* f, uint ch )
{
    if(ch > addresses.size())
        std::cout << "Invalid VC Exception " << std::endl;


    //Route the head
    if( f->type == HEAD )
    {
        HeadFlit* header = static_cast< HeadFlit* >( f );
        addresses[ch].last_adaptive_port = 0;
        addresses[ch].possible_out_ports.clear();
        addresses[ch].possible_out_vcs.clear();
        possible_out_ports.clear();
        possible_out_vcs.clear();
        addresses[ch].last_adaptive_port = 0;

        if( rc_method == RING_ROUTING)
        {
            possible_out_ports.clear();
            possible_out_vcs.clear();
            route_ring( header );
            addresses[ch].out_port = possible_out_ports.at(0);
            addresses[ch].possible_out_vcs.push_back(possible_out_vcs.at(0));
            addresses[ch].possible_out_ports.push_back(possible_out_ports.at(0));

        }
        if( rc_method == TWONODE_ROUTING)
        {
            possible_out_ports.clear();
            possible_out_vcs.clear();
            route_twonode( header );
            addresses[ch].out_port = possible_out_ports.at(0);
            addresses[ch].possible_out_vcs.push_back(possible_out_vcs.at(0));
            addresses[ch].possible_out_ports.push_back(possible_out_ports.at(0));
        }
        if( rc_method == XY_ROUTING)
        {
            addresses [ch].out_port = route_x_y(header->dst_node );
            addresses[ch].possible_out_vcs.push_back(0);
            addresses [ch].possible_out_ports.push_back(route_x_y(header->dst_node));

        }
        if( rc_method == XY_ROUTING_HETERO)
        {
            route_x_y_hetero(header);
            addresses [ch].out_port = possible_out_ports.at(0);
            addresses[ch].possible_out_vcs.push_back(0);
            addresses [ch].possible_out_ports.push_back(possible_out_ports.at(0));

        }
        if( rc_method == TORUS_ROUTING)
        {
            possible_out_ports.clear();
            possible_out_vcs.clear();
            route_torus( header );
            addresses[ch].out_port = possible_out_ports.at(0);
            addresses[ch].possible_out_vcs.push_back(possible_out_vcs.at(0));
            addresses[ch].possible_out_ports.push_back(possible_out_ports.at(0));
            assert ( possible_out_ports.size() == 1);
            assert ( possible_out_vcs.size() == 1);
        }


        addresses [ch].route_valid = true;

    }
    else if(f->type == TAIL)
    {
        if( !addresses[ch].route_valid)
        {
            printf("TAIL InvalidAddrException" );
        }

        addresses[ch].route_valid = false;
        addresses[ch].possible_out_ports.clear();
        addresses[ch].possible_out_vcs.clear();
        addresses[ch].last_adaptive_port = 0;
        possible_out_ports.clear();
        possible_out_vcs.clear();
    }
    else if (f->type == BODY)
    {
        if( !addresses[ch].route_valid)
        {
            printf("BODY InvalidAddrException" );
        }
    }
    else
    {
        printf(" InvalidFlitException fty: %d", f->type);
    }

    return ;
}		/* -----  end of method genericRC::push  ----- */

uint
GenericRC::get_output_port ( uint ch)
{
    uint oport = -1;
    if (addresses[ch].last_adaptive_port == addresses[ch].possible_out_ports.size())
        addresses[ch].last_adaptive_port = 0;
    oport  = addresses[ch].possible_out_ports[addresses[ch].last_adaptive_port];
    addresses[ch].last_adaptive_port++;

    return oport;
}		/* -----  end of method genericRC::get_output_port  ----- */

uint
GenericRC::no_adaptive_vcs( uint ch )
{
    return addresses[ch].possible_out_vcs.size();
}

uint
GenericRC::no_adaptive_ports( uint ch )
{
    return addresses[ch].possible_out_ports.size();
}

uint
GenericRC::get_virtual_channel ( uint ch )
{
    uint och = -1;
    if (addresses[ch].last_vc == addresses[ch].possible_out_vcs.size())
        addresses[ch].last_vc = 0;
    
    och = addresses[ch].possible_out_vcs[addresses[ch].last_vc];
    addresses[ch].last_vc++;

    return och;
}		/* -----  end of method genericRC::get_vc  ----- */

void
GenericRC::resize ( uint ch )
{
    vcs = ch;
    addresses.resize(ch);
    for ( uint i = 0 ; i<ch ; i++ )
    {
        addresses[i].route_valid = false;
        addresses[i].last_vc = 0;
    }
    return ;
}		/* -----  end of method genericRC::set_no_channels  ----- */

bool
GenericRC::is_empty ()
{
    uint channels = addresses.size();
    for ( uint i=0 ; i<channels ; i++ )
        if(addresses[i].route_valid)
            return false;

    return true;
}		/* -----  end of method genericRC::is_empty  ----- */

std::string
GenericRC::toString () const
{
    std::stringstream str;
    str << "GenericRC"
        << "\tchannels: " << addresses.size();
    return str.str();
}		/* -----  end of function GenericRC::toString  ----- */
#endif   /* ----- #ifndef _genericaddressdecoder_cc_INC  ----- */

