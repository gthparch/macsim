/** @file manifold-decl.h
 *  Manifold header
 *  This file contains the declarations (not the implementation)
 *  of the Manifold class.  The actual implementation is in 
 *  manifold.h. We split them apart since we have circular dependencies
 *  with component requiring manifold, and manifold requiring component.
 */
 
// George F. Riley, (and others) Georgia Tech, Fall 2010


#ifndef __MANIFOLD_DECL_H__
#define __MANIFOLD_DECL_H__

#include <iostream>
#include <set>
#include <map>
#include "common-defs.h"
#include "component-decl.h"

namespace manifold {
namespace kernel {

class Clock;

// First define the base class Event, then the events with up
// to four templated parameters.

/** Defines the base event class for tick events.
 */
class TickEventBase 
{
 public:
 
 /** Constructor
  *  @arg \c t Latency of event in ticks.
  *  @arg \c c Reference to the clock ticks are based on.
  */
 TickEventBase(Ticks_t t, Clock& c)
   : time(t), uid(nextUID++), clock(c), cancelled(false), rising(true) {}

 /** Constructor
  *  @arg \c t Latency of event in ticks
  *  @arg \c u Unique id of the event
  *  @arg \c c Reference to the clock ticks are based on.
  */  
 TickEventBase(Ticks_t t, int u, Clock& c) 
   : time(t), uid(u), clock(c), cancelled(false), rising(true) {}
   
  /** Virtual function, all subclasses must implement CallHandler
   */
  virtual void CallHandler() = 0;
  
 public:
 
  /** Timestamp (units of ticks) for the event
   */
  Ticks_t time;
  
  /** Each event has a unique identifier to break timestamp ties
   */
  int     uid;
  
  /** Clock reference for the tick event
   */
  Clock&  clock;
  
  /** True if cancelled before scheduling
   */
  bool    cancelled;
  
  /** True if rising edge event
   */
  bool    rising;     

 private:
 
  /** Holds the next unused unique event identifier
   */
  static int nextUID;
};

/** TickEventID class subclasses TickEventBase, and 
 *  is the return type from all schedule functions. 
 *  This is used to cancel the event.
 */
class TickEventId : public TickEventBase
{
public:
 
 /** Constructor
  *  @arg \c t Latency specified in ticks.
  *  @arg \c u Unique id for the event.
  *  @arg \c c Reference clock for ticks.
  */
 TickEventId(Ticks_t t, int u, Clock& c) : TickEventBase(t, u, c) {}
 
  /** Calls the callback function when the event is processed.
   */
  void CallHandler() {}
};

// ----------------------------------------------------------------------- //

/** TickEvent0 subclasses TickEventBase, it defines a 
 *  0 parameter member function CallHandler for a TickEvent
 */
template<typename T, typename OBJ>
class TickEvent0 : public TickEventBase
{
public:

  /** Constructor
   *  @arg \c t Latency in ticks.
   *  @arg \c c Reference clock used with tick latency.
   *  @arg \c f 0 Paramater call back function pointer for tick event
   *  @arg \c obj0 Object the callback is called on.
   */
  TickEvent0(Ticks_t t, Clock& c, void (T::*f)(void), OBJ* obj0)
    : TickEventBase(t,c), handler(f), obj(obj0){}
  
  /** Defines the callback handler function pointer
   */    
  void (T::*handler)(void);
  
  /** Object the CallHandler uses for the callback function
   */
  OBJ*      obj;
public:

  /** Calls the callback function when the event is processed.
   */
  void CallHandler();
};

template <typename T, typename OBJ>
void TickEvent0<T, OBJ>::CallHandler()
{
  (obj->*handler)();
}

/** TickEvent1 subclasses TickEventBase, it defines a 
 *  1 parameter member function CallHandler for a TickEvent
 */
template<typename T, typename OBJ, typename U1, typename T1>
class TickEvent1 : public TickEventBase
{
public:

  /** Constructor
   *  @arg \c t Latency in ticks.
   *  @arg \c c Reference clock used with tick latency.
   *  @arg \c f 1 Parameter callback function pointer for tick event
   *  @arg \c obj0 Object the callback is called on.
   *  @arg \c t1_0 1st parameter in callback function list
   */
  TickEvent1(Ticks_t t, Clock& c, void (T::*f)(U1), OBJ* obj0, T1 t1_0)
    : TickEventBase(t,c), handler(f), obj(obj0), t1(t1_0){}

  /** Defines the callback handler function pointer
   */        
  void (T::*handler)(U1);
  
  /** Object the CallHandler uses for the callback function
   */
  OBJ*      obj;
  
  /** 1st parameter in callback function list
   */
  T1        t1;
  
public:

  /** Calls the callback function when the event is processed.
   */
  void CallHandler();
};

template <typename T, typename OBJ, typename U1, typename T1>
void TickEvent1<T, OBJ, U1, T1>::CallHandler()
{
  (obj->*handler)(t1);
}

/** TickEvent2 subclasses TickEventBase, it defines a 
 *  2 parameter member function CallHandler for a TickEvent
 */
template<typename T, typename OBJ,
         typename U1, typename T1, 
         typename U2, typename T2>
class TickEvent2 : public TickEventBase
{
public:

  /** Constructor
   *  @arg \c t Latency in ticks.
   *  @arg \c c Reference clock used with tick latency.
   *  @arg \c f 2 Parameter callback function pointer for tick event
   *  @arg \c obj0 Object the callback is called on.
   *  @arg \c t1_0 1st parameter in callback function list
   *  @arg \c t2_0 2nd parameter in callback function list      
   */    
  TickEvent2(Ticks_t t, Clock& c, void (T::*f)(U1, U2), OBJ* obj0, T1 t1_0, T2 t2_0)
    : TickEventBase(t,c), handler(f), obj(obj0), t1(t1_0), t2(t2_0) {}

  /** Defines the callback handler function pointer
   */        
  void (T::*handler)(U1, U2);
  
  /** Object the CallHandler uses for the callback function
   */  
  OBJ*      obj;

  /** 1st parameter in callback function list
   */
  T1        t1;

  /** 2nd parameter in callback function list
   */  
  T2        t2;
  
public:

  /** Calls the callback function when the event is processed.
   */
  void CallHandler();
};

template <typename T, typename OBJ, 
          typename U1, typename T1,
          typename U2, typename T2>
void TickEvent2<T, OBJ, U1, T1, U2, T2>::CallHandler()
{
  (obj->*handler)(t1, t2);
}

/** TickEvent3 subclasses TickEventBase, it defines a 
 *  3 parameter member function CallHandler for a TickEvent
 */
template <typename T,  typename OBJ,
          typename U1, typename T1,
          typename U2, typename T2,
          typename U3, typename T3>
class TickEvent3 : public TickEventBase {
public:

   /** Constructor
    *  @arg \c t Latency in ticks.
    *  @arg \c c Reference clock used with tick latency.
    *  @arg \c f 3 Parameter callback function pointer for tick event
    *  @arg \c obj0 Object the callback is called on.
    *  @arg \c t1_0 1st parameter in callback function list
    *  @arg \c t2_0 2nd parameter in callback function list 
    *  @arg \c t3_0 3rd parameter in callback function list
    */    
   TickEvent3(Ticks_t t, Clock& c, void (T::*f)(U1, U2, U3), OBJ *obj0, T1 t1_0, T2 t2_0, T3 t3_0)  
     : TickEventBase(t,c), handler(f), obj(obj0), t1(t1_0), t2(t2_0), t3(t3_0) {}

   /** Defines the callback handler function pointer
    */      
   void (T::*handler)(U1, U2, U3);
   
   /** Object the CallHandler uses for the callback function
    */   
   OBJ* obj;

   /** 1st parameter in callback function list
    */   
   T1 t1;

   /** 2nd parameter in callback function list
    */   
   T2 t2;

   /** 3rd parameter in callback function list
    */   
   T3 t3;
   
public:

