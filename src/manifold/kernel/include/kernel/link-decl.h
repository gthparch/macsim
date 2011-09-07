/** @file link-decl.h
 *  This file defines the Link class and subclasses.  The implementation
 *  is in link.h.
 */

// George F. Riley, (and others) Georgia Tech, Fall 2010 

#ifndef __LINK_H__
#define __LINK_H__


#include <iostream>
#include <stdint.h>
#include <vector>

#include "common-defs.h"


namespace manifold {
namespace kernel {

class Clock;

/** Base class for objects keeping track of link arrival handlers.
 */
template <typename T>
class LinkOutputBase 
{
public:

 /** Constructor
  *  @arg \c lat The latency in ticks.
  *  @arg \c ii The input index(port) number of the receiving component.
  *  @arg \c c A pointer to the reference clock for ticks.
  *  @arg \c delay The latency in time.
  *  @arg \c isTimed Flag to determine which latency to use.
  *  @arg \c isHalf Flag to determine if the ticks latency is half ticks.    
  */
 LinkOutputBase(Ticks_t lat, int ii, Clock* c, Time_t delay,
                bool isTimed, bool isHalf) 
   : latency(lat), inputIndex(ii), clock(c),
    timeLatency(delay), timed(isTimed), half(isHalf) {}
    
  /** Virutal function that schedules a receive event occurence.
   */    
  virtual void ScheduleRxEvent() = 0;
  
  /** The Send function called on a link to send data by a link
   * @arg \c t The actual data to send by the link
   */  
  void    Send(const T& t)
  {
    data = t;
    ScheduleRxEvent();
  }

  void    SendTick(const T& t, Ticks_t delay)
  {
    data = t;
    Ticks_t default_latency = latency;
    latency = delay;
    ScheduleRxEvent();
    latency = default_latency;
  }
public:

  /** The data that will be sent by the link
   */
  T       data;
  
  /** Latency of the link in ticks
   */
  Ticks_t latency;
  
  /** The input index(port) number of the receiving component
   */
  int     inputIndex;
  
  /** Reference clock pointer for ticked delays
   */
  Clock*  clock;
  
  /** The latency of the link in time
   */
  Time_t  timeLatency;
  
  /** True if latency is in time
   */
  bool    timed;       
  
  /** True if latency in half ticks
   */
  bool    half;        
};


template <typename T1> class Link;

/** LinkOutput class subclasses LinkOutputBase, templated
 *  with T1 (type of data to send) and OBJ (type of receiver)
 */
template <typename T1, typename OBJ>
class LinkOutput : public LinkOutputBase<T1>
{
public:

 /** Constructor
  *  @arg \c link A ptr to the link object this output refers to.
  *  @arg \c f The 2 param callback function ptr used upon a link arrival event.
  *  @arg \c o Ptr to the object the callback function will be called on.
  *  @arg \c latency The latency in terms of ticks for this output.
  *  @arg \c inputIndex The input index(port) number of the receiving component
  *  @arg \c c Ptr to the referenced clock used for ticked latency.
  *  @arg \c d The latency in terms of time for this output.
  *  @arg \c isT True if the latency is in time.
  *  @arg \c isH True if the latency is in half ticks.               
  */
 LinkOutput(Link<T1>* link, void(OBJ::*f)(int, T1), OBJ* o,
            Ticks_t latency, int inputIndex, Clock* c, Time_t d, 
            bool isT, bool isH)
   :   LinkOutputBase<T1>(latency, inputIndex, c, d, isT, isH),
    obj(o), handler(f), pLink(link) {}
    
  /** Function to schedule a receive event
   */
  void ScheduleRxEvent();
private:

  /** Object the callback function is called on.
   */
  OBJ* obj;
  
  /** Callback function ptr used upon a link arrival event.
   */
  void(OBJ::*handler)(int, T1);
  
