#ifndef  _GENERICSWITCHARBITER_CC_INC
#define  _GENERICSWITCHARBITER_CC_INC

#include	"genericSwitchArbiter.h"

GenericSwitchArbiter::GenericSwitchArbiter()
{
    name = "swa";
}

GenericSwitchArbiter::~GenericSwitchArbiter()
{
}

void
GenericSwitchArbiter::resize(uint p, uint v)
{
    ports = p;
    vcs = v;
    requested.resize(ports);
    priority_reqs.resize(ports);
    last_port_winner.resize(ports);
    requesting_inputs.resize(ports);
    last_winner.resize(ports);

    for ( uint i=0; i<ports; i++)
    {
        requested[i].resize(ports*vcs);
        requesting_inputs[i].resize(ports*vcs);
        last_winner[i].win_cycle = 0;
    }

    for ( uint i=0; i<ports; i++)
        for ( uint j=0; j<(ports*vcs); j++)
        {
            requested[i][j]=false;
        }

    for ( uint i=0; i<ports; i++)
        {
            last_port_winner[i] = 0;
        }

}

bool
GenericSwitchArbiter::is_requested( uint oport, uint inport, uint och )
{
    if( oport >= ports || inport >= ports || och >= vcs)
    {
        std::cout << " Error in SWA oport: "<< oport <<" inp: " << inport <<" och: " << och << std::endl;
        exit(1);
    }
    return requested[oport][inport*vcs+och];
}

void
GenericSwitchArbiter::request(uint oport, uint ovc, uint inport, uint ivc )
{
    requested[oport][inport*vcs+ovc] = true;
    requesting_inputs[oport][inport*vcs+ovc].port = inport;
    requesting_inputs[oport][inport*vcs+ovc].ch=ivc;
    requesting_inputs[oport][inport*vcs+ovc].in_time = manifold::kernel::Manifold::NowTicks();
    return;
}

SA_unit
GenericSwitchArbiter::pick_winner( uint oport)
{
            return do_round_robin_arbitration(oport);
}

SA_unit
GenericSwitchArbiter::do_round_robin_arbitration( uint oport)
{

    if( last_winner[oport].win_cycle >= manifold::kernel::Manifold::NowTicks())
        return last_winner[oport];

    /* Now look at contesting input ports on this channel and pick
     * a winner*/
    bool winner_found = false;
    for( uint i=last_port_winner[oport]+1; i<(ports*vcs); i++)
    {
        if(requested[oport][i])
        {
            last_port_winner[oport] = i;
            winner_found = true;
            last_winner[oport].port = requesting_inputs[oport][i].port;
            last_winner[oport].ch= requesting_inputs[oport][i].ch;
            last_winner[oport].win_cycle= manifold::kernel::Manifold::NowTicks();
            return last_winner[oport];
        }
    }


    if(!winner_found)
        for( uint i=0; i<=last_port_winner[oport]; i++)
        {
            if(requested[oport][i])
            {
                last_port_winner[oport] = i;
                winner_found = true;
                last_winner[oport].port = requesting_inputs[oport][i].port;
                last_winner[oport].ch= requesting_inputs[oport][i].ch;
                last_winner[oport].win_cycle= manifold::kernel::Manifold::Now();
                return last_winner[oport];
            }
        }
    if(!winner_found)
    {
        std::cerr << "ERROR: RR Cant find port winner" << std::endl;
        exit(1);
    }


    return last_winner[oport];
}

void
GenericSwitchArbiter::clear_requestor( uint oport, uint inport, uint och)
{
    requested[oport][inport*vcs+och] = false;
    return;
}


bool
GenericSwitchArbiter::is_empty()
{

    for( uint i=0; i<ports; i++)
        for( uint j=0; j<(ports*vcs); j++)
            if(requested[i][j] )
                return false;

    return true;

}

std::string
GenericSwitchArbiter::toString() const
{
    std::stringstream str;
    str << "GenericSwitchArbiter: matrix size "
        << "\t requested_qu row_size: " << requested.size();
    if( requested.size())
        str << " col_size: " << requested[0].size()
            ;
    return str.str();
}
#endif
