
#ifndef  LINKDATA_CC_INC
#define  LINKDATA_CC_INC

#include	"linkData.h"
#include        "flit.h"

LinkData::LinkData()
{
}

LinkData::~LinkData()
{
    f = NULL;
}

int
LinkData::Serialize (const LinkData& p,unsigned char* buf )
{
    int pos = 0;
    memcpy(buf+pos,&p.src, sizeof(uint)); pos+=sizeof(uint);
    memcpy(buf+pos,&p.vc, sizeof(uint)); pos+=sizeof(uint);
    memcpy(buf+pos,&p.type, sizeof(int)); pos+=sizeof(int);

    if ( p.type == FLIT)
    {
        memcpy(buf+pos,&p.f->type, sizeof(int)); pos+=sizeof(int);
#ifdef _DEBUG
        memcpy(buf+pos,&p.f->flit_id, sizeof(uint)); pos+=sizeof(uint);
#endif
        if ( p.f->type == HEAD )
        {
            const HeadFlit* hf = static_cast<const HeadFlit*>(p.f);
            memcpy(buf+pos,&hf->virtual_channel, sizeof(uint)); pos+=sizeof(uint);
            memcpy(buf+pos,&hf->pkt_length, sizeof(uint)); pos+=sizeof(uint);

            memcpy(buf+pos,&hf->src_node, sizeof(uint)); pos+=sizeof(uint);
            memcpy(buf+pos,&hf->dst_node, sizeof(uint)); pos+=sizeof(uint);
            memcpy(buf+pos,&hf->dst_component_id, sizeof(uint)); pos+=sizeof(uint);
            memcpy(buf+pos,&hf->address, sizeof(uint64_t)); pos+=sizeof(uint64_t);
            memcpy(buf+pos,&hf->mclass, sizeof(int)); pos+=sizeof(int);
            memcpy(buf+pos,&hf->packet_originated_time, sizeof(uint64_t)); pos+=sizeof(uint64_t);

            delete hf;
        }
        else if ( p.f->type == BODY )
        {
            const BodyFlit* bf = static_cast<const BodyFlit*>(p.f);
            memcpy(buf+pos,&bf->virtual_channel, sizeof(uint)); pos+=sizeof(uint);
            memcpy(buf+pos,&bf->pkt_length, sizeof(uint)); pos+=sizeof(uint);

            delete bf;
        }
        else if ( p.f->type == TAIL )
        {
            const TailFlit* tf = static_cast<const TailFlit*>(p.f);
            memcpy(buf+pos,&tf->virtual_channel, sizeof(uint)); pos+=sizeof(uint);
            memcpy(buf+pos,&tf->pkt_length, sizeof(uint)); pos+=sizeof(uint);

            memcpy(buf+pos,&tf->sent_time, sizeof(uint64_t)); pos+=sizeof(uint64_t);
            memcpy(buf+pos,&tf->enter_network_time, sizeof(uint64_t)); pos+=sizeof(uint64_t);

            delete tf;
        }
        else
        {
            cout << " ERROR Invalid flit type " << p.f->type << endl;
            cout.flush();
            //MPI_Abort(MPI_COMM_WORLD,1);
            exit(1);
        }
    }

    delete &p;
    return pos;
}

LinkData*
LinkData::Deserialize ( const unsigned char* data )
{ 
    LinkData* ld = new LinkData();
    int pos=0;
    memcpy(&ld->src,data, sizeof(uint)); pos+=sizeof(uint);
    memcpy(&ld->vc,data+pos, sizeof(uint)); pos+=sizeof(uint);
    memcpy(&ld->type,data+pos, sizeof(int)); pos+=sizeof(int);

    if( ld->type == FLIT)
    {
        int ty;
        memcpy(&ty,data+pos, sizeof(int)); pos+=sizeof(int);

        switch ( ty )
        {
            case 1:
                {
                    HeadFlit* hf = new HeadFlit();
#ifdef _DEBUG
                    memcpy(&hf->flit_id,data+pos, sizeof(uint)); pos+=sizeof(uint);
#endif
                    memcpy(&hf->virtual_channel,data+pos, sizeof(uint)); pos+=sizeof(uint);
                    memcpy(&hf->pkt_length,data+pos, sizeof(uint)); pos+=sizeof(uint);
                    memcpy(&hf->src_node,data+pos, sizeof(uint)); pos+=sizeof(uint);
                    memcpy(&hf->dst_node,data+pos, sizeof(uint)); pos+=sizeof(uint);
                    memcpy(&hf->dst_component_id,data+pos, sizeof(uint)); pos+=sizeof(uint);
                    memcpy(&hf->address,data+pos, sizeof(uint64_t)); pos+=sizeof(uint64_t);
                    memcpy(&hf->mclass,data+pos, sizeof(int)); pos+=sizeof(int);
                    memcpy(&hf->packet_originated_time,data+pos, sizeof(uint64_t)); pos+=sizeof(uint64_t);
                    ld->f=hf;
                    break;
                }
            case 2:
                {
                    BodyFlit* bf = new BodyFlit();
#ifdef _DEBUG
                    memcpy(&bf->flit_id,data+pos, sizeof(uint)); pos+=sizeof(uint);
#endif
                    memcpy(&bf->virtual_channel,data+pos, sizeof(uint)); pos+=sizeof(uint);
                    memcpy(&bf->pkt_length,data+pos, sizeof(uint)); pos+=sizeof(uint);
                    ld->f=bf;
                    break;
                }
            case 3:
                {
                    TailFlit* tf = new TailFlit();
#ifdef _DEBUG
                    memcpy(&tf->flit_id,data+pos, sizeof(uint)); pos+=sizeof(uint);
#endif
                    memcpy(&tf->virtual_channel,data+pos, sizeof(uint)); pos+=sizeof(uint);
                    memcpy(&tf->pkt_length,data+pos, sizeof(uint)); pos+=sizeof(uint);

                    memcpy(&tf->sent_time,data+pos, sizeof(uint64_t)); pos+=sizeof(uint64_t);
                    memcpy(&tf->enter_network_time,data+pos, sizeof(uint64_t)); pos+=sizeof(uint64_t);
                    ld->f=tf;
                    break;
                }
            default:
                {

                    cout	<< " Unk flit type in deser " << ty << endl;
                    //MPI_Abort(MPI_COMM_WORLD,1);
                    exit(1);
                }
        }
    }

    return ld; 
}

LinkData 
LinkData::Deserialize(const unsigned char* buf, int len)
{
    // Not using this
    std::cerr << " Deser not implemented LinkData" << std::endl;
    exit(1);

    return *(new LinkData()); // just so that it compiles
}

std::string 
LinkData::toString(void) const
{
    std::stringstream str;
    str 
        << " vc: " <<vc 
        << " src_id: " <<src
        << " type: " << type
        ;
    return str.str();
}

/* 
   LinkData
   LinkData::operator=(const LinkData& p )
   {
   LinkData* ld2 = new LinkData();
   ld2->src = p.src;
   ld2->vc = p.vc;
   ld2->type = p.type;

   if ( p.type == FLIT)
   {
   ld2->f = p.f;
   }

   return *ld2;
   }
 * */

#endif   /* ----- #ifndef LINKDATA_CC_INC  ----- */