   /** Calls the callback function when the event is processed.
    */ 
   void CallHandler();
};

template <typename T,  typename OBJ,
          typename U1, typename T1,
          typename U2, typename T2,
          typename U3, typename T3> 
void TickEvent3<T,OBJ,U1,T1,U2,T2,U3,T3>::CallHandler() {
     (obj->*handler)(t1,t2,t3);
}

/** TickEvent4 subclasses TickEventBase, it defines a 
 *  4 parameter member function CallHandler for a TickEvent
 */
template <typename T,  typename OBJ,
          typename U1, typename T1,
          typename U2, typename T2,
          typename U3, typename T3,
          typename U4, typename T4>
class TickEvent4 : public TickEventBase {
public:

   /** Constructor
    *  @arg \c t Latency in ticks.
    *  @arg \c c Reference clock used with tick latency.
    *  @arg \c f 4 Parameter callback function pointer for tick event
    *  @arg \c obj0 Object the callback is called on.
    *  @arg \c t1_0 1st parameter in callback function list
    *  @arg \c t2_0 2nd parameter in callback function list 
    *  @arg \c t3_0 3rd parameter in callback function list
    *  @arg \c t4_0 4th parameter in callback function list            
    */    
   TickEvent4(Ticks_t t, Clock& c, void (T::*f)(U1, U2, U3, U4), OBJ *obj0, T1 t1_0, T2 t2_0, T3 t3_0, T4 t4_0)  
     : TickEventBase(t,c), handler(f), obj(obj0), t1(t1_0), t2(t2_0), t3(t3_0), t4(t4_0){}
   
   /** Defines the callback handler function pointer
    */    
   void (T::*handler)(U1, U2, U3, U4);
   
   /** Object the CallHandler uses for the callback function
    */
   OBJ* obj;

   /** 1st parameter in callback function list
    */   
   T1 t1;
   
   /** 2nd parameter in callback function list
    */   
   T2 t2;
   
   /** 3rd parameter in callback function list
    */   
   T3 t3;
   
   /** 4th parameter in callback function list
    */   
   T4 t4;
   
public:

   /** Calls the callback function when the event is processed.
    */  
   void CallHandler();
};

template <typename T,  typename OBJ,
          typename U1, typename T1,
          typename U2, typename T2,
          typename U3, typename T3,
          typename U4, typename T4> 
void TickEvent4<T,OBJ,U1,T1,U2,T2,U3,T3,U4,T4>::CallHandler() {
     (obj->*handler)(t1,t2,t3,t4);
}

// ----------------------------------------------------------------------- //
// Also need a variant of the Event0 that calls a static function,
// not a member function.

/** TickEvent0Stat subclasses TickEventBase, it defines a 
 *  tick event with a 0 parameter static callback function 
 *  rather than a object member function.
 */
class TickEvent0Stat : public TickEventBase
{
public:

  /** Constructor
   *  @arg \c t Latency in ticks.
   *  @arg \c c Reference clock for tick latency.
   *  @arg \c f 0 Parameter static callback function pointer.
   */
  TickEvent0Stat(Ticks_t t, Clock& c, void (*f)(void))
    : TickEventBase(t,c), handler(f){}
  
  /** Defines the static callback handler function pointer.
   */  
  void (*handler)(void);

public:

  /** Calls the static callback function when the event is processed.
   */  
  void CallHandler();
};

/** TickEvent1Stat subclasses TickEventBase, it defines a 
 *  tick event with a 1 parameter static callback function 
 *  rather than a object member function.
 */
template<typename U1, typename T1>
class TickEvent1Stat : public TickEventBase
{
public:

  /** Constructor
   *  @arg \c t Latency in ticks.
   *  @arg \c c Reference clock for tick latency.
   *  @arg \c f 1 Parameter static callback function pointer.
   *  @arg \c t1_0 1st parameter in callback function list   
   */
  TickEvent1Stat(Ticks_t t, Clock& c, void (*f)(U1), T1 t1_0)
    : TickEventBase(t,c), handler(f), t1(t1_0){}
    
  /** Defines the static callback handler function pointer.
   */     
  void (*handler)(U1);
  
  /** 1st parameter in callback function list
   */ 
  T1        t1;

public:

  /** Calls the static callback function when the event is processed.
   */ 
  void CallHandler();
};

template <typename U1, typename T1>
void TickEvent1Stat<U1, T1>::CallHandler()
{
  handler(t1);
}

/** TickEvent2Stat subclasses TickEventBase, it defines a 
 *  tick event with a 2 parameter static callback function 
 *  rather than a object member function.
 */
template<typename U1, typename T1, 
         typename U2, typename T2>
class TickEvent2Stat : public TickEventBase
{
public:

  /** Constructor
   *  @arg \c t Latency in ticks.
   *  @arg \c c Reference clock for tick latency.
   *  @arg \c f 2 Parameter static callback function pointer.
   *  @arg \c t1_0 1st parameter in callback function list
   *  @arg \c t2_0 2nd parameter in callback function list    
   */
  TickEvent2Stat(Ticks_t t, Clock& c, void (*f)(U1, U2), T1 t1_0, T2 t2_0)
    : TickEventBase(t,c), handler(f), t1(t1_0), t2(t2_0) {}
  
  /** Defines the static callback handler function pointer.
   */     
  void (*handler)(U1, U2);
  
  /** 1st parameter in static callback function list
   */   
  T1        t1;

  /** 2nd parameter in callback function list
   */   
  T2        t2;

public:

  /** Calls the static callback function when the event is processed.
   */ 
  void CallHandler();
};

template <typename U1, typename T1,
          typename U2, typename T2>
void TickEvent2Stat<U1, T1, U2, T2>::CallHandler()
{
  handler(t1, t2);
}

/** TickEvent3Stat subclasses TickEventBase, it defines a 
 *  tick event with a 3 parameter static callback function 
 *  rather than a object member function.
 */
template <typename U1, typename T1,
          typename U2, typename T2,
          typename U3, typename T3>
class TickEvent3Stat : public TickEventBase {
public:
  
   /** Constructor
    *  @arg \c t Latency in ticks.
    *  @arg \c c Reference clock for tick latency.
    *  @arg \c f 3 Parameter static callback function pointer.
    *  @arg \c t1_0 1st parameter in callback function list
    *  @arg \c t2_0 2nd parameter in callback function list 
    *  @arg \c t3_0 3rd parameter in callback function list    
    */
   TickEvent3Stat(Ticks_t t, Clock& c, void (*f)(U1, U2, U3), T1 t1_0, T2 t2_0, T3 t3_0)  
     : TickEventBase(t,c), handler(f), t1(t1_0), t2(t2_0), t3(t3_0) {}

   /** Defines the static callback handler function pointer.
    */      
   void (*handler)(U1, U2, U3);
   
   /** 1st parameter in static callback function list
    */    
   T1 t1;
   
   /** 2nd parameter in static callback function list
    */ 
   T2 t2;
   
   /** 3rd parameter in static callback function list
    */    
   T3 t3;
   
public:

   /** Calls the static callback function when the event is processed.
    */ 
   void CallHandler();
};

template <typename U1, typename T1,
          typename U2, typename T2,
          typename U3, typename T3> 
void TickEvent3Stat<U1,T1,U2,T2,U3,T3>::CallHandler()
{
  handler(t1,t2,t3);
}

/** TickEvent4Stat subclasses TickEventBase, it defines a 
 *  tick event with a 4 parameter static callback function 
 *  rather than a object member function.
 */
template <typename U1, typename T1,
          typename U2, typename T2,
          typename U3, typename T3,
          typename U4, typename T4>
class TickEvent4Stat : public TickEventBase {
public:

   /** Constructor
    *  @arg \c t Latency in ticks.
    *  @arg \c c Reference clock for tick latency.
    *  @arg \c f 4 Parameter static callback function pointer.
    *  @arg \c t1_0 1st parameter in callback function list
    *  @arg \c t2_0 2nd parameter in callback function list 
    *  @arg \c t3_0 3rd parameter in callback function list
    *  @arg \c t4_0 4th parameter in callback function list    
    */
   TickEvent4Stat(Ticks_t t, Clock& c, void (*f)(U1, U2, U3, U4), T1 t1_0, T2 t2_0, T3 t3_0, T4 t4_0)  
     : TickEventBase(t,c), handler(f), t1(t1_0), t2(t2_0), t3(t3_0), t4(t4_0){}

