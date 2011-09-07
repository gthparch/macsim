// George F. Riley, (and others) Georgia Tech, Fall 2010

#include "link.h"

#ifndef NO_MPI

#include "messenger.h"

namespace manifold {
namespace kernel {

//! Template specialization for uint32_t.
template<>
void LinkOutputRemote<uint32_t> :: ScheduleRxEvent()
{
    if(timed) {
	TheMessenger.send_uint32_msg(dest, compIndex, inputIndex, Manifold::Now(),
	                 Manifold::Now() + timeLatency, data);
    }
    else if(half) {
	TheMessenger.send_uint32_msg(dest, compIndex, inputIndex, Manifold::NowHalfTicks(*(this->clock)),
	                 Manifold::NowHalfTicks(*(this->clock)) + latency, data);
    }
    else {
	TheMessenger.send_uint32_msg(dest, compIndex, inputIndex, Manifold::NowTicks(*(this->clock)),
	                 Manifold::NowTicks(*(this->clock)) + latency, data);
    }
}


//! Template specialization for uint64_t.
template<>
void LinkOutputRemote<uint64_t> :: ScheduleRxEvent()
{
    if(timed) {
	TheMessenger.send_uint64_msg(dest, compIndex, inputIndex, Manifold::Now(),
	                 Manifold::Now() + timeLatency, data);
    }
    else if(half) {
	TheMessenger.send_uint64_msg(dest, compIndex, inputIndex, Manifold::NowHalfTicks(*(this->clock)),
	                 Manifold::NowHalfTicks(*(this->clock)) + latency, data);
    }
    else {
	TheMessenger.send_uint64_msg(dest, compIndex, inputIndex, Manifold::NowTicks(*(this->clock)),
	                 Manifold::NowTicks(*(this->clock)) + latency, data);
    }
}


template<>
void LinkInputBaseT<uint32_t> ::
Recv(Ticks_t tick, Time_t time, unsigned char* data, int len)
{
  assert(0); //This function should never be called.
}

template<>
void LinkInputBaseT<uint64_t> ::
Recv(Ticks_t tick, Time_t time, unsigned char* data, int len)
{
  assert(0); //This function should never be called.
}

} //namespace kernel
} //namespace manifold

#endif //#ifndef NO_MPI