  /** Points to the link this output is associated with
   */
  Link<T1>* pLink;  
};


#ifndef NO_MPI
/** LinkOutputRemote class subclasses LinkOutputBase, templated
 *  with T (type of data to send). This is created when the receiving
 *  component is in a different LP.
 */
template <typename T>
class LinkOutputRemote : public LinkOutputBase<T>
{
public:

  /** Constructor
   *  @arg \c link A ptr to the link object this output refers to.
   *  @arg \c compIdx Component index of the receiving component.
   *  @arg \c inputIndex The input index(port) number of the receiving component
   *  @arg \c c Ptr to the referenced clock used for ticked latency.
   *  @arg \c latency The latency in terms of ticks for this output.
   *  @arg \c d The latency in terms of time for this output.
   *  @arg \c isT True if the latency is in time.
   *  @arg \c isH True if the latency is in half ticks.               
   */
  LinkOutputRemote(Link<T>* link, int compIdx, int inputIndex,
                   Clock* c, Ticks_t latency, Time_t d, 
                   bool isT, bool isH);

    
  /** Function to schedule a receive event
   */
  void ScheduleRxEvent();

private:
  /** destination LP ID
   */
  LpId_t dest;

  /** Component index of receiving component.
   */
  CompId_t compIndex;
  
  /** Points to the link this output is associated with
   */
  Link<T>* pLink;  
};



/** Base class for LinkInput. Since the component keeps a vector
 *  of LinkInput objects, we need an non-templated base class.
 */
class LinkInputBase
{
public:
  /** Constructor
   *  @arg \c inputIdx Index of the input.
   *  @arg \c c Clock associated with the input.
   *  @arg \c isTimed True if the latency is in time.
   *  @arg \c isHalf True if the latency is in half ticks.               
   */
  LinkInputBase(int inputIdx, Clock* c, bool isTimed, bool isHalf) :
        inputIndex(inputIdx), clock(c), timed(isTimed), half(isHalf) {}

  template<typename T>
  void Recv(Ticks_t tick, Time_t time, T& data);

  virtual void Recv(Ticks_t tick, Time_t time, unsigned char* data, int len) = 0;

protected:
  int inputIndex;
  Clock* clock;
  bool timed;
  bool half;
};


//The following 2 templated Serialize() functions are created to solve a compiler
//problem occurring when the data sent between 2 components is a pointer type,
//such as MyType*. In this case, calling T :: Deserialize() directly in Recv()
//below would cause a compile problem.
template <typename T>
T Deserialize(const T&, unsigned char* buf)
{
  return T :: Deserialize(buf, 0); 
}

template <typename T>
T* Deserialize(T*, unsigned char* buf)
{
  return T :: Deserialize(buf); 
}

/** The sole purpose of this class is to act as a link in the inheritance chain
 *  between LinkInputBase and LinkInput. The root cause of the what makes this
 *  necessary is that templated functions can not be virtual. In Recv() eventually
 *  we want to call ScheduleRxEvent() and somehow pass the templated data. If
 *  ScheduleRxEvent() is tempalted, then it cannot be virtual and be overridden by
 *  LinkInput.
 */
template <typename T>
class LinkInputBaseT : public LinkInputBase
{
public:
  LinkInputBaseT(int inputIndex, Clock* c, bool isT, bool isH)
   :   LinkInputBase(inputIndex, c, isT, isH){}
  void set_data(T& data)
  {
    this->data = data;
  }

  void Recv(Ticks_t tick, Time_t time, unsigned char* data, int len)
  {
    //Cannot call T :: Deserialize() directly, because if T is a pointer type such as
    //MyType*, then compiler would complain Deserialize() is not a member of MyType*.
    //Therefore, we created 2 template functions above to solve this problem.
    this->data = Deserialize(this->data, data);
    ScheduleRxEvent(tick, time);
  }