   /** Defines the static callback handler function pointer.
    */      
   void (*handler)(U1, U2, U3, U4);
   
   /** 1st parameter in static callback function list
    */    
   T1 t1;
   
   /** 2nd parameter in static callback function list
    */    
   T2 t2;
   
   /** 3rd parameter in static callback function list
    */    
   T3 t3;
   
   /** 4th parameter in static callback function list
    */    
   T4 t4;
   
public:

   /** Calls the static callback function when the event is processed.
    */ 
   void CallHandler();
};

template <typename U1, typename T1,
          typename U2, typename T2,
          typename U3, typename T3,
          typename U4, typename T4> 
void TickEvent4Stat<U1,T1,U2,T2,U3,T3,U4,T4>::CallHandler() {
     handler(t1,t2,t3,t4);
}

// ----------------------------------------------------------------------- //  

/** Defines the base event class for floating point events.
 */
class EventBase 
{
 public:
 
 /** Constructor
  *  @arg \c t Latency in time.
  */
 EventBase(double t) : time(t), uid(nextUID++) {}
 
 /** Constructor
  *  @arg \c t Latency in time.
  *  @arg \c u Unique id of the event. 
  */
 EventBase(double t, int u) : time(t), uid(u) {}
  
  /** Virtual function, all subclasses must implement CallHandler
   */   
  virtual void CallHandler() = 0;
  
 public:
 
      /** Timestamp for the event
       */
      double time;
      
      /** Each event has a uniques identifier to break timestamp ties
       */
      int    uid;
      
 private:
 
      /** Holds the next available unique id. 
       */
      static int nextUID;
};

/** EventId subclasses EventBase, and is the return type from 
 *  all schedule functions.  This is used to cancel the event.
 */
class EventId : public EventBase
{
public:

  /** Constructor
   *  @arg \c t Latency in time.
   *  @arg \c u Unique id of the event.
   */
  EventId(double t, int u) : EventBase(t, u) {}
  
  /** Calls the callback function when the event is processed.
   */ 
  void CallHandler() {}
};

/** Event0 defines an event object with a 
 *  0 parameter member function callback
 */
template<typename T, typename OBJ>
class Event0 : public EventBase
{
public:

  /** Constructor
   *  @arg \c t Latency in time.
   *  @arg \c f 0 Parameter member function callback pointer.
   *  @arg \c obj0 Object the callback function is called on.
   */
  Event0(double t, void (T::*f)(void), OBJ* obj0)
    : EventBase(t), handler(f), obj(obj0){}
  
  /** Defines the static callback handler function pointer.
   */        
  void (T::*handler)(void);
      
  /** Object the CallHandler uses for the callback function
   */
  OBJ*      obj;
  
public:

  /** Calls the callback function when the event is processed.
   */ 
  void CallHandler();
};

template <typename T, typename OBJ>
void Event0<T, OBJ>::CallHandler()
{
  (obj->*handler)();
}

/** Event1 defines an event object with a 
 *  1 parameter member function callback
 */
template<typename T, typename OBJ, typename U1, typename T1>
class Event1 : public EventBase
{
public:

  /** Constructor
   *  @arg \c t Latency in time.
   *  @arg \c f 1 Parameter member function callback pointer.
   *  @arg \c obj0 Object the callback function is called on.
   *  @arg \c t1_0 1st parameter in callback function list   
   */
  Event1(double t, void (T::*f)(U1), OBJ* obj0, T1 t1_0)
    : EventBase(t), handler(f), obj(obj0), t1(t1_0){}

  /** Defines the member function callback handler pointer.
   */     
  void (T::*handler)(U1);
  
  /** Object the CallHandler uses for the callback function
   */  
  OBJ*      obj;
  
  /** 1st parameter in callback function list  
   */  
  T1        t1;
  
public:

  /** Calls the callback function when the event is processed.
   */ 
  void CallHandler();
};

template <typename T, typename OBJ, typename U1, typename T1>
void Event1<T, OBJ, U1, T1>::CallHandler()
{
  (obj->*handler)(t1);
}

/** Event2 defines an event object with a 
 *  2 parameter member function callback
 */
template<typename T, typename OBJ,
         typename U1, typename T1, 
         typename U2, typename T2>
class Event2 : public EventBase
{
public:

  /** Constructor
   *  @arg \c t Latency in time.
   *  @arg \c f 2 Parameter member function callback pointer.
   *  @arg \c obj0 Object the callback function is called on.
   *  @arg \c t1_0 1st parameter in callback function list
   *  @arg \c t2_0 2nd parameter in callback function list    
   */
  Event2(double t, void (T::*f)(U1, U2), OBJ* obj0, T1 t1_0, T2 t2_0)
    : EventBase(t), handler(f), obj(obj0), t1(t1_0), t2(t2_0) {}

  /** Defines the member function callback handler pointer.
   */ 
  void (T::*handler)(U1, U2);
  
  /** Object the CallHandler uses for the callback function
   */   
  OBJ*      obj;

  /** 1st parameter in callback function list  
   */  
  T1        t1;
  
  /** 2nd parameter in callback function list  
   */  
  T2        t2;
  
public:

  /** Calls the callback function when the event is processed.
   */ 
  void CallHandler();
};

template <typename T, typename OBJ, 
          typename U1, typename T1,
          typename U2, typename T2>
void Event2<T, OBJ, U1, T1, U2, T2>::CallHandler()
{
  (obj->*handler)(t1, t2);
}

/** Event3 defines an event object with a 
 *  3 parameter member function callback
 */
template <typename T,  typename OBJ,
          typename U1, typename T1,
          typename U2, typename T2,
          typename U3, typename T3>
class Event3 : public EventBase {
public:

   /** Constructor
    *  @arg \c t Latency in time.
    *  @arg \c f 3 Parameter member function callback pointer.
    *  @arg \c obj0 Object the callback function is called on.
    *  @arg \c t1_0 1st parameter in callback function list
    *  @arg \c t2_0 2nd parameter in callback function list 
    *  @arg \c t3_0 3rd parameter in callback function list    
    */
   Event3(double t, void (T::*f)(U1, U2, U3), OBJ *obj0, T1 t1_0, T2 t2_0, T3 t3_0)  
     : EventBase(t), handler(f), obj(obj0), t1(t1_0), t2(t2_0), t3(t3_0) {}
     
   /** Defines the member function callback handler pointer.
    */      
   void (T::*handler)(U1, U2, U3);

   /** Object the CallHandler uses for the callback function
    */    
   OBJ* obj;
   
   /** 1st parameter in callback function list  
    */   
   T1 t1;
   
   /** 2nd parameter in callback function list  
    */   
   T2 t2;
   
   /** 3rd parameter in callback function list  
    */   
   T3 t3;
   
public:

   /** Calls the callback function when the event is processed.
    */ 
   void CallHandler();
};

template <typename T,  typename OBJ,
          typename U1, typename T1,
          typename U2, typename T2,
          typename U3, typename T3> 
void Event3<T,OBJ,U1,T1,U2,T2,U3,T3>::CallHandler() {
     (obj->*handler)(t1,t2,t3);
}

/** Event4 defines an event object with a 
 *  4 parameter member function callback
 */
template <typename T,  typename OBJ,
          typename U1, typename T1,
          typename U2, typename T2,
          typename U3, typename T3,
          typename U4, typename T4>
class Event4 : public EventBase {
public:

