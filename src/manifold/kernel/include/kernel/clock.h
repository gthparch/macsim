/** @file clock.h
 *  Contains classes and functions related to clocks.
 */
 
// George F. Riley, (and others) Georgia Tech, Fall 2010

#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <iostream>
#include <list>
#include <map>
#include <vector>

#include "common-defs.h"
#include "manifold-decl.h"

namespace manifold {
namespace kernel {

/** Base class for objects keeping track of tick handlers.
 */
class tickObjBase 
{
 public:
 
 /** By default tick handlers are enabled
  */
 tickObjBase() : enabled(true) {}
 
 /** Virtual rising tick handler
  */
 virtual void CallRisingTick() = 0;
 
 /** Virtual falling tick handler
  */ 
 virtual void CallFallingTick() = 0;
 
 /** Enables tick handlers
  */
 void         Enable() {enabled = true;}
 
 /** Disables tick handlers.
  */
 void         Disable() {enabled = false;}
 
 /** Tick handler flag for responding to clock ticks
  */
 bool         enabled;
};

/** Object to keep track of tick handlers.
 */
template <typename OBJ>
class tickObj : public tickObjBase {
 public:

 /** @arg \c o Object registered with the clock.
  */
 tickObj(OBJ* o) : obj(o), risingFunct(0), fallingFunct(0) {}
 
 /** @arg \c o Object registered with the clock.
  *  @arg \c r Pointer to rising function that will be called on tick.
  *  @arg \c f Pointer to falling function that will be called on tick.
  */
 tickObj(OBJ* o, void (OBJ::*r)(void), void (OBJ::*f)(void)) 
  : obj(o), risingFunct(r), fallingFunct(f)
  {
  }
  
 /** Calls rising clk callback function on object registered with clock.
  */ 
 void CallRisingTick() 
  {
    if (risingFunct) {
      (obj->*risingFunct)();
    }
  }
  
 /** Calls falling clk callback function on object registered with clock.
  */ 
 void CallFallingTick() 
  {
    if (fallingFunct)
      {
        (obj->*fallingFunct)();
      }
  }
 
 /** Object registered with clock.
  */  
 OBJ* obj;
 
 /** Pointer to rising function that will be called on tick.
  */
 void (OBJ::*risingFunct)(void);
 
 /** Pointer to falling function that will be called on tick.
  */
 void (OBJ::*fallingFunct)(void);
};

#define CLOCK_CALENDAR_LENGTH 128

/** \class Clock A clock object that components can register to.
 *
 *  The clock object contains:
 *  1) List of objects requiring the Rising and Falling callbacks
 *  2) List of future events that are scheduled using the "ticks"
 *     time instead of floating point time.  This is implemented
 *     using a calendar queue for any event less than 
 *     CLOCK_CALENDER_LENGTH ticks in the future, and a simple sorted
 *     list (map) of events scheduled more than that length in the future.
 */
class Clock
{
 public:
 
  /** @arg Frequency(hz) of the clock
   */
  Clock(double);
  
  /** Processes all rising edge callbacks that have been registered with the clock.
   */
  void Rising();

  /** Processes all falling edge callbacks that have been registered with the clock.
   */  
  void Falling();
  
  /** Inserts an event to be processed
   *  @arg The time specified in the TickEvent is ticks in the future
   *  /return The Id of the event is returned.
   */
  TickEventId Insert(TickEventBase*);

  /** Inserts an event to be processed
   *  @arg The time specified in the TickEvent is half-ticks in the future
   *  \return The Id of the event is returned.
   */  
  TickEventId InsertHalf(TickEventBase*);
  
  /** Cancels an event
   *  @arg The event with the specified Id is cancelled.
   */
  void        Cancel(TickEventId);
  
  /** Returns floating point time of the next tick. 
   */
  Time_t      NextTickTime() const;
  
  /** Returns the current tick counter
   */
  Ticks_t     NowTicks() const;
  
  /** Returns the current tick counter in half ticks
   */
  Ticks_t     NowHalfTicks() const;
  
  /** Processes all events in the current tick
   */
  void        ProcessThisTick();

 public:
  
  /** Period of the clock (seconds)
   */
  double  period;
  
