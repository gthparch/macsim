
#ifndef  NETWORKPACKET_CC_INC
#define  NETWORKPACKET_CC_INC

#include	"networkPacket.h"
#include	"flit.h"

NetworkPacket::NetworkPacket()
{
//    address = 0;
    src_node = -1;
//    dst_node = -1;
}

NetworkPacket::~NetworkPacket()
{

}

bool 
NetworkPacket::operator==(const NetworkPacket& p)
{
//    return (address==p.address) && 
       return (src_node==p.src_node);
//        && (dst_node==p.dst_node);
}

//ostream &operator<<(ostream& stream, const NetworkPacket& pkt)
string
NetworkPacket::toString () const 
{
    stringstream str;
    str 
        << " src_node: " << hex << (uint)this->src_node << dec
        << " dst_node: " << (uint)this->dst_node 
        << " dst_compId: " << (uint)this->dst_component_id
        << " address: " << hex << (uint)this->address << dec
        << " pkt_length: " << this->pkt_length
        << " payload_length: " << this->payload_length 
        ;
    return str.str();
}

int
NetworkPacket:: Serialize(NetworkPacket p, unsigned char*buf)
{
    return 0;
}

NetworkPacket 
NetworkPacket:: Deserialize(const unsigned char* data, int len)
{
    NetworkPacket p;
  /*
    cout << "\nDSER LEN: " << len;

    int pos = 0;
    p.src_node |= ((int)data[pos++]);
    p.src_node |= ((int)(data[pos++])) << 8;
    p.src_node |= ((int)(data[pos++])) << 16;
    p.src_node |= ((int)(data[pos++])) << 24;
    */

    return p;
}

void
NetworkPacket::from_flit_level_packet(FlitLevelPacket* flp)
{
    src_node = flp->src_node;
    dst_node = flp->dst_node;
    dst_component_id = flp->dst_component_id;
    address = flp->address;
    mclass = flp->mc;
    proc_buffer_id = flp->virtual_channel;
    pkt_length = flp->pkt_length;
    enter_network_time = flp->enter_network_time;
    req = flp->req;
    // not copying payload and sent time 
}

void
NetworkPacket::to_flit_level_packet(FlitLevelPacket* flp, uint lw)
{
    /* find the number of flits */
    uint extra_flits = 0;
    uint no_bf = 0;
    switch ( mclass  ) {
        case PROC_REQ:
        case L1_REQ:
        case L2_REQ:
        case L3_REQ:
            extra_flits+=2;
            break;

        case MC_RESP:	
            extra_flits+=2;
            no_bf =  (uint)ceil ( this->payload_length/lw);
            break;

        default:	
            cout << " Type unk \n NetworkPacket::to_flit_level_packet" << endl;
            exit(1);
            break;
    }
    uint tot_flits = extra_flits + no_bf;  
    flp->pkt_length = tot_flits;
    flp->flits.resize(tot_flits);

    HeadFlit* hf = new HeadFlit();
    hf->pkt_length = tot_flits;
    hf->src_node = src_node;
    hf->dst_node = dst_node;
    hf->dst_component_id = dst_component_id;
    hf->address = address;
    hf->mclass = mclass;
    hf->req = req;
#ifdef _DEBUG
    MTRand mtrand1;
    hf->flit_id = mtrand1.randInt()%200;
#endif

    flp->flits[0] = hf;

    /* Request packets are short.. no body flits */
    for ( uint i=0; i<no_bf; i++)
    {
        BodyFlit* bf = new BodyFlit();
#ifdef _DEBUG
        MTRand mtrand1;
        bf->flit_id = mtrand1.randInt()%200;
#endif
        bf->type = BODY;
        bf->pkt_length = tot_flits;
        flp->flits[i+1] =bf;
    }
    //    if ( mclass != PROC_REQ )
    {
        TailFlit* tf = new TailFlit();
#ifdef _DEBUG
        MTRand mtrand1;
        tf->flit_id = mtrand1.randInt()%200;
#endif
        tf->type = TAIL;
        tf->pkt_length = tot_flits;
        tf->enter_network_time = enter_network_time;
        flp->flits[tot_flits-1] =tf;
    }

//    assert( flp->flits.size() > 0 && flp->flits.size() == flp->pkt_length );
//    assert( flp->flits.at(0)->type == HEAD && flp->flits.at(1)->type != HEAD );
}

#endif   /* ----- #ifndef NETWORKPACKET_CC_INC  ----- */