   /** Constructor
    *  @arg \c t Latency in time.
    *  @arg \c f 4 Parameter member function callback pointer.
    *  @arg \c obj0 Object the callback function is called on.
    *  @arg \c t1_0 1st parameter in callback function list
    *  @arg \c t2_0 2nd parameter in callback function list 
    *  @arg \c t3_0 3rd parameter in callback function list
    *  @arg \c t4_0 4th parameter in callback function list    
    */
   Event4(double t, void (T::*f)(U1, U2, U3, U4), OBJ *obj0, T1 t1_0, T2 t2_0, T3 t3_0, T4 t4_0)  
     : EventBase(t), handler(f), obj(obj0), t1(t1_0), t2(t2_0), t3(t3_0), t4(t4_0){}

   /** Defines the member function callback handler pointer.
    */      
   void (T::*handler)(U1, U2, U3, U4);
   
   /** Object the CallHandler uses for the callback function
    */    
   OBJ* obj;

   /** 1st parameter in callback function list  
    */     
   T1 t1;
   
   /** 2nd parameter in callback function list  
    */     
   T2 t2;
   
   /** 3rd parameter in callback function list  
    */     
   T3 t3;
   
   /** 4th parameter in callback function list  
    */     
   T4 t4;
   
public:

   /** Calls the callback function when the event is processed.
    */ 
   void CallHandler();
};

template <typename T,  typename OBJ,
          typename U1, typename T1,
          typename U2, typename T2,
          typename U3, typename T3,
          typename U4, typename T4> 
void Event4<T,OBJ,U1,T1,U2,T2,U3,T3,U4,T4>::CallHandler() {
     (obj->*handler)(t1,t2,t3,t4);
}

// Also need a variant of the Event0 that calls a static function,
// not a member function.

/** Event0Stat subclasses EventBase, it defines a 
 *  time event with a 0 parameter static callback function 
 *  rather than a object member function.
 */
class Event0Stat : public EventBase
{
public:

  /** Constructor
   *  @arg \c t Latency in time.
   *  @arg \c f 0 Parameter static callback function pointer.
   */
  Event0Stat(double t, void (*f)(void))
    : EventBase(t), handler(f){}

  /** Defines the static callback handler function pointer.
   */      
  void (*handler)(void);

public:

  /** Calls the callback function when the event is processed.
   */ 
  void CallHandler();
};

/** Event1Stat subclasses EventBase, it defines a 
 *  time event with a 1 parameter static callback function 
 *  rather than a object member function.
 */
template<typename U1, typename T1>
class Event1Stat : public EventBase
{
public:

  /** Constructor
   *  @arg \c t Latency in time.
   *  @arg \c f 1 Parameter static callback function pointer.
   *  @arg \c t1_0 1st parameter in callback function list   
   */
  Event1Stat(double t, void (*f)(U1), T1 t1_0)
    : EventBase(t), handler(f), t1(t1_0){}

  /** Defines the static callback handler function pointer.
   */          
  void (*handler)(U1);
  
  /** 1st parameter in callback function list  
   */   
  T1        t1;

public:

  /** Calls the callback function when the event is processed.
   */ 
  void CallHandler();
};

template <typename U1, typename T1>
void Event1Stat<U1, T1>::CallHandler()
{
  handler(t1);
}

/** Event2Stat subclasses EventBase, it defines a 
 *  time event with a 2 parameter static callback function 
 *  rather than a object member function.
 */
template<typename U1, typename T1, 
         typename U2, typename T2>
class Event2Stat : public EventBase
{
public:

  /** Constructor
   *  @arg \c t Latency in time.
   *  @arg \c f 2 Parameter static callback function pointer.
   *  @arg \c t1_0 1st parameter in callback function list
   *  @arg \c t2_0 2nd parameter in callback function list    
   */
  Event2Stat(double t, void (*f)(U1, U2), T1 t1_0, T2 t2_0)
    : EventBase(t), handler(f), t1(t1_0), t2(t2_0) {}
    
  /** Defines the static callback handler function pointer.
   */      
  void (*handler)(U1, U2);

  /** 1st parameter in callback function list  
   */   
  T1        t1;

  /** 2nd parameter in callback function list  
   */   
  T2        t2;

public:

  /** Calls the callback function when the event is processed.
   */ 
  void CallHandler();
};

template <typename U1, typename T1,
          typename U2, typename T2>
void Event2Stat<U1, T1, U2, T2>::CallHandler()
{
  handler(t1, t2);
}

/** Event3Stat subclasses EventBase, it defines a 
 *  time event with a 3 parameter static callback function 
 *  rather than a object member function.
 */
template <typename U1, typename T1,
          typename U2, typename T2,
          typename U3, typename T3>
class Event3Stat : public EventBase {
public:

   /** Constructor
    *  @arg \c t Latency in time.
    *  @arg \c f 3 Parameter static callback function pointer.
    *  @arg \c t1_0 1st parameter in callback function list
    *  @arg \c t2_0 2nd parameter in callback function list 
    *  @arg \c t3_0 3rd parameter in callback function list    
    */
   Event3Stat(double t, void (*f)(U1, U2, U3), T1 t1_0, T2 t2_0, T3 t3_0)  
     : EventBase(t), handler(f), t1(t1_0), t2(t2_0), t3(t3_0) {}

   /** Defines the static callback handler function pointer.
    */           
   void (*handler)(U1, U2, U3);

   /** 1st parameter in callback function list  
    */    
   T1 t1;
   
   /** 2nd parameter in callback function list  
    */    
   T2 t2;

   /** 3rd parameter in callback function list  
    */    
   T3 t3;
   
public:

   /** Calls the callback function when the event is processed.
    */ 
   void CallHandler();
};

template <typename U1, typename T1,
          typename U2, typename T2,
          typename U3, typename T3> 
void Event3Stat<U1,T1,U2,T2,U3,T3>::CallHandler()
{
  handler(t1,t2,t3);
}

/** Event4Stat subclasses EventBase, it defines a 
 *  time event with a 4 parameter static callback function 
 *  rather than a object member function.
 */
template <typename U1, typename T1,
          typename U2, typename T2,
          typename U3, typename T3,
          typename U4, typename T4>
class Event4Stat : public EventBase {
public:

   /** Constructor
    *  @arg \c t Latency in time.
    *  @arg \c f 4 Parameter static callback function pointer.
    *  @arg \c t1_0 1st parameter in callback function list
    *  @arg \c t2_0 2nd parameter in callback function list 
    *  @arg \c t3_0 3rd parameter in callback function list
    *  @arg \c t4_0 4th parameter in callback function list    
    */
   Event4Stat(double t, void (*f)(U1, U2, U3, U4), T1 t1_0, T2 t2_0, T3 t3_0, T4 t4_0)  
     : EventBase(t), handler(f), t1(t1_0), t2(t2_0), t3(t3_0), t4(t4_0){}

   /** Defines the static callback handler function pointer.
    */      
   void (*handler)(U1, U2, U3, U4);
   
   /** 1st parameter in callback function list  
    */    
   T1 t1;

   /** 2nd parameter in callback function list  
    */    
   T2 t2;
   
   /** 3rd parameter in callback function list  
    */    
   T3 t3;

   /** 4th parameter in callback function list  
    */    
   T4 t4;
   
public:

   /** Calls the callback function when the event is processed.
    */ 
   void CallHandler();
};

template <typename U1, typename T1,
          typename U2, typename T2,
          typename U3, typename T3,
          typename U4, typename T4> 
void Event4Stat<U1,T1,U2,T2,U3,T3,U4,T4>::CallHandler() 
{
     handler(t1,t2,t3,t4);
}

/** Defines the event comparator.
 */
class event_less
{
public:
  event_less() { }
  inline bool operator()(EventBase* const & l, const EventBase* const & r) const {
    if(l->time < r->time) return true;
    if (l->time == r->time) return l->uid < r->uid;
    return false;
  }
};

/** Defines the type for the sorted event list.
 */
typedef std::set<EventBase*, event_less> EventSet_t;

/** \class Manifold 
 *  The Manifold class is implemented with only static functions.
 */
class Manifold
{
public:

  static void Init(int argc, char** argv);

  /** Run the simulation
   */
  static void    Run(double macsim_simcycle);                 
  
