#ifndef  FLIT_CC_INC
#define  FLIT_CC_INC

#include	"flit.h"

// Phit class function implementations
Phit::Phit()
{
    ft = UNK;
}

Phit::~Phit()
{

}

/* 
 * Flit class implementation
 */
Flit::Flit()
{
    type = UNK;
    virtual_channel = -1;
    no_phits = 0;
    pkt_length = 0;
}

Flit::~Flit()
{

}

string
Flit::toString ( ) const
{
    stringstream str;
    str <<
        " type: " << type <<
        " vc: " << virtual_channel <<
        " no_phits: " << no_phits <<
        " pkt_length: " << pkt_length <<
#ifdef _DEBUG
        " flit_id: " << flit_id <<
#endif
        " ";
    return str.str();
}		/* -----  end of method Flit::toString  ----- */

/* 
 * HeadFlit class implementation
 */
HeadFlit::HeadFlit()
{
    type =HEAD;
}

HeadFlit::~HeadFlit()
{
}

string
HeadFlit::toString () const
{
    stringstream str;
    str <<
        " srcn: " << src_node <<
        " dstn: " << dst_node <<
        " dst_compid: " << dst_component_id <<
        " addr: " << hex << address << dec <<
        " class: " << mclass <<
        " ";
    str << static_cast<const Flit*>(this)->toString();
    return str.str();
}		/* -----  end of method HeadFlit::toString  ----- */

void
HeadFlit::populate_head_flit(void)
{
    /* May want to construct control bit streams etc here */
}

/* 
 * BodyFlit class implementation
 */
BodyFlit::BodyFlit()
{
    type =BODY;
}

BodyFlit::~BodyFlit()
{
}

string
BodyFlit::toString () const
{
    stringstream str;
    str << " BODY "
        << endl;
    return str.str();
}		/* -----  end of method BodyFlit::toString  ----- */

void
BodyFlit::populate_body_flit(void)
{
    /* May want to construct control bit streams etc here */
}

/* 
 * TailFlit class implementation
 */
TailFlit::TailFlit()
{
    type =TAIL ;
}

TailFlit::~TailFlit()
{
}

string
TailFlit::toString () const
{
    stringstream str;
    str << "TAIL"
        << endl;
    return str.str();
}		/* -----  end of method TailFlit::toString  ----- */

void
TailFlit::populate_tail_flit(void)
{
    /* May want to construct control bit streams etc here */
}

/* 
 * FlitLevelPacket class implementation
 */
FlitLevelPacket::FlitLevelPacket()
{
}

FlitLevelPacket::~FlitLevelPacket()
{
}

void
FlitLevelPacket::add ( Flit* f )
{
    switch ( f->type  ) {
        case HEAD:	
            {
                this->pkt_length = f->pkt_length;
                this->virtual_channel = f->virtual_channel;

                HeadFlit* hf = static_cast< HeadFlit* >(f);
                this->src_node = hf->src_node;
                this->dst_node = hf->dst_node;
                this->dst_component_id = hf->dst_component_id;
                this->address = hf->address;
                this->mc = hf->mclass;
                this->req = hf->req;

                delete hf;
                break;
            }

        case BODY:	
            {
                BodyFlit* bf = static_cast< BodyFlit* >(f);
                delete bf;
                break;
            }

        case TAIL:	
            {
                TailFlit* tf = static_cast< TailFlit* >(f);
                this->enter_network_time = tf->enter_network_time;
                delete tf;
                break;
            }

        default:	
            cerr << " ERROR: Flit type unk " << endl;
            exit(1);
            break;
    }
    return ;
}		/* -----  end of method FlitLevelPacket::add  ----- */

Flit* 
FlitLevelPacket::get_next_flit ( void )
{
    Flit* np = flits.front();
    flits.pop_front();
    return np;
}

uint
FlitLevelPacket::size ( void )
{
    return flits.size() ;
}		/* -----  end of method FlitLevelPacket::size  ----- */

bool
FlitLevelPacket::valid_packet ( void )
{
    if ( flits.size() )
        return ( flits.size() == flits[0]->pkt_length);
    else
        return false ;
}		/* -----  end of method FlitLevelPacket::valid_packet  ----- */

#endif   /* ----- #ifndef FLIT_CC_INC  ----- */
