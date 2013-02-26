// Implementation of Link objects for Manifold
// George F. Riley, (and others) Georgia Tech, Fall 2010

#ifndef __LINK_IMPL_H__
#define __LINK_IMPL_H__

#include <iostream>
#include <stdint.h>
#include <vector>
#include <assert.h>

#include "common-defs.h"
#include "link-decl.h"
#include "manifold-decl.h"
#ifndef NO_MPI
#include "messenger.h"
#endif

namespace manifold {
namespace kernel {

template <typename T1, typename OBJ>
  void LinkOutput<T1, OBJ>::ScheduleRxEvent()
{
  if (LinkOutputBase<T1>::timed)
    { // Timed link, use the timed schedule
      Manifold::ScheduleTime(LinkOutputBase<T1>::timeLatency, handler, obj,
                         LinkOutputBase<T1>::inputIndex,
                         LinkOutputBase<T1>::data);
    }
  else if (LinkOutputBase<T1>::half)
    { // Schedule on half ticks
      Manifold::ScheduleClockHalf(
                                  LinkOutputBase<T1>::latency,
                                  *LinkOutputBase<T1>::clock,
                                  handler, obj, 
                                  LinkOutputBase<T1>::inputIndex,
                                  LinkOutputBase<T1>::data);
    }
  else
    { // Schedule on ticks
      Manifold::ScheduleClock(
                              LinkOutputBase<T1>::latency,
                              *LinkOutputBase<T1>::clock,
                              handler, obj, 
                              LinkOutputBase<T1>::inputIndex,
                              LinkOutputBase<T1>::data);
    }
  }


#ifndef NO_MPI
template<typename T>
LinkOutputRemote<T>::LinkOutputRemote(Link<T>* link, int compIdx,
            int inputIndex, Clock* c, Ticks_t latency, Time_t d, 
            bool isT, bool isH)
   :   LinkOutputBase<T>(latency, inputIndex, c, d, isT, isH),
    compIndex(compIdx), pLink(link)
{
  dest = Component :: GetComponentLP(compIdx);
}

//The following 2 templated Serialize() functions are created to solve a compiler
//problem occurring when the data sent between 2 components is a pointer type,
//such as MyType*. In this case, calling T :: Serialize() directly in ScheduleRxEvent()
//below would cause a compile problem.
template <typename T>
int Serialize(T data, unsigned char** d)
{
    return T :: Serialize(data, d);
}

template <typename T>
int Serialize(T* data, unsigned char** d)
{
    return T :: Serialize(*data, *d);
}

template <typename T>
void LinkOutputRemote<T>::ScheduleRxEvent()
{
  //unsigned char d[MAX_DATA_SIZE];
  //Cannot call T :: Serialize(this->data, d), because if T is a pointer type such as
  //MyType*, then compiler would complain Serialize() is not a member of MyType*.
  //Therefore, we created 2 template functions above to solve this problem.
  unsigned char* d;

  int len = Serialize(this->data, &d);
  if(this->timed) {
    TheMessenger.send_serial_msg(dest, compIndex, this->inputIndex, Manifold::Now(),
	                 Manifold::Now() + this->timeLatency, d, len);
  }
  else if(this->half) {
    TheMessenger.send_serial_msg(dest, compIndex, this->inputIndex, Manifold::NowHalfTicks(*(this->clock)),
	                 Manifold::NowHalfTicks(*(this->clock)) + this->latency, d, len);
  }
  else {
    TheMessenger.send_serial_msg(dest, compIndex, this->inputIndex, Manifold::NowTicks(*(this->clock)),
	                 Manifold::NowTicks(*(this->clock)) + this->latency, d, len);
  }
}


//! specialization for uint32_t
template <>
void LinkOutputRemote<uint32_t>::ScheduleRxEvent();

//! specialization for uint64_t
template <>
void LinkOutputRemote<uint64_t>::ScheduleRxEvent();

//####### must provide specialization for char, unsigned char, int, etc.


template<typename T>
void LinkInputBase :: Recv(Ticks_t tick, Time_t time, T& data)
{
  LinkInputBaseT<T>* baseT = (LinkInputBaseT<T>*)(this); // downcast!!
  baseT->set_data(data);
  baseT->ScheduleRxEvent(tick, time);
}

template<>
void LinkInputBaseT<uint32_t> ::
Recv(Ticks_t tick, Time_t time, unsigned char* data, int len);

template<>
void LinkInputBaseT<uint64_t> ::
Recv(Ticks_t tick, Time_t time, unsigned char* data, int len);


template <typename T, typename OBJ>
void LinkInput<T, OBJ>::ScheduleRxEvent(Ticks_t tick, Time_t time)
{
    if(this->timed) {
        Manifold::ScheduleTime(time - Manifold::Now(), handler, obj, this->inputIndex, this->data);
    }
    else if(this->half) {
        Manifold::ScheduleClockHalf(tick - Manifold::NowHalfTicks(*(this->clock)), *(this->clock), handler, obj, this->inputIndex, this->data);
    }
    else {
        Manifold::ScheduleClock(tick - Manifold::NowTicks(*(this->clock)), *(this->clock), handler, obj, this->inputIndex, this->data);
    }
}
#endif //#ifndef NO_MPI

template<typename T>
void LinkBase::Send(T t)
{
  //Link<T>* pLink = dynamic_cast<Link<T>*>(this);
  // Figure this out later. the dynamic_cast should have worked
  Link<T>* pLink = (Link<T>*)(this);
  if (pLink == 0)
    {
      std::cout << "Oops!  Type mismatch sending on link" << std::endl;
    }
  else
    {
      for (size_t i = 0; i < pLink->outputs.size(); ++i)
        {
          pLink->outputs[i]->Send(t);
        }
    }
}

template<typename T>
void LinkBase::SendTick(T t, Ticks_t delay)
{
  //Link<T>* pLink = dynamic_cast<Link<T>*>(this);
  // Figure this out later. the dynamic_cast should have worked
  Link<T>* pLink = (Link<T>*)(this);
  if (pLink == 0)
    {
      std::cout << "Oops!  Type mismatch sending on link" << std::endl;
    }
  else
    {
      for (size_t i = 0; i < pLink->outputs.size(); ++i)
        {
          pLink->outputs[i]->SendTick(t, delay);
        }
    }
}


template <typename T>
template <typename O> void Link<T>::AddOutput(void(O::*f)(int, T), 
                                              O* o,
                                              Ticks_t l, int ii,
                                              Clock* c,
                                              Time_t delay,
                                              bool isTimed,
                                              bool isHalf)
{
  LinkOutputBase<T>* lo = 
    new LinkOutput<T, O>(this, f, o, l, ii, c, delay, isTimed, isHalf);
  outputs.push_back(lo);
}

#ifndef NO_MPI
template <typename T>
void Link<T>::AddOutputRemote(CompId_t compIdx, int inputIndex, Clock* c, Ticks_t l,
                              Time_t delay, bool isTimed, bool isHalf)
{
  LinkOutputBase<T>* lo = 
    new LinkOutputRemote<T>(this, compIdx, inputIndex, c, l, delay, isTimed, isHalf);
  outputs.push_back(lo);
}
#endif //#ifndef NO_MPI

} //namespace kernel
} //namespace manifold

#endif