  /** Stop the simulation
   */
  static void    Stop(); 
  
  /** Stop at the specified time
   *  @arg A time.
   */
  static void    StopAtTime(Time_t);
  
  /** Stop at specified tick (default clk)
   *  @arg A tick count.
   */
  static void    StopAt(Ticks_t);  
  
  /** Stop at ticks, based on specified clk
   *  @arg A tick count.
   *  @arg A reference clock for tick count.         
   */
  static void    StopAtClock(Ticks_t, Clock&);
  
  /** Return the current simulation time
   */
  static double  Now();                 
  
  /** Current simtime in ticks, default clk
   */
  static Ticks_t NowTicks();            
  
  /** Current simtime in half ticks, default clk
   */
  static Ticks_t NowHalfTicks();            
  
  /** Current simtime in ticks, given clk
   */
  static Ticks_t NowTicks(Clock&);            
  
  /** Current simtime in half ticks, given clk
   */
  static Ticks_t NowHalfTicks(Clock&);            
  
  /** Cancel previously scheduled event
   *  @arg Unique time event id number.
   */
  static bool    Cancel(EventId&);
  
  /** Cancel prev scheduled Tick event
   *  @arg Unique tick event id number.
   */          
  static bool    Cancel(TickEventId&);
  
  /** Peek (doesn't remove) earliest event from the even list
   */
  static EventId Peek();

  /** Return earliest event
   */  
  static EventBase* GetEarliestEvent();
  
  /** Returns MPI Rank if distributed, zero if not
   */
  static LpId_t GetRank();
  
  /** Start the MPI processing             
   */ 
  static void   EnableDistributed();

#ifdef KERNEL_UTEST
  static void unhalt();
#endif

  // There are some variations of the required API.
  // First specifies latency in units of clock ticks for the default clock
  // This the common case and should be used most of the time.

  // The next two specify latency in units of clock ticks for the default
  // clock                          

  /** Connects an output of a component to an input on a second component.
   *  There are either one or two template parameters.  The first is always
   *  the type of the receiving component.  This is needed to insure the
   *  callback function is of the right type.  The second is optional
   *  and is the type of the data being sent on the link.
   *  If not specified, uint64_t is assumed.
   *  The handlers MUST all have the first parameter as an int, which
   *  indicates the input number the arrival is for.  The second is
   *  a uint64_t (default). Latency is in reference to the default clock.
   *  @arg \c sourceComponent The source component's unique id.
   *  @arg \c sourceIndex The source index(port) number.
   *  @arg \c dstComponent The destination component's unique id.
   *  @arg \c dstIndex The destination index(port) number.
   *  @arg \c handler Callback function pointer that expects uint64_t data.
   *  @arg \c tickLatency Link latency in ticks.
   */    
   /*
  template <typename T>
  static LinkId_t Connect(CompId_t sourceComponent, int sourceIndex,
                          CompId_t dstComponent,    int dstIndex,
                          void (T::*handler)(int, uint64_t),
                          Ticks_t tickLatency);
*/
  /** Connects an output of a component to an input on a second component.
   *  There are either one or two template parameters.  The first is always
   *  the type of the receiving component.  This is needed to insure the
   *  callback function is of the right type.  The second is optional
   *  and is the type of the data being sent on the link.
   *  If not specified, uint64_t is assumed.
   *  The handlers MUST all have the first parameter as an int, which
   *  indicates the input number the arrival is for.  The second is
   *  an arbitrary type T. Latency is in reference to the default clock.
   *  @arg \c sourceComponent The source component's unique id.
   *  @arg \c sourceIndex The source index(port) number.
   *  @arg \c dstComponent The destination component's unique id.
   *  @arg \c dstIndex The destination index(port) number.
   *  @arg \c handler Callback function pointer that expects a arbitrary type data.
   *  @arg \c tickLatency Link latency in ticks.
   */                          
  template <typename T, typename T2>
  static void Connect(CompId_t sourceComponent, int sourceIndex,
                          CompId_t dstComponent,    int dstIndex,
                          void (T::*handler)(int, T2),
                          Ticks_t tickLatency) throw (LinkTypeMismatchException);

  // The next two specify latency in units of half clock ticks for the default
  // clock                          

  /** Connects an output of a component to an input on a second component.
   *  There are either one or two template parameters.  The first is always
   *  the type of the receiving component.  This is needed to insure the
   *  callback function is of the right type.  The second is optional
   *  and is the type of the data being sent on the link.
   *  If not specified, uint64_t is assumed.
   *  The handlers MUST all have the first parameter as an int, which
   *  indicates the input number the arrival is for.  The second is
   *  a uint64_t (default). Latency is in reference to the default clock.
   *  @arg \c sourceComponent The source component's unique id.
   *  @arg \c sourceIndex The source index(port) number.
   *  @arg \c dstComponent The destination component's unique id.
   *  @arg \c dstIndex The destination index(port) number.
   *  @arg \c handler Callback function pointer that expects uint64_t data.
   *  @arg \c tickLatency Link latency in half ticks.
   */
/*
  template <typename T>
  static LinkId_t ConnectHalf(CompId_t sourceComponent, int sourceIndex,
                              CompId_t dstComponent,    int dstIndex,
                              void (T::*handler)(int, uint64_t),
                              Ticks_t tickLatency);
*/
  /** Connects an output of a component to an input on a second component.
   *  There are either one or two template parameters.  The first is always
   *  the type of the receiving component.  This is needed to insure the
   *  callback function is of the right type.  The second is optional
   *  and is the type of the data being sent on the link.
   *  If not specified, uint64_t is assumed.
   *  The handlers MUST all have the first parameter as an int, which
   *  indicates the input number the arrival is for.  The second is
   *  an arbitrary type T. Latency is in reference to the default clock.
   *  @arg \c sourceComponent The source component's unique id.
   *  @arg \c sourceIndex The source index(port) number.
   *  @arg \c dstComponent The destination component's unique id.
   *  @arg \c dstIndex The destination index(port) number.
   *  @arg \c handler Callback function pointer that expects a arbitrary type data.
   *  @arg \c tickLatency Link latency in half ticks.
   */                              
  template <typename T, typename T2>
  static void ConnectHalf(CompId_t sourceComponent, int sourceIndex,
                              CompId_t dstComponent,    int dstIndex,
                              void (T::*handler)(int, T2),
                              Ticks_t tickLatency) throw (LinkTypeMismatchException);
                              
  // The next two specify latency in units of clock ticks for the specified
  // clock
  
  /** Connects an output of a component to an input on a second component.
   *  There are either one or two template parameters.  The first is always
   *  the type of the receiving component.  This is needed to insure the
   *  callback function is of the right type.  The second is optional
   *  and is the type of the data being sent on the link.
   *  If not specified, uint64_t is assumed.
   *  The handlers MUST all have the first parameter as an int, which
   *  indicates the input number the arrival is for.  The second is
   *  a uint64_t (default). Latency is in reference to the specified clock.
   *  @arg \c sourceComponent The source component's unique id.
   *  @arg \c sourceIndex The source index(port) number.
   *  @arg \c dstComponent The destination component's unique id.
   *  @arg \c dstIndex The destination index(port) number.
   *  @arg \c c The specified reference clock for clock ticks.
   *  @arg \c handler Callback function pointer that expects uint64_t data.
   *  @arg \c tickLatency Link latency in ticks.
   */  
   /*
  template <typename T>
  static LinkId_t ConnectClock(CompId_t sourceComponent, int sourceIndex,
                               CompId_t dstComponent,    int dstIndex,
                               Clock& c,
                               void (T::*handler)(int, uint64_t),
                               Ticks_t tickLatency);
*/ 
  /** Connects an output of a component to an input on a second component.
   *  There are either one or two template parameters.  The first is always
   *  the type of the receiving component.  This is needed to insure the
   *  callback function is of the right type.  The second is optional
   *  and is the type of the data being sent on the link.
   *  If not specified, uint64_t is assumed.
   *  The handlers MUST all have the first parameter as an int, which
   *  indicates the input number the arrival is for.  The second is
   *  an arbitrary type T. Latency is in reference to the specified clock.
   *  @arg \c sourceComponent The source component's unique id.
   *  @arg \c sourceIndex The source index(port) number.
   *  @arg \c dstComponent The destination component's unique id.
   *  @arg \c dstIndex The destination index(port) number.
   *  @arg \c c The specified reference clock for clock ticks.
   *  @arg \c handler Callback function pointer that expects a arbitrary type data.
   *  @arg \c tickLatency Link latency in ticks.
   */                                                                
  template <typename T, typename T2>
  static void ConnectClock(CompId_t sourceComponent, int sourceIndex,
                               CompId_t dstComponent,    int dstIndex,
                               Clock& c,
                               void (T::*handler)(int, T2),
                               Ticks_t tickLatency) throw (LinkTypeMismatchException);