  virtual void ScheduleRxEvent(Ticks_t tick, Time_t time) = 0;

protected:
  T data;
};


/** A LinkInput is created when the sending component is in a different LP. It stores
 *  some info so that an input data can be processed. The info includes pointer to Clock,
 *  as well as the data type. The data type is stored by making LinkInput a template
 *  class.
 */
template <typename T, typename OBJ>
class LinkInput : public LinkInputBaseT<T>
{
public:

 /** Constructor
  *  @arg \c f The 2 param callback function ptr used upon a link arrival event.
  *  @arg \c o Ptr to the object the callback function will be called on.
  *  @arg \c inputIndex The input index(port) number of the receiving component
  *  @arg \c c Ptr to the referenced clock used for ticked latency.
  *  @arg \c isTimed True if the latency is in time.
  *  @arg \c isHalf True if the latency is in half ticks.               
  */
 LinkInput(void(OBJ::*f)(int, T), OBJ* o, int inputIndex, Clock* c,
            bool isTimed, bool isHalf)
   :   LinkInputBaseT<T>(inputIndex, c, isTimed, isHalf),
    obj(o), handler(f) {}
    
  void ScheduleRxEvent(Ticks_t, Time_t);

private:
  /** Object the callback function is called on.
   */
  OBJ* obj;
  
  /** Callback function ptr used upon a link arrival event.
   */
  void(OBJ::*handler)(int, T);
};

#endif //#ifndef NO_MPI


/** LinkBase class is the base class for all links
 */
class LinkBase
{
public:
  
  /** Empty constructor
   */
  LinkBase(){}
  
  /** Virtual Destructor
   *  Needs polymorphic for dynamic_cast
   */
  virtual ~LinkBase() {}
  
  /** Function used to send data on the link.
   *  @arg The actual data to be sent.
   */
  template<typename T>
  void Send(T t);
  template<typename T>
  void SendTick(T t, Ticks_t delay);
  
#ifdef MOVED_TO_IMPL
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
#endif

};

/** \class Link Link class subclasses LinkBase
 */ 
template <typename T>
class Link : public LinkBase
{
public:

  /** Constructor
   */
  Link() {}
  
  /** Adds an output to this Link, a link can be one to many.
   *  @arg \c f Callback function ptr for this output on an event arrival. 
   *  @arg \c o Object the callback function will be called on.
   *  @arg \c l Latency in ticks of this output. 
   *  @arg \c ii Input index(port) number on the receiving component to use.
   *  @arg \c c Reference clock used for latency in ticks. 
   *  @arg \c delay Latency of the link in terms of time.
   *  @arg \c isTimed True if the latency in time is used for this link.
   *  @arg \c isHalf True if the latency in ticks is acutally half ticks.
   */
  template <typename O> void AddOutput(void(O::*f)(int, T), O* o,
                                       Ticks_t l, int ii,
                                       Clock* c,
                                       Time_t delay = 0.0,
                                       bool isTimed = false,
                                       bool isHalf = false);


#ifndef NO_MPI
  /** Adds an output to this Link, the other component is in a different LP.
   *  @arg \c compIdx Index of the other component.
   *  @arg \c inputIndex Input index(port) number on the receiving component to use.
   *  @arg \c c Reference clock used for latency in ticks. 
   *  @arg \c l Latency in ticks of this output. 
   *  @arg \c delay Latency of the link in terms of time.
   *  @arg \c isTimed True if the latency in time is used for this link.
   *  @arg \c isHalf True if the latency in ticks is acutally half ticks.
   */
  void AddOutputRemote(CompId_t compIdx, int inputIndex, Clock* c,
                       Ticks_t l,
                       Time_t delay = 0.0,
                       bool isTimed = false,
                       bool isHalf = false);
#endif //#ifndef NO_MPI

public:
  
  /** Vector of outputs, templated with T (the type of output data).
   *  Rules: The output type (T) is constant for all output targets.
   */
  std::vector<LinkOutputBase<T>*> outputs;

};


} //namespace kernel
} //namespace manifold


#endif
