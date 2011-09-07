// Contains the .h implementations of the Manifold Object
// We separate this because component needs manifold, and manifold
// needs component.  By separating the definition (manifold.h) from
// the implementation (this file), and the same for component.h
// and component-impl.h, we can make this work.
// George F. Riley, (and others) Georgia Tech, Fall 2010

#ifndef __MANIFOLD_IMPL_H__
#define __MANIFOLD_IMPL_H__

#include "manifold-decl.h"
#include "component-decl.h"
#include "clock.h"

#include <iostream>
#include <assert.h>

namespace manifold {
namespace kernel {

// Implementation of the templated Connect functions
// The first two specify delay in units of ticks for the default clock
/*
template <typename T>
LinkId_t Manifold::Connect(CompId_t sourceComponent, int sourceIndex,
                           CompId_t dstComponent,    int dstIndex,
                           void (T::*handler)(int,  uint64_t),
                           Ticks_t  latencyTicks)
{
  Clock& c = Clock::Master();
  Component* src = Component::GetComponent(sourceComponent);
  Component* dst = Component::GetComponent(dstComponent);
  // Insure not ghost; code later
  // The AddOutput call will create a new link if needed, or will
  // return existing.
  Link<uint64_t>* link = src->AddOutputLink<uint64_t>(sourceIndex);
  link->AddOutput<T>(handler, (T*)dst, latencyTicks, dstIndex, &c);
}
*/

template <typename T, typename T2>
void Manifold::Connect(CompId_t sourceComponent, int sourceIndex,
                           CompId_t dstComponent,    int dstIndex,
                           void (T::*handler)(int,  T2),
                           Ticks_t latencyTicks) throw (LinkTypeMismatchException)
{
  Clock& c = Clock::Master();
  Component* src = Component::GetComponent(sourceComponent);
  Component* dst = Component::GetComponent(dstComponent);
  LpId_t srcLP = Component::GetComponentLP(sourceComponent);
  LpId_t dstLP = Component::GetComponentLP(dstComponent);

  if(srcLP == Manifold :: GetRank()) {//source component in this LP
    // The AddOutput call will create a new link if needed, or will
    // return existing.
    Link<T2>* link = src->AddOutputLink<T2>(sourceIndex);

    if(dstLP == Manifold :: GetRank()) {
      link->AddOutput(handler, (T*)dst, latencyTicks, dstIndex, &c);
    }
    else {
#ifndef NO_MPI
      link->AddOutputRemote(dstComponent, dstIndex, &c, latencyTicks);
#endif
    }

  }
  else { //sourceComponent not in this LP
#ifndef NO_MPI
    if(dstLP == Manifold :: GetRank()) {
      dst->AddInput(dstIndex, handler, (T*)dst, &c);
    }
    else {
      //do nothing
    }
#endif
  }

}

// The next two specify delay in units of half-ticks for the default clock
/*
template <typename T>
LinkId_t Manifold::ConnectHalf(CompId_t sourceComponent, int sourceIndex,
                               CompId_t dstComponent,    int dstIndex,
                               void (T::*handler)(int,  uint64_t),
                               Ticks_t  latencyTicks)
{
  Clock& c = Clock::Master();
  Component* src = Component::GetComponent(sourceComponent);
  Component* dst = Component::GetComponent(dstComponent);
  // Insure not ghost; code later
  // The AddOutput call will create a new link if needed, or will
  // return existing.
  Link<uint64_t>* link = src->AddOutputLink<uint64_t>(sourceIndex);
  // On AddOutput, Time = 0, isTimed = false, isHalf = true
  link->AddOutput(handler, (T*)dst, latencyTicks, dstIndex, &c, 0, false, true);
}
*/

template <typename T, typename T2>
void Manifold::ConnectHalf(CompId_t sourceComponent, int sourceIndex,
                               CompId_t dstComponent,    int dstIndex,
                               void (T::*handler)(int,  T2),
                               Ticks_t latencyTicks) throw (LinkTypeMismatchException)
{
  Clock& c = Clock::Master();
  Component* src = Component::GetComponent(sourceComponent);
  Component* dst = Component::GetComponent(dstComponent);
  LpId_t srcLP = Component::GetComponentLP(sourceComponent);
  LpId_t dstLP = Component::GetComponentLP(dstComponent);

  if(srcLP == Manifold :: GetRank()) {//source component in this LP
    // The AddOutput call will create a new link if needed, or will
    // return existing.
    Link<T2>* link = src->AddOutputLink<T2>(sourceIndex);
    if(dstLP == Manifold :: GetRank()) {
      link->AddOutput(handler, (T*)dst, latencyTicks, dstIndex, &c, 0, false, true);
    }
    else {
#ifndef NO_MPI
      link->AddOutputRemote(dstComponent, dstIndex, &c, latencyTicks, 0, false, true);
#endif
    }

  }
  else { //sourceComponent not in this LP
#ifndef NO_MPI
    if(dstLP == Manifold :: GetRank()) {
      dst->AddInput(dstIndex, handler, (T*)dst, &c, false, true);
    }
    else {
      //do nothing
    }
#endif
  }

}

// The next two specify delay in units of ticks for the specified clock
/*
template <typename T>
LinkId_t Manifold::ConnectClock(CompId_t sourceComponent, int sourceIndex,
                                CompId_t dstComponent,    int dstIndex,
                                Clock& c,
                                void (T::*handler)(int,  uint64_t),
                                Ticks_t  latencyTicks)
{
  Component* src = Component::GetComponent(sourceComponent);
  Component* dst = Component::GetComponent(dstComponent);
  // Insure not ghost; code later
  // The AddOutput call will create a new link if needed, or will
  // return existing.
  Link<uint64_t>* link = src->AddOutputLink<uint64_t>(sourceIndex);
  // On AddOutput, Time = 0, isTimed = false, isHalf = false
  link->AddOutput(handler, (T*)dst, latencyTicks, dstIndex, &c, 0, false, false);
}
*/

template <typename T, typename T2>
void Manifold::ConnectClock(CompId_t sourceComponent, int sourceIndex,
                                CompId_t dstComponent,    int dstIndex,
                                Clock& c,
                                void (T::*handler)(int,  T2),
                                Ticks_t latencyTicks) throw (LinkTypeMismatchException)
{
  Component* src = Component::GetComponent(sourceComponent);
  Component* dst = Component::GetComponent(dstComponent);
  LpId_t srcLP = Component::GetComponentLP(sourceComponent);
  LpId_t dstLP = Component::GetComponentLP(dstComponent);

  if(srcLP == Manifold :: GetRank()) {//source component in this LP
    // The AddOutput call will create a new link if needed, or will
    // return existing.
    Link<T2>* link = src->AddOutputLink<T2>(sourceIndex);
    if(dstLP == Manifold :: GetRank()) {
      link->AddOutput(handler, (T*)dst, latencyTicks, dstIndex, &c);
    }
    else {
#ifndef NO_MPI
      link->AddOutputRemote(dstComponent, dstIndex, &c, latencyTicks);
#endif
    }

  }
  else { //sourceComponent not in this LP
#ifndef NO_MPI
    if(dstLP == Manifold :: GetRank()) {
      dst->AddInput(dstIndex, handler, (T*)dst, &c);
    }
    else {
      //do nothing
    }
#endif
  }

}

// The next two specify delay in units of half-ticks for the specified clock
/*
template <typename T>
LinkId_t Manifold::ConnectClockHalf(CompId_t sourceComponent, int sourceIndex,
                                    CompId_t dstComponent,    int dstIndex,
                                    Clock& c,
                                    void (T::*handler)(int,  uint64_t),
                                    Ticks_t  latencyTicks)
{
  Component* src = Component::GetComponent(sourceComponent);
  Component* dst = Component::GetComponent(dstComponent);
  // Insure not ghost; code later
  // The AddOutput call will create a new link if needed, or will
  // return existing.
  Link<uint64_t>* link = src->AddOutputLink<uint64_t>(sourceIndex);
  // On AddOutput, Time = 0, isTimed = false, isHalf = true
  link->AddOutput(handler, (T*)dst, latencyTicks, dstIndex, &c, 0, false, true);
}
*/

template <typename T, typename T2>
void Manifold::ConnectClockHalf(CompId_t sourceComponent, int sourceIndex,
                                    CompId_t dstComponent,    int dstIndex,
                                    Clock& c,
                                    void (T::*handler)(int,  T2),
                                    Ticks_t latencyTicks) throw (LinkTypeMismatchException)
{
  Component* src = Component::GetComponent(sourceComponent);
  Component* dst = Component::GetComponent(dstComponent);
  LpId_t srcLP = Component::GetComponentLP(sourceComponent);
  LpId_t dstLP = Component::GetComponentLP(dstComponent);

  if(srcLP == Manifold :: GetRank()) {//source component in this LP
    // The AddOutput call will create a new link if needed, or will
    // return existing.
    Link<T2>* link = src->AddOutputLink<T2>(sourceIndex);
    if(dstLP == Manifold :: GetRank()) {
      link->AddOutput(handler, (T*)dst, latencyTicks, dstIndex, &c, 0, false, true);
    }
    else {
#ifndef NO_MPI
      link->AddOutputRemote(dstComponent, dstIndex, &c, latencyTicks, 0, false, true);
#endif
    }

  }
  else { //sourceComponent not in this LP
#ifndef NO_MPI
    if(dstLP == Manifold :: GetRank()) {
      dst->AddInput(dstIndex, handler, (T*)dst, &c, false, true);
    }
    else {
      //do nothing
    }
#endif
  }
}

// The final two specify delay in units of time
/*
template <typename T>
LinkId_t Manifold::ConnectTime(CompId_t sourceComponent, int sourceIndex,
                               CompId_t dstComponent,    int dstIndex,
                               void (T::*handler)(int,  uint64_t),
                               Time_t  latency)
{
  Component* src = Component::GetComponent(sourceComponent);
  Component* dst = Component::GetComponent(dstComponent);
  // Insure not ghost; code later
  // The AddOutput call will create a new link if needed, or will
  // return existing.
  Link<uint64_t>* link = src->AddOutputLink<uint64_t>(sourceIndex);
  // On AddOutput, Time = latency, isTimed = true, isHalf = false
  link->AddOutput(handler, (T*)dst, 0, dstIndex, nil, latency, true, false);
}
*/

template <typename T, typename T2>
void Manifold::ConnectTime(CompId_t sourceComponent, int sourceIndex,
                               CompId_t dstComponent,    int dstIndex,
                               void (T::*handler)(int,  T2),
                               Time_t latency) throw (LinkTypeMismatchException)
{
  Component* src = Component::GetComponent(sourceComponent);
  Component* dst = Component::GetComponent(dstComponent);
  LpId_t srcLP = Component::GetComponentLP(sourceComponent);
  LpId_t dstLP = Component::GetComponentLP(dstComponent);

  if(srcLP == Manifold :: GetRank()) {//source component in this LP
    // The AddOutput call will create a new link if needed, or will
    // return existing.
    Link<T2>* link = src->AddOutputLink<T2>(sourceIndex);
    if(dstLP == Manifold :: GetRank()) {
      link->AddOutput(handler, (T*)dst, 0, dstIndex, nil, latency, true, false);
    }
    else {
#ifndef NO_MPI
      link->AddOutputRemote(dstComponent, dstIndex, nil, 0, latency, true, false);
#endif
    }

  }
  else { //sourceComponent not in this LP
#ifndef NO_MPI
    if(dstLP == Manifold :: GetRank()) {
      dst->AddInput(dstIndex, handler, (T*)dst, nil, true, false);
    }
    else {
      //do nothing
    }
#endif
  }

}


// Implementation of the templated scheduling functions
  // --------------------------------------------------------------------- //
  // This set are for "tick" events, that  are scheduled on integral ticks
  // with respect to the Master (first defined) clock
  template <typename T, typename OBJ>
     TickEventId Manifold::Schedule(Ticks_t t, void(T::*handler)(void), OBJ* obj)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent0<T, OBJ>(t, c, handler, obj);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1>
    TickEventId Manifold::Schedule(Ticks_t t, void(T::*handler)(U1), OBJ* obj, T1 t1)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent1<T, OBJ, U1, T1>(t, c, handler, obj, t1);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2>
    TickEventId Manifold::Schedule(Ticks_t t, void(T::*handler)(U1, U2), OBJ* obj, T1 t1, T2 t2)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent2<T, OBJ, U1, T1, U2, T2>(t, c, handler, obj, t1, t2);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3>
    TickEventId Manifold::Schedule(Ticks_t t, void(T::*handler)(U1, U2, U3), OBJ* obj, T1 t1, T2 t2, T3 t3)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent3<T, OBJ, U1, T1, U2, T2, U3, T3>(t, c, handler, obj, t1, t2, t3);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3,
    typename U4, typename T4>
     TickEventId Manifold::Schedule(Ticks_t t, void(T::*handler)(U1, U2, U3, U4), OBJ* obj, T1 t1, T2 t2, T3 t3, T4 t4)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent4<T, OBJ, U1, T1, U2, T2, U3, T3, U4, T4>(t, c, handler, obj, t1, t2, t3, t4);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  // --------------------------------------------------------------------- //
  // This set are for "tick" events, that  are scheduled on integral ticks
  // with respect to the Master (first defined) clock.  The ticks argument
  // is in units of half-ticks, allowing a mixing of rising and
  // falling edge events
  template <typename T, typename OBJ>
     TickEventId Manifold::ScheduleHalf(Ticks_t t, void(T::*handler)(void), OBJ* obj)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent0<T, OBJ>(t, c, handler, obj);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1>
    TickEventId Manifold::ScheduleHalf(Ticks_t t, void(T::*handler)(U1), OBJ* obj, T1 t1)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent1<T, OBJ, U1, T1>(t, c, handler, obj, t1);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2>
    TickEventId Manifold::ScheduleHalf(Ticks_t t, void(T::*handler)(U1, U2), OBJ* obj, T1 t1, T2 t2)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent2<T, OBJ, U1, T1, U2, T2>(t, c, handler, obj, t1, t2);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3>
    TickEventId Manifold::ScheduleHalf(Ticks_t t, void(T::*handler)(U1, U2, U3), OBJ* obj, T1 t1, T2 t2, T3 t3)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent3<T, OBJ, U1, T1, U2, T2, U3, T3>(t, c, handler, obj, t1, t2, t3);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3,
    typename U4, typename T4>
     TickEventId Manifold::ScheduleHalf(Ticks_t t, void(T::*handler)(U1, U2, U3, U4), OBJ* obj, T1 t1, T2 t2, T3 t3, T4 t4)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent4<T, OBJ, U1, T1, U2, T2, U3, T3, U4, T4>(t, c, handler, obj, t1, t2, t3, t4);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  // --------------------------------------------------------------------- //
  // This set are for "tick" events, that  are scheduled on integral ticks
  // with respect to the specified clock
  template <typename T, typename OBJ>
     TickEventId Manifold::ScheduleClock(Ticks_t t, Clock& c, void(T::*handler)(void), OBJ* obj)
  {
    TickEventBase* ev = new TickEvent0<T, OBJ>(t, c, handler, obj);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1>
    TickEventId Manifold::ScheduleClock(Ticks_t t, Clock& c, void(T::*handler)(U1), OBJ* obj, T1 t1)
  {
    TickEventBase* ev = new TickEvent1<T, OBJ, U1, T1>(t, c, handler, obj, t1);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2>
    TickEventId Manifold::ScheduleClock(Ticks_t t, Clock& c, void(T::*handler)(U1, U2), OBJ* obj, T1 t1, T2 t2)
  {
    TickEventBase* ev = new TickEvent2<T, OBJ, U1, T1, U2, T2>(t, c, handler, obj, t1, t2);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3>
    TickEventId Manifold::ScheduleClock(Ticks_t t, Clock& c, void(T::*handler)(U1, U2, U3), OBJ* obj, T1 t1, T2 t2, T3 t3)
  {
    TickEventBase* ev = new TickEvent3<T, OBJ, U1, T1, U2, T2, U3, T3>(t, c, handler, obj, t1, t2, t3);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3,
    typename U4, typename T4>
     TickEventId Manifold::ScheduleClock(Ticks_t t, Clock& c, void(T::*handler)(U1, U2, U3, U4), OBJ* obj, T1 t1, T2 t2, T3 t3, T4 t4)
  {
    TickEventBase* ev = new TickEvent4<T, OBJ, U1, T1, U2, T2, U3, T3, U4, T4>(t, c, handler, obj, t1, t2, t3, t4);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  // --------------------------------------------------------------------- //
  // This set are for "tick" events, that  are scheduled on integral half-ticks
  // with respect to the specified clock
  template <typename T, typename OBJ>
     TickEventId Manifold::ScheduleClockHalf(Ticks_t t, Clock& c, void(T::*handler)(void), OBJ* obj)
  {
    TickEventBase* ev = new TickEvent0<T, OBJ>(t, c, handler, obj);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1>
    TickEventId Manifold::ScheduleClockHalf(Ticks_t t, Clock& c, void(T::*handler)(U1), OBJ* obj, T1 t1)
  {
    TickEventBase* ev = new TickEvent1<T, OBJ, U1, T1>(t, c, handler, obj, t1);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2>
    TickEventId Manifold::ScheduleClockHalf(Ticks_t t, Clock& c, void(T::*handler)(U1, U2), OBJ* obj, T1 t1, T2 t2)
  {
    TickEventBase* ev = new TickEvent2<T, OBJ, U1, T1, U2, T2>(t, c, handler, obj, t1, t2);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3>
    TickEventId Manifold::ScheduleClockHalf(Ticks_t t, Clock& c, void(T::*handler)(U1, U2, U3), OBJ* obj, T1 t1, T2 t2, T3 t3)
  {
    TickEventBase* ev = new TickEvent3<T, OBJ, U1, T1, U2, T2, U3, T3>(t, c, handler, obj, t1, t2, t3);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3,
    typename U4, typename T4>
     TickEventId Manifold::ScheduleClockHalf(Ticks_t t, Clock& c, void(T::*handler)(U1, U2, U3, U4), OBJ* obj, T1 t1, T2 t2, T3 t3, T4 t4)
  {
    TickEventBase* ev = new TickEvent4<T, OBJ, U1, T1, U2, T2, U3, T3, U4, T4>(t, c, handler, obj, t1, t2, t3, t4);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

// ----------------------------------------------------------------------- //
// This set schedules floating point time with member function callback
  template <typename T, typename OBJ>
     EventId Manifold::ScheduleTime(double t, void(T::*handler)(void), OBJ* obj)
  {
    double future = t + Now();
    EventBase* ev = new Event0<T, OBJ>(future, handler, obj);
    events.insert(ev);
    return EventId(future, ev->uid);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1>
     EventId Manifold::ScheduleTime(double t, void(T::*handler)(U1), OBJ* obj, T1 t1)
  {
    double future = t + Now();
    EventBase* ev = new Event1<T, OBJ, U1, T1>(future, handler, obj, t1);
    events.insert(ev);
    return EventId(future, ev->uid);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2>
     EventId Manifold::ScheduleTime(double t, void(T::*handler)(U1, U2), OBJ* obj, T1 t1, T2 t2)
  {
    double future = t + Now();
    EventBase* ev = new Event2<T, OBJ, U1, T1, U2, T2>(future, handler, obj, t1, t2);
    events.insert(ev);
    return EventId(future, ev->uid);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3>
     EventId Manifold::ScheduleTime(double t, void(T::*handler)(U1, U2, U3), OBJ* obj, T1 t1, T2 t2, T3 t3)
  {
    double future = t + Now();
    EventBase* ev = new Event3<T, OBJ, U1, T1, U2, T2, U3, T3>(future, handler, obj, t1, t2, t3);
    events.insert(ev);
    return EventId(future, ev->uid);
  }

  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3,
    typename U4, typename T4>
     EventId Manifold::ScheduleTime(double t, void(T::*handler)(U1, U2, U3, U4), OBJ* obj, T1 t1, T2 t2, T3 t3, T4 t4)
  {
    double future = t + Now();
    EventBase* ev = new Event4<T, OBJ, U1, T1, U2, T2, U3, T3, U4, T4>(future, handler, obj, t1, t2, t3, t4);
    events.insert(ev);
    return EventId(future, ev->uid);
  }

// --------------------------------------------------------------------- //
// Manifold::Schedulers for static (non-member) callback functions
// with integer (ticks) time from the default clock.
#ifdef IMPLEMENTED_IN_MANIFOLD_CC
   TickEventId Manifold::Schedule(Ticks_t t, void(*handler)(void))
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent0Stat(t, c, handler);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }
#endif

  template <typename U1, typename T1>
     TickEventId Manifold::Schedule(Ticks_t t, void(*handler)(U1), T1 t1)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent1Stat<U1, T1>(t, c, handler, t1);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename U1, typename T1,
            typename U2, typename T2>
     TickEventId Manifold::Schedule(Ticks_t t, void(*handler)(U1, U2), T1 t1, T2 t2)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent2Stat<U1, T1, U2, T2>(t, c, handler, t1, t2);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3>
     TickEventId Manifold::Schedule(Ticks_t t, void(*handler)(U1, U2, U3), T1 t1, T2 t2, T3 t3)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent3Stat<U1, T1, U2, T2, U3, T3>(t, c, handler, t1, t2, t3);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3,
            typename U4, typename T4>
     TickEventId Manifold::Schedule(Ticks_t t, void(*handler)(U1, U2, U3, U4), T1 t1, T2 t2, T3 t3, T4 t4)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent4Stat<U1, T1, U2, T2, U3, T3, U4, T4>(t, c, handler, t1, t2, t3, t4);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

// --------------------------------------------------------------------- //
// Manifold::Schedulers for static (non-member) callback functions
// with integer (ticks) time from the default clock.
#ifdef IMPLEMENTED_IN_MANIFOLD_CC
   TickEventId Manifold::ScheduleHalf(Ticks_t t, void(*handler)(void))
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent0Stat(t, c, handler);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }
#endif

  template <typename U1, typename T1>
     TickEventId Manifold::ScheduleHalf(Ticks_t t, void(*handler)(U1), T1 t1)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent1Stat<U1, T1>(t, c, handler, t1);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename U1, typename T1,
            typename U2, typename T2>
     TickEventId Manifold::ScheduleHalf(Ticks_t t, void(*handler)(U1, U2), T1 t1, T2 t2)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent2Stat<U1, T1, U2, T2>(t, c, handler, t1, t2);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3>
     TickEventId Manifold::ScheduleHalf(Ticks_t t, void(*handler)(U1, U2, U3), T1 t1, T2 t2, T3 t3)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent3Stat<U1, T1, U2, T2, U3, T3>(t, c, handler, t1, t2, t3);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3,
            typename U4, typename T4>
     TickEventId Manifold::ScheduleHalf(Ticks_t t, void(*handler)(U1, U2, U3, U4), T1 t1, T2 t2, T3 t3, T4 t4)
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent4Stat<U1, T1, U2, T2, U3, T3, U4, T4>(t, c, handler, t1, t2, t3, t4);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

// --------------------------------------------------------------------- //
// Manifold::Schedulers for static (non-member) callback functions
// with integer (ticks) time from the specified clock
#ifdef IMPLEMENTED_IN_MANIFOLD_CC
TickEventId Manifold::ScheduleClock(Ticks_t t, Clock& c, void(*handler)(void))
  {
    TickEventBase* ev = new TickEvent0Stat(t, c, handler);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }
#endif

  template <typename U1, typename T1>
     TickEventId Manifold::ScheduleClock(Ticks_t t, Clock& c, void(*handler)(U1), T1 t1)
  {
    TickEventBase* ev = new TickEvent1Stat<U1, T1>(t, c, handler, t1);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename U1, typename T1,
            typename U2, typename T2>
     TickEventId Manifold::ScheduleClock(Ticks_t t, Clock& c, void(*handler)(U1, U2), T1 t1, T2 t2)
  {
    TickEventBase* ev = new TickEvent2Stat<U1, T1, U2, T2>(t, c, handler, t1, t2);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3>
     TickEventId Manifold::ScheduleClock(Ticks_t t, Clock& c, void(*handler)(U1, U2, U3), T1 t1, T2 t2, T3 t3)
  {
    TickEventBase* ev = new TickEvent3Stat<U1, T1, U2, T2, U3, T3>(t, c, handler, t1, t2, t3);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3,
            typename U4, typename T4>
     TickEventId Manifold::ScheduleClock(Ticks_t t, Clock& c, void(*handler)(U1, U2, U3, U4), T1 t1, T2 t2, T3 t3, T4 t4)
  {
    TickEventBase* ev = new TickEvent4Stat<U1, T1, U2, T2, U3, T3, U4, T4>(t, c, handler, t1, t2, t3, t4);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

// --------------------------------------------------------------------- //
// Manifold::Schedulers for static (non-member) callback functions
// with half-ticks time from the specified clock
#ifdef IMPLEMENTED_IN_MANIFOLD_CC
TickEventId Manifold::ScheduleClockHalf(Ticks_t t, Clock& c, void(*handler)(void))
  {
    TickEventBase* ev = new TickEvent0Stat(t, c, handler);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }
#endif

  template <typename U1, typename T1>
     TickEventId Manifold::ScheduleClockHalf(Ticks_t t, Clock& c, void(*handler)(U1), T1 t1)
  {
    TickEventBase* ev = new TickEvent1Stat<U1, T1>(t, c, handler, t1);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename U1, typename T1,
            typename U2, typename T2>
     TickEventId Manifold::ScheduleClockHalf(Ticks_t t, Clock& c, void(*handler)(U1, U2), T1 t1, T2 t2)
  {
    TickEventBase* ev = new TickEvent2Stat<U1, T1, U2, T2>(t, c, handler, t1, t2);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3>
     TickEventId Manifold::ScheduleClockHalf(Ticks_t t, Clock& c, void(*handler)(U1, U2, U3), T1 t1, T2 t2, T3 t3)
  {
    TickEventBase* ev = new TickEvent3Stat<U1, T1, U2, T2, U3, T3>(t, c, handler, t1, t2, t3);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3,
            typename U4, typename T4>
     TickEventId Manifold::ScheduleClockHalf(Ticks_t t, Clock& c, void(*handler)(U1, U2, U3, U4), T1 t1, T2 t2, T3 t3, T4 t4)
  {
    TickEventBase* ev = new TickEvent4Stat<U1, T1, U2, T2, U3, T3, U4, T4>(t, c, handler, t1, t2, t3, t4);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

// --------------------------------------------------------------------- //
// Manifold::Schedulers for static (non-member) callback functions
// with floating point time.
#ifdef IMPLEMENTED_IN_MANIFOLD_CC
   EventId Manifold::ScheduleTime(double t, void(*handler)(void))
  {
    double future = t + Now();
    EventBase* ev = new Event0Stat(future, handler);
    events.insert(ev);
    return EventId(future, ev->uid);
  }
#endif
  template <typename U1, typename T1>
     EventId Manifold::ScheduleTime(double t, void(*handler)(U1), T1 t1)
  {
    double future = t + Now();
    EventBase* ev = new Event1Stat<U1, T1>(future, handler, t1);
    events.insert(ev);
    return EventId(future, ev->uid);
  }

  template <typename U1, typename T1,
            typename U2, typename T2>
     EventId Manifold::ScheduleTime(double t, void(*handler)(U1, U2), T1 t1, T2 t2)
  {
    double future = t + Now();
    EventBase* ev = new Event2Stat<U1, T1, U2, T2>(future, handler, t1, t2);
    events.insert(ev);
    return EventId(future, ev->uid);
  }

  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3>
     EventId Manifold::ScheduleTime(double t, void(*handler)(U1, U2, U3), T1 t1, T2 t2, T3 t3)
  {
    double future = t + Now();
    EventBase* ev = new Event3Stat<U1, T1, U2, T2, U3, T3>(future, handler, t1, t2, t3);
    events.insert(ev);
    return EventId(future, ev->uid);
  }

  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3,
            typename U4, typename T4>
     EventId Manifold::ScheduleTime(double t, void(*handler)(U1, U2, U3, U4), T1 t1, T2 t2, T3 t3, T4 t4)
  {
    double future = t + Now();
    EventBase* ev = new Event4Stat<U1, T1, U2, T2, U3, T3, U4, T4>(future, handler, t1, t2, t3, t4);
    events.insert(ev);
    return EventId(future, ev->uid);
  }

} //namespace kernel
} //namespace manifold

#endif