  // The next two specify latency in units of half clock ticks for the specified
  // clock
  
  /** Connects an output of a component to an input on a second component.
   *  There are either one or two template parameters.  The first is always
   *  the type of the receiving component.  This is needed to insure the
   *  callback function is of the right type.  The second is optional
   *  and is the type of the data being sent on the link.
   *  If not specified, uint64_t is assumed.
   *  The handlers MUST all have the first parameter as an int, which
   *  indicates the input number the arrival is for.  The second is
   *  a uint64_t (default). Latency is in reference to the specified clock.
   *  @arg \c sourceComponent The source component's unique id.
   *  @arg \c sourceIndex The source index(port) number.
   *  @arg \c dstComponent The destination component's unique id.
   *  @arg \c dstIndex The destination index(port) number.
   *  @arg \c c The specified reference clock for clock ticks.
   *  @arg \c handler Callback function pointer that expects uint64_t data.
   *  @arg \c tickLatency Link latency in half ticks.
   */   
   /*
  template <typename T>
  static LinkId_t ConnectClockHalf(CompId_t sourceComponent, int sourceIndex,
                                   CompId_t dstComponent,    int dstIndex,
                                   Clock& c,
                                   void (T::*handler)(int, uint64_t),
                                   Ticks_t tickLatency);
   */                                
  /** Connects an output of a component to an input on a second component.
   *  There are either one or two template parameters.  The first is always
   *  the type of the receiving component.  This is needed to insure the
   *  callback function is of the right type.  The second is optional
   *  and is the type of the data being sent on the link.
   *  If not specified, uint64_t is assumed.
   *  The handlers MUST all have the first parameter as an int, which
   *  indicates the input number the arrival is for.  The second is
   *  an arbitrary type T. Latency is in reference to the specified clock.
   *  @arg \c sourceComponent The source component's unique id.
   *  @arg \c sourceIndex The source index(port) number.
   *  @arg \c dstComponent The destination component's unique id.
   *  @arg \c dstIndex The destination index(port) number.
   *  @arg \c c The specified reference clock for clock ticks.
   *  @arg \c handler Callback function pointer that expects a arbitrary type data.
   *  @arg \c tickLatency Link latency in half ticks.
   */                                    
  template <typename T, typename T2>
  static void ConnectClockHalf(CompId_t sourceComponent, int sourceIndex,
                                   CompId_t dstComponent,    int dstIndex,
                                   Clock& c,
                                   void (T::*handler)(int, T2),
                                   Ticks_t tickLatency) throw (LinkTypeMismatchException);

  // The final two specify latency in units seconds
  // clock
  
  /** Connects an output of a component to an input on a second component.
   *  There are either one or two template parameters.  The first is always
   *  the type of the receiving component.  This is needed to insure the
   *  callback function is of the right type.  The second is optional
   *  and is the type of the data being sent on the link.
   *  If not specified, uint64_t is assumed.
   *  The handlers MUST all have the first parameter as an int, which
   *  indicates the input number the arrival is for.  The second is
   *  a uint64_t (default). Latency is in reference to the default clock.
   *  @arg \c sourceComponent The source component's unique id.
   *  @arg \c sourceIndex The source index(port) number.
   *  @arg \c dstComponent The destination component's unique id.
   *  @arg \c dstIndex The destination index(port) number.
   *  @arg \c handler Callback function pointer that expects uint64_t data.
   *  @arg \c tickLatency Link latency in time.
   */
   /*
  template <typename T>
  static LinkId_t ConnectTime(CompId_t sourceComponent, int sourceIndex,
                              CompId_t dstComponent,    int dstIndex,
                              void (T::*handler)(int, uint64_t),
                              Time_t latency);
    */                          
  /** Connects an output of a component to an input on a second component.
   *  There are either one or two template parameters.  The first is always
   *  the type of the receiving component.  This is needed to insure the
   *  callback function is of the right type.  The second is optional
   *  and is the type of the data being sent on the link.
   *  If not specified, uint64_t is assumed.
   *  The handlers MUST all have the first parameter as an int, which
   *  indicates the input number the arrival is for.  The second is
   *  an arbitrary type T. Latency is in reference to the default clock.
   *  @arg \c sourceComponent The source component's unique id.
   *  @arg \c sourceIndex The source index(port) number.
   *  @arg \c dstComponent The destination component's unique id.
   *  @arg \c dstIndex The destination index(port) number.
   *  @arg \c handler Callback function pointer that expects a arbitrary type data.
   *  @arg \c tickLatency Link latency in time.
   */                                  
  template <typename T, typename T2>
  static void ConnectTime(CompId_t sourceComponent, int sourceIndex,
                              CompId_t dstComponent,    int dstIndex,
                              void (T::*handler)(int, T2),
                              Time_t latency) throw (LinkTypeMismatchException);

  // Define the templated schedule functions
  // There are variations for up to four parameters,
  // and another set for non-member (static) function callbacks.
  // These are the definitions of the functions, the implementations are
  // in manifold.h
  // --------------------------------------------------------------------- //
  // This set are for "tick" events, that  are scheduled on integral ticks
  // with respect to the "master" (first defined) clock

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the master clock.
   *  @arg \c handler 0 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  \return Unique tick event id
   */
  template <typename T, typename OBJ>
    static TickEventId Schedule(Ticks_t t, void(T::*handler)(void), OBJ* obj);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the master clock.
   *  @arg \c handler 1 Parameter callback function pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list
   *  \return Unique tick event id
   */
  template <typename T, typename OBJ,
    typename U1, typename T1>
    static TickEventId Schedule(Ticks_t t, void(T::*handler)(U1), OBJ* obj, T1 t1);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the master clock.
   *  @arg \c handler 2 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list    
   *  \return Unique tick event id
   */
  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2>
    static TickEventId Schedule(Ticks_t t, void(T::*handler)(U1, U2), OBJ* obj, T1 t1, T2 t2);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the master clock.
   *  @arg \c handler 3 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list   
   *  \return Unique tick event id
   */
  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3>
    static TickEventId Schedule(Ticks_t t, void(T::*handler)(U1, U2, U3), OBJ* obj, T1 t1, T2 t2, T3 t3);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the master clock.
   *  @arg \c handler 4 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list
   *  @arg \c t4 4th parameter in callback function list   
   *  \return Unique tick event id
   */
  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3,
    typename U4, typename T4>
    static TickEventId Schedule(Ticks_t t, void(T::*handler)(U1, U2, U3, U4), OBJ* obj, T1 t1, T2 t2, T3 t3, T4 t4);