  /** Frequency of the clock (hz)
   */
  double  freq;
  
  /** The rising edge is the "tick", the falling is one-half tick later
   *  True if next is rising edge
   */
  bool    nextRising;       
  
  /** The rising edge is the "tick", the falling is one-half tick later
   *  Tick count of next tick
   */   
  Ticks_t nextTick;
  
  /** Returns a reference to the master clock
   */                  
  static  Clock& Master();
  
  /** Returns the time for the master clock
   */
  static  Ticks_t Now();
  
  /** Returns the time for the master clock
   */
  static  Ticks_t NowHalf();
  
  /** Returns the time from the specified clock
   */
  static  Ticks_t Now(Clock&);
  
  /** Returns the time from the specified clock
   */
  static  Ticks_t NowHalf(Clock&);
  
  /** Register an object with the specified clock object
   *  Uses "master" clock
   * @arg \c obj A pointer to the component to register with the clock.
   * @arg \c rising A pointer to the member function that will be called on a 
   * rising edge.  This function must not take any arguments or return any
   * values.
   * @arg \c falling A pointer to the member function that will be called on a 
   * falling edge.  This function must not take any arguments or return any 
   * values.
   * \return Returns a tickObjBase ptr used to disable/enable tick handlers.
   */ 
  template <typename O> static tickObjBase* Register(
                                                     O*          obj,
                                                     void(O::*rising)(void), 
                                                     void(O::*falling)(void));
  /** Register an object with the specified clock object
   *  Uses specified clock
   * @arg \c whichClock Specified clock to register to.
   * @arg \c obj A pointer to the component to register with the clock.
   * @arg \c rising A pointer to the member function that will be called on a 
   * rising edge.  This function must not take any arguments or return any
   * values.
   * @arg \c falling A pointer to the member function that will be called on a 
   * falling edge.  This function must not take any arguments or return any 
   * values.
   * \return Returns a tickObjBase ptr used to disable/enable tick handlers.   
   */    
  template <typename O> static tickObjBase* Register(
                                                     Clock&  whichClock,
                                                     O*      obj,
                                                     void(O::*rising)(void), 
                                                     void(O::*falling)(void));
  /** Typedefs for the calendar queue
   */
  typedef std::vector<TickEventBase*> EventVec_t;

  /** Typedefs for the calendar queue
   */
  typedef std::vector<EventVec_t>     EventVecVec_t;

  /** To account for events "too far" in the future, we do need
   * a sorted (multimap) for those
   */
  typedef std::multimap<Ticks_t, TickEventBase*> EventMap_t;
  
  /** Design just needs an unsorted container (vector) of clocks
   */
  typedef std::vector<Clock*> ClockVec_t;
private:
  
  /** Stores the actual calendar queue 
   */
  EventVecVec_t calendar;
  
  /** Stores events outside the calendar queue
   */   
  EventMap_t    events;
  
  /** Holds the list of handlers that have been registered
   */
  std::list<tickObjBase*> tickObjs;

  /** Stores the vector of clock objects
   */
  static ClockVec_t* clocks;
public:
  
  /** Returns the vector of clock objects
   */
  static ClockVec_t& GetClocks();

#ifdef KERNEL_UTEST
public:
  std::list<tickObjBase*>& getRegistered() { return tickObjs; }
  void unregisterAll() { tickObjs.clear(); }
  const EventVecVec_t& getCalendar() const { return calendar; }
  const EventMap_t& getEventMap() const { return events; }
#endif

};


template <typename O> tickObjBase* Clock::Register(
                                                   O*          obj,
                                                   void(O::*rising)(void), 
                                                   void(O::*falling)(void))
{
  // Get the clock registered for this period
  Clock& c = Master();
  return Register(c, obj, rising, falling);
}

template <typename O> tickObjBase* Clock::Register(
                                                   Clock&      c,
                                                   O*          obj,
                                                   void(O::*rising)(void), 
                                                   void(O::*falling)(void))
{
  tickObj<O>* t = new tickObj<O>(obj, rising, falling);
  c.tickObjs.push_back(t);
  return t;
}

} //namespace kernel
} //namespace manifold


#endif
