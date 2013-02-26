// Manifold object implementation
// George F. Riley, (and others) Georgia Tech, Spring 2010

#include <stdio.h>
#include <stdlib.h>
#include "common-defs.h"
#include "manifold.h"
#include "component.h"
#ifndef NO_MPI
#include "messenger.h"
#endif


namespace manifold {
namespace kernel {

// Static variables
int        EventBase::nextUID = 0;
int        TickEventBase::nextUID = 0;
bool       Manifold::isDistributed = false;
EventSet_t Manifold::events;
bool       Manifold::halted = false;
double     Manifold::simTime = 0;
Time_t     Manifold::grantedTime = 0;
//Ticks_t    Manifold::simTicks = 0;
//Time_t     Manifold::stopTime = 0.0;
//Ticks_t    Manifold::stopTicks = 0;
//Clock*     Manifold::stopClock = nil;

// TickEvent0Stat is not a template, so we implement it here
void TickEvent0Stat::CallHandler()
{
  handler();
}

// Event0Stat is not a template, so we implement it here
void Event0Stat::CallHandler()
{
  handler();
}

// The static Schedule with no args is not a template, so implement here
   TickEventId Manifold::Schedule(Ticks_t t, void(*handler)(void))
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent0Stat(t, c, handler);
    c.Insert(ev);
    return TickEventId(Manifold::NowTicks() + t, ev->uid, c);
  }

// The static Schedule with no args is not a template, so implement here
   TickEventId Manifold::ScheduleHalf(Ticks_t t, void(*handler)(void))
  {
    Clock& c = Clock::Master();
    TickEventBase* ev = new TickEvent0Stat(t, c, handler);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

// The static Schedule with no args is not a template, so implement here
TickEventId Manifold::ScheduleClock(Ticks_t t, Clock& c, void(*handler)(void))
  {
    TickEventBase* ev = new TickEvent0Stat(t, c, handler);
    c.Insert(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

// The static Schedule with no args is not a template, so implement here
TickEventId Manifold::ScheduleClockHalf(Ticks_t t, Clock& c, void(*handler)(void))
  {
    TickEventBase* ev = new TickEvent0Stat(t, c, handler);
    c.InsertHalf(ev);
    return TickEventId(ev->time, ev->uid, c);
  }

// The static ScheduleTime with no args is not a template, so implement here
   EventId Manifold::ScheduleTime(double t, void(*handler)(void))
  {
    double future = t + Now();
    EventBase* ev = new Event0Stat(future, handler);
    events.insert(ev);
    return EventId(future, ev->uid);
  }


// Static methods
LpId_t Manifold::GetRank()
{
#ifndef NO_MPI
  return TheMessenger.get_node_id();
#else
  return 0;
#endif
  //if (!isDistributed) return 0;
  //return /* MPI_Rank() */ 0;
}

void Manifold::EnableDistributed()
{
  isDistributed = true;
  /* MPI_INIT() */
}

//====================================================================
//====================================================================
void Manifold::Init(int argc, char** argv)
{
#ifndef NO_MPI
  TheMessenger.init(argc, argv);
#endif
}

//====================================================================
//====================================================================
#if 0
void Manifold::Run(double macsim_simcycle)
{
  if(!events.empty())
    {
      EventSet_t::iterator i = events.begin();
      EventBase* ev = *i;  // Get the event
      // Set the simulation time
      simTime = macsim_simcycle;
	  while( simTime == ev->time)
	  {
		  ev->CallHandler();
		  // Remove the event from the pending list
		  events.erase(i);
		  // And delete the event
		  delete ev;
		  
		  if(events.empty())
		  	return;
		  	
		  i = events.begin();
		  ev = *i;  // Get the event
      }
    }
} 
#else
void Manifold::Run(double macsim_simcycle)
{
  //std::cout << "IRIS sim time is " << Manifold::NowTicks() << "\n";
  simTime = macsim_simcycle;

      // Get the time of the next timed event
      EventBase* nextEvent = nil;
      if (!events.empty())
      { 
         nextEvent = *events.begin();        
      }
      
      // Next we  need to find the clock object with the next earliest tick
      Clock* nextClock = nil;
      Time_t nextClockTime = INFINITY;
      #if 1
      Clock::ClockVec_t& clocks = Clock::GetClocks();
      for (size_t i = 0; i < clocks.size(); ++i)
        {
          if (!nextClock)
            { // First one
              nextClock = clocks[i];
              nextClockTime = nextClock->NextTickTime();
            }
          else
            {
              if (clocks[i]->NextTickTime() < simTime)
                {
                  nextClock = clocks[i];
                  nextClockTime = nextClock->NextTickTime();
                }
            }
        }

	if (nextEvent != nil)
	std::cout << "clock: omg nextEvent != NULL " << nextEvent->time << "\n";
	//std::cout << "clock: nextclocktime/simTime: " 
	//	<< nextClockTime << "/" << simTime << "\n";
	#endif
      double nextTime = nextClockTime; //time of next clock/timed event, whichever is earlier
      bool clockIsNext = true;  //the next clock event is earlier
      if(nextEvent != nil && nextEvent->time < simTime)
	{
	  nextTime = nextEvent->time;
	  clockIsNext = false;
	}

	// Now process either the next event or next clock tick
	// depending on which one is earliest           
	if(clockIsNext)
	{ // Process clock event
	    nextClock->ProcessThisTick();
	}
	else
	{ // Process timed event
	    // Set the simulation time
	    //simTime = nextEvent->time;
	    simTime = macsim_simcycle;
	    // Call the event handler
	    nextEvent->CallHandler();
	    // Remove the event from the pending list
	    events.erase(events.begin());
	    // And delete the event
	    delete nextEvent;
	}


}
#endif

void Manifold::Stop()
{
  halted = true;
}

void Manifold::StopAtTime(Time_t s)
{
  ScheduleTime(s, &Manifold::Stop);
}

void Manifold::StopAt(Ticks_t t)
{
  Schedule(t, &Manifold::Stop);
}

void Manifold::StopAtClock(Ticks_t t, Clock& c)
{
  ScheduleClock(t, c, &Manifold::Stop);
}

double Manifold::Now()
{
  return simTime;
}

Ticks_t Manifold::NowTicks()
{
  return Clock::Now();
}

Ticks_t Manifold::NowHalfTicks()
{
  return Clock::NowHalf();
}

Ticks_t Manifold::NowTicks(Clock& c)
{
  return Clock::Now(c);
}

Ticks_t Manifold::NowHalfTicks(Clock& c)
{
  return Clock::NowHalf(c);
}

bool Manifold::Cancel(EventId& evid)
{
  EventSet_t::iterator it = events.find(&evid);
  if (it == events.end()) return false; // Not found
  events.erase(it);                     // Otherwise erase it
  return true;
}

EventId Manifold::Peek()
{ // Return eventid for earliest event, but do not remove it
  // Event list must not be empty
  EventSet_t::iterator it = events.begin();
  return EventId((*it)->time, (*it)->uid);
}

EventBase* Manifold::GetEarliestEvent()
{
  EventSet_t::iterator it = events.begin();
  if (it == events.end()) return 0;
  return *it;
}


#ifndef NO_MPI
//====================================================================
//====================================================================
bool Manifold::isSafeToProcess(double requestTime)
{
    static LBTS_Msg* LBTS = new LBTS_Msg[TheMessenger.get_node_size()];

      if(requestTime <= grantedTime)
        {
	  return true;
        }
      else
        {
	  LBTS_Msg lbts_msg = {0,0,0,0,0};
	  lbts_msg.tx_count = TheMessenger.get_numSent();
	  lbts_msg.rx_count = TheMessenger.get_numReceived();
	  lbts_msg.smallest_time = requestTime;
	  int nodeId = TheMessenger.get_node_id();
	  LBTS[nodeId] = lbts_msg;

	  TheMessenger.allGather((char*)&(LBTS[nodeId]), sizeof(LBTS_Msg), (char*)LBTS);
	  //MPI_Allgather(&(LBTS[nodeId]), sizeof(LBTS_MSG), MPI_BYTE, LBTS, 
	//		sizeof(LBTS_MSG), MPI_BYTE, MPI_COMM_WORLD);
	  int rx=0;
	  int tx=0;
	  double smallest_time = LBTS[0].smallest_time;

	  for(int i=0; i<TheMessenger.get_node_size(); i++)
	    {
	      tx+=LBTS[i].tx_count;
	      rx+=LBTS[i].rx_count;

	      if(LBTS[i].smallest_time < smallest_time)
		{
		  smallest_time=LBTS[i].smallest_time;
		}
	    }
	  if(rx==tx)
	    {
	      //grantedTime=DistributedSimulator::smallest_time+LinkRTI::minTxTime;
	      grantedTime = smallest_time;
	      if(requestTime <= grantedTime)
		return true;
              else
	        return false;
	    }
	  else
	    {
	      return false;
	    }
	}
}

//====================================================================
//====================================================================
void Manifold :: handle_incoming_messages()
{
    int received = 0;
    do {
        Message_s& msg = TheMessenger.irecv_message(&received);
	if(received != 0) {
	    Component* comp = Component :: GetComponent<Component>(msg.compIndex);
	    switch(msg.type) {
	        case Message_s :: M_UINT32:
		    comp->Recv_remote(msg.inputIndex, msg.sendTick, msg.recvTick,
		                      msg.sendTime, msg.recvTime, msg.uint32_data);
		    break;
	        case Message_s :: M_UINT64:
		    comp->Recv_remote(msg.inputIndex, msg.sendTick, msg.recvTick,
		                      msg.sendTime, msg.recvTime, msg.uint64_data);
		    break;
	        case Message_s :: M_SERIAL:
		    comp->Recv_remote(msg.inputIndex, msg.sendTick, msg.recvTick,
		                      msg.sendTime, msg.recvTime, msg.data, msg.data_len);
		    break;
		default:
		    std::cerr << "unknown message type" << std::endl;
		    exit(1);
	    }//switch
	}
    }while(received != 0);
}
#endif //#ifndef NO_MPI


#ifdef KERNEL_UTEST
//IMPORTANT: must be called before any Schedule() calls.
void Manifold::unhalt()
{
   halted=false;
   Clock::ClockVec_t& clocks = Clock::GetClocks();
   for (size_t i = 0; i < clocks.size(); ++i) {
       //when simulation restarts, clock should be at rising edge
       clocks[i]->nextRising = true;
   }
}
#endif

} //namespace kernel
} //namespace manifold