  // --------------------------------------------------------------------- //
  // This set are for "tick" events, that  are scheduled on integral ticks
  // with respect to the "master" (first defined) clock.  The units are
  // half-ticks. If presently on a rising edge, a value of 3 (for example)
  // results in a falling edge event 1.5 cycles in the future
  
  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the master clock.
   *  @arg \c handler 0 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  \return Unique tick event id
   */  
  template <typename T, typename OBJ>
    static TickEventId ScheduleHalf(Ticks_t t, void(T::*handler)(void), OBJ* obj);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the master clock.
   *  @arg \c handler 1 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list   
   *  \return Unique tick event id
   */
  template <typename T, typename OBJ,
    typename U1, typename T1>
    static TickEventId ScheduleHalf(Ticks_t t, void(T::*handler)(U1), OBJ* obj, T1 t1);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the master clock.
   *  @arg \c handler 2 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list    
   *  \return Unique tick event id
   */
  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2>
    static TickEventId ScheduleHalf(Ticks_t t, void(T::*handler)(U1, U2), OBJ* obj, T1 t1, T2 t2);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the master clock.
   *  @arg \c handler 3 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list   
   *  \return Unique tick event id
   */
  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3>
    static TickEventId ScheduleHalf(Ticks_t t, void(T::*handler)(U1, U2, U3), OBJ* obj, T1 t1, T2 t2, T3 t3);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the master clock.
   *  @arg \c handler 4 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list
   *  @arg \c t4 4th parameter in callback function list
   *  \return Unique tick event id
   */
  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3,
    typename U4, typename T4>
    static TickEventId ScheduleHalf(Ticks_t t, void(T::*handler)(U1, U2, U3, U4), OBJ* obj, T1 t1, T2 t2, T3 t3, T4 t4);

  // This set are for "tick" events, that  are scheduled on integral ticks
  // with respect to the specified clock
  
  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the specified clock.
   *  @arg \c c Reference clock for latency.
   *  @arg \c handler 0 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  \return Unique tick event id
   */  
  template <typename T, typename OBJ>
    static TickEventId ScheduleClock(Ticks_t t, Clock& c, void(T::*handler)(void), OBJ* obj);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the specified clock.
   *  @arg \c c Reference clock for latency.
   *  @arg \c handler 1 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list   
   *  \return Unique tick event id
   */  
  template <typename T, typename OBJ,
    typename U1, typename T1>
    static TickEventId ScheduleClock(Ticks_t t, Clock& c, void(T::*handler)(U1), OBJ* obj, T1 t1);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the specified clock.
   *  @arg \c c Reference clock for latency.
   *  @arg \c handler 2 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list    
   *  \return Unique tick event id
   */      
  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2>
    static TickEventId ScheduleClock(Ticks_t t, Clock& c, void(T::*handler)(U1, U2), OBJ* obj, T1 t1, T2 t2);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the specified clock.
   *  @arg \c c Reference clock for latency.
   *  @arg \c handler 3 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list   
   *  \return Unique tick event id
   */      
  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3>
    static TickEventId ScheduleClock(Ticks_t t, Clock& c, void(T::*handler)(U1, U2, U3), OBJ* obj, T1 t1, T2 t2, T3 t3);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the specified clock.
   *  @arg \c c Reference clock for latency.
   *  @arg \c handler 4 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list
   *  @arg \c t4 4th parameter in callback function list   
   *  \return Unique tick event id
   */      
  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3,
    typename U4, typename T4>
    static TickEventId ScheduleClock(Ticks_t t, Clock& c, void(T::*handler)(U1, U2, U3, U4), OBJ* obj, T1 t1, T2 t2, T3 t3, T4 t4);

  // This set are for "tick" events, that  are scheduled on integral half-ticks
  // with respect to the specified clock
  
  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the specified clock.
   *  @arg \c c Reference clock for latency.
   *  @arg \c handler 0 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  \return Unique tick event id
   */      
  template <typename T, typename OBJ>
    static TickEventId ScheduleClockHalf(Ticks_t t, Clock& c, void(T::*handler)(void), OBJ* obj);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the specified clock.
   *  @arg \c c Reference clock for latency.
   *  @arg \c handler 1 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list   
   *  \return Unique tick event id
   */
  template <typename T, typename OBJ,
    typename U1, typename T1>
    static TickEventId ScheduleClockHalf(Ticks_t t, Clock& c, void(T::*handler)(U1), OBJ* obj, T1 t1);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the specified clock.
   *  @arg \c c Reference clock for latency.
   *  @arg \c handler 2 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list    
   *  \return Unique tick event id
   */
  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2>
    static TickEventId ScheduleClockHalf(Ticks_t t, Clock& c, void(T::*handler)(U1, U2), OBJ* obj, T1 t1, T2 t2);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the specified clock.
   *  @arg \c c Reference clock for latency.
   *  @arg \c handler 3 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list   
   *  \return Unique tick event id
   */
  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3>
    static TickEventId ScheduleClockHalf(Ticks_t t, Clock& c, void(T::*handler)(U1, U2, U3), OBJ* obj, T1 t1, T2 t2, T3 t3);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the specified clock.
   *  @arg \c c Reference clock for latency.
   *  @arg \c handler 4 Parameter member function callback pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list
   *  @arg \c t4 4th parameter in callback function list   
   *  \return Unique tick event id
   */
  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3,
    typename U4, typename T4>
    static TickEventId ScheduleClockHalf(Ticks_t t, Clock& c, void(T::*handler)(U1, U2, U3, U4), OBJ* obj, T1 t1, T2 t2, T3 t3, T4 t4);

  // -------------------------------------------------------------------- //
  // This set is for member function callbacks with floating point time
  
  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in time with respect to the default clock.
   *  @arg \c handler 0 Parameter member function callback function pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  \return Unique time event id
   */  
  template <typename T, typename OBJ>
    static EventId ScheduleTime(double t, void(T::*handler)(void), OBJ* obj);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in time with respect to the default clock.
   *  @arg \c handler 1 Parameter member function callback function pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list
   *  \return Unique time event id
   */
  template <typename T, typename OBJ,
    typename U1, typename T1>
    static EventId ScheduleTime(double t, void(T::*handler)(U1), OBJ* obj, T1 t1);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in time with respect to the default clock.
   *  @arg \c handler 2 Parameter member function callback function pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  \return Unique time event id
   */
  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2>
    static EventId ScheduleTime(double t, void(T::*handler)(U1, U2), OBJ* obj, T1 t1, T2 t2);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in time with respect to the default clock.
   *  @arg \c handler 3 Parameter member function callback function pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list  
   *  \return Unique time event id
   */
  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3>
    static EventId ScheduleTime(double t, void(T::*handler)(U1, U2, U3), OBJ* obj, T1 t1, T2 t2, T3 t3);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in time with respect to the default clock.
   *  @arg \c handler 4 Parameter member function callback function pointer.
   *  @arg \c obj Object the callback function will be called on.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list
   *  @arg \c t4 4th parameter in callback function list    
   *  \return Unique time event id
   */
  template <typename T, typename OBJ,
    typename U1, typename T1,
    typename U2, typename T2,
    typename U3, typename T3,
    typename U4, typename T4>
    static EventId ScheduleTime(double t, void(T::*handler)(U1, U2, U3, U4), OBJ* obj, T1 t1, T2 t2, T3 t3, T4 t4);

  // --------------------------------------------------------------------- //
  // Schedulers for static callback functions with integral (ticks)  time
  // using the default clock

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the default clock.
   *  @arg \c handler 0 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list
   *  @arg \c t4 4th parameter in callback function list    
   *  \return Unique tick event id
   */  
  static TickEventId Schedule(Ticks_t t, void(*handler)(void));

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the default clock.
   *  @arg \c handler 1 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  \return Unique tick event id
   */      
  template <typename U1, typename T1>
    static TickEventId Schedule(Ticks_t t, void(*handler)(U1), T1 t1);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the default clock.
   *  @arg \c handler 2 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  \return Unique tick event id
   */      
  template <typename U1, typename T1,
            typename U2, typename T2>
    static TickEventId Schedule(Ticks_t t, void(*handler)(U1, U2), T1 t1, T2 t2);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the default clock.
   *  @arg \c handler 3 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list   
   *  \return Unique tick event id
   */      
  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3>
    static TickEventId Schedule(Ticks_t t, void(*handler)(U1, U2, U3), T1 t1, T2 t2, T3 t3);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the default clock.
   *  @arg \c handler 4 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list
   *  @arg \c t4 4th parameter in callback function list    
   *  \return Unique tick event id
   */      
  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3,
            typename U4, typename T4>
    static TickEventId Schedule(Ticks_t t, void(*handler)(U1, U2, U3, U4), T1 t1, T2 t2, T3 t3, T4 t4);

  // --------------------------------------------------------------------- //
  // Schedulers for static callback functions with half time
  // using the default clock
  
  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the default clock.
   *  @arg \c handler 0 Parameter static callback function pointer.
   *  \return Unique tick event id
   */      
  static TickEventId ScheduleHalf(Ticks_t t, void(*handler)(void));

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the default clock.
   *  @arg \c handler 1 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  \return Unique tick event id
   */
  template <typename U1, typename T1>
    static TickEventId ScheduleHalf(Ticks_t t, void(*handler)(U1), T1 t1);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the default clock.
   *  @arg \c handler 2 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  \return Unique tick event id
   */    
  template <typename U1, typename T1,
            typename U2, typename T2>
    static TickEventId ScheduleHalf(Ticks_t t, void(*handler)(U1, U2), T1 t1, T2 t2);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the default clock.
   *  @arg \c handler 3 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list
   *  \return Unique tick event id
   */    
  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3>
    static TickEventId ScheduleHalf(Ticks_t t, void(*handler)(U1, U2, U3), T1 t1, T2 t2, T3 t3);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the default clock.
   *  @arg \c handler 4 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list
   *  @arg \c t4 4th parameter in callback function list    
   *  \return Unique tick event id
   */    
  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3,
            typename U4, typename T4>
    static TickEventId ScheduleHalf(Ticks_t t, void(*handler)(U1, U2, U3, U4), T1 t1, T2 t2, T3 t3, T4 t4);

  // Schedulers for static callback functions with integral (ticks)  time
  // using the specified clock

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the specified clock.
   *  @arg The reference clock the latency is based on.
   *  @arg \c handler 0 Parameter static callback function pointer.
   *  \return Unique tick event id
   */    
  static TickEventId ScheduleClock(Ticks_t t, Clock&, void(*handler)(void));

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the specified clock.
   *  @arg The reference clock the latency is based on.
   *  @arg \c handler 1 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  \return Unique tick event id
   */
  template <typename U1, typename T1>
    static TickEventId ScheduleClock(Ticks_t t, Clock&, void(*handler)(U1), T1 t1);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the specified clock.
   *  @arg The reference clock the latency is based on.
   *  @arg \c handler 2 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  \return Unique tick event id
   */
  template <typename U1, typename T1,
            typename U2, typename T2>
    static TickEventId ScheduleClock(Ticks_t t, Clock&, void(*handler)(U1, U2), T1 t1, T2 t2);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the specified clock.
   *  @arg The reference clock the latency is based on.
   *  @arg \c handler 3 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list
   *  \return Unique tick event id
   */
  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3>
    static TickEventId ScheduleClock(Ticks_t t, Clock&, void(*handler)(U1, U2, U3), T1 t1, T2 t2, T3 t3);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in ticks with respect to the specified clock.
   *  @arg The reference clock the latency is based on.
   *  @arg \c handler 4 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list
   *  @arg \c t4 4th parameter in callback function list    
   *  \return Unique tick event id
   */
  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3,
            typename U4, typename T4>
    static TickEventId ScheduleClock(Ticks_t t, Clock&, void(*handler)(U1, U2, U3, U4), T1 t1, T2 t2, T3 t3, T4 t4);

  // Schedulers for static callback functions with half ticks time
  // using the specified clock

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the specified clock.
   *  @arg The reference clock the latency is based on.
   *  @arg \c handler 0 Parameter static callback function pointer.
   *  \return Unique tick event id
   */  
  static TickEventId ScheduleClockHalf(Ticks_t t, Clock&, void(*handler)(void));

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the specified clock.
   *  @arg The reference clock the latency is based on.
   *  @arg \c handler 1 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  \return Unique tick event id
   */      
  template <typename U1, typename T1>
    static TickEventId ScheduleClockHalf(Ticks_t t, Clock&, void(*handler)(U1), T1 t1);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the specified clock.
   *  @arg The reference clock the latency is based on.
   *  @arg \c handler 2 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  \return Unique tick event id
   */      
  template <typename U1, typename T1,
            typename U2, typename T2>
    static TickEventId ScheduleClockHalf(Ticks_t t, Clock&, void(*handler)(U1, U2), T1 t1, T2 t2);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the specified clock.
   *  @arg The reference clock the latency is based on.
   *  @arg \c handler 3 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list
   *  \return Unique tick event id
   */      
  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3>
    static TickEventId ScheduleClockHalf(Ticks_t t, Clock&, void(*handler)(U1, U2, U3), T1 t1, T2 t2, T3 t3);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in half ticks with respect to the specified clock.
   *  @arg The reference clock the latency is based on.
   *  @arg \c handler 4 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list
   *  @arg \c t4 4th parameter in callback function list    
   *  \return Unique tick event id
   */      
  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3,
            typename U4, typename T4>
    static TickEventId ScheduleClockHalf(Ticks_t t, Clock&, void(*handler)(U1, U2, U3, U4), T1 t1, T2 t2, T3 t3, T4 t4);

  // --------------------------------------------------------------------- //
  // Schedulers for static callback functions with floating point time
  
  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in time with respect to the default clock.
   *  @arg \c handler 0 Parameter static callback function pointer.
   *  \return Unique time event id
   */      
  static EventId ScheduleTime(double t, void(*handler)(void));

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in time with respect to the default clock.
   *  @arg \c handler 1 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  \return Unique time event id
   */
  template <typename U1, typename T1>
    static EventId ScheduleTime(double t, void(*handler)(U1), T1 t1);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in time with respect to the default clock.
   *  @arg \c handler 2 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  \return Unique time event id
   */
  template <typename U1, typename T1,
            typename U2, typename T2>
    static EventId ScheduleTime(double t, void(*handler)(U1, U2), T1 t1, T2 t2);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in time with respect to the default clock.
   *  @arg \c handler 3 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list
   *  \return Unique time event id
   */
  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3>
    static EventId ScheduleTime(double t, void(*handler)(U1, U2, U3), T1 t1, T2 t2, T3 t3);

  /** To Schedule an event for a component without going through a link, this 
   *  function can only be called on components within a rank(logical process).
   *  @arg \c t Latency in time with respect to the default clock.
   *  @arg \c handler 4 Parameter static callback function pointer.
   *  @arg \c t1 1st parameter in callback function list
   *  @arg \c t2 2nd parameter in callback function list 
   *  @arg \c t3 3rd parameter in callback function list
   *  @arg \c t4 4th parameter in callback function list    
   *  \return Unique time event id
   */
  template <typename U1, typename T1,
            typename U2, typename T2,
            typename U3, typename T3,
            typename U4, typename T4>
    static EventId ScheduleTime(double t, void(*handler)(U1, U2, U3, U4), T1 t1, T2 t2, T3 t3, T4 t4);


  // Static variables
  
private:
  
  /** A flag set to TRUE if simulation is distributed.
   */ 
  static bool       isDistributed;
  
  /** Stores the sorted pending events.
   */
  static EventSet_t events;
  
  /** A flag set to TRUE when the simulation is to be stopped.
   */
  static bool       halted;
  
  /** The current simulation time on this LP.
   */
  static double     simTime;
  
  //static Ticks_t    simTicks;
  //static Time_t     stopTime;  // Time to stop if non-zero
  //static Ticks_t    stopTicks; // Tick time to stop if non-zero
  //static Clock*     stopClock; // Clock to use for stop ticks

  struct LBTS_Msg {
    int epoch;
    int tx_count;
    int rx_count;
    int myId;
    Time_t smallest_time;
  };

#ifdef KERNEL_UTEST
public:
#endif
  static Time_t grantedTime;
  static bool isSafeToProcess(Time_t);
  static void handle_incoming_messages();

};

} //namespace kernel
} //namespace manifold

#endif
