// Implementation of Clock objects for Manifold
// George F. Riley, (and others) Georgia Tech, Fall 2010

#include "clock.h"
#include "manifold.h"
#include <list>
#include <cstdlib>

using namespace std;

namespace manifold {
namespace kernel {

// Map of all clocks
Clock::ClockVec_t* Clock::clocks = 0;

Clock::Clock(double f) : period(1/f), freq(f), nextRising(true), nextTick(0),
                         calendar(CLOCK_CALENDAR_LENGTH, EventVec_t())
{
  if (!clocks)
    {
      clocks = new ClockVec_t;
    }

  clocks->push_back(this);
}

void Clock::Rising()
{ // Call rising edge function on all registered objects
  list<tickObjBase*>::iterator iter;
  for(iter=tickObjs.begin(); iter!=tickObjs.end(); iter++)
    {
      tickObjBase* to = *iter;
      if (to->enabled) to->CallRisingTick();
    }
}

void Clock::Falling()
{ // Call falling edge function on all registered objects
  list<tickObjBase*>::iterator iter;
  for(iter=tickObjs.begin(); iter!=tickObjs.end(); iter++)
    {
      tickObjBase* to = *iter;
      if (to->enabled) to->CallFallingTick();
    }
}

TickEventId Clock::Insert(TickEventBase* ev)
{
  Ticks_t when = nextTick + ev->time;
  // First make sure it's in range for the calendar queue
  if (ev->time >= CLOCK_CALENDAR_LENGTH)
    { // Just insert into the sorted queue
      // The sorted queue uses units of half-ticks
      when = when * 2;
      if (!ev->rising) when++;
      ev->time = when; // Convert the time in the event to half ticks
      events.insert(make_pair(when, ev));
    }
  else
    { // Add to calendar
      ev->time = when;  // Convert ticks time to absolute from relative
      size_t index = when % CLOCK_CALENDAR_LENGTH;
      EventVec_t& eventVec = calendar[index];
      eventVec.push_back(ev);
    }
  return TickEventId((Ticks_t)ev->time,(int)ev->uid,(Clock&)*this);
}

TickEventId Clock::InsertHalf(TickEventBase* ev)
{ // Time is in  units of half-ticks.  If odd, toggle rising/falling
/*
  Ticks_t when = nextTick + ev->time;
  if (when & 0x01) ev->rising = !nextRising;
  // And convert time to full tick units
  ev->time = ev->time / 2;
  return Insert(ev);
*/
  Ticks_t thisTick = nextTick * 2;
  if (!nextRising) thisTick++;   //thisTick is the number of half ticks
  Ticks_t when = thisTick + ev->time;
  if (ev->time & 0x01) ev->rising = !nextRising;
  else ev->rising = nextRising;

  // And convert time to RELATIVE (relative to now, i.e, nextTick) full tick units
  ev->time = when / 2 - nextTick;
  return Insert(ev);
}

void Clock::Cancel(TickEventId)
{ // Implement later
}

Time_t Clock::NextTickTime() const
{
  Time_t time = (Time_t)nextTick / freq;
  if (!nextRising) time += period/ 2.0; // The falling is 1/2 period later
  return time;
}

Ticks_t Clock::NowTicks() const
{
  return nextTick;
}

Ticks_t Clock::NowHalfTicks() const
{
  return nextTick * 2 + (nextRising ? 0 : 1);
}

void Clock::ProcessThisTick()
{
  // First see if any events in the sorted queue need processing
  if (!events.empty())
    {
      // This structure uses half-tick units, so we can mix and
      // match rising and falling edge events
      Ticks_t thisTick = nextTick * 2;
      if (!nextRising) thisTick++; 
      while(true)
        {
          TickEventBase* ev = events.begin()->second;
          if (ev->time > thisTick) break; // Not time for this event
          // Process the event
          ev->CallHandler();
          // Delete the event
          delete ev;
          // Remove from queue
          events.erase(events.begin());
        }
    }
  // Now process all events
  Ticks_t thisIndex = nextTick % CLOCK_CALENDAR_LENGTH;
  EventVec_t& events = calendar[thisIndex];
  for (size_t i = 0; i < events.size(); ++i)
    {
      TickEventBase* ev = events[i];
      if (!ev) continue; // Already processed
      // make sure the event matches the rising/falling
      if (!ev->rising && nextRising) continue;
      if (ev->rising && !nextRising) continue;
      // Process the event
      ev->CallHandler();
      delete ev;
      events[i] = nil;
    }
  if (!nextRising)
    {
      // Clear these events from the vector
      events.clear();
    }
  // Finally call the Tick function on all registered objects.
  if (nextRising)
    {
      Rising();
      nextRising = false;
    }
  else
    {
      Falling();
      // After falling edge, advance to next tick time
      nextRising = true;
      nextTick++;
    }
}

// Static functions
Clock& Clock::Master()
{
  if (clocks->empty())
    {
      cout << "FATAL ERROR, Clock::Master called with no clocks" << endl;
      exit(1);
    }
  return *((*clocks)[0]);
}

Ticks_t Clock::Now()
{
  Clock& c = Master();
  return c.NowTicks();
}

Ticks_t Clock::NowHalf()
{
  Clock& c = Master();
  return c.NowHalfTicks();
}

Ticks_t Clock::Now(Clock& c)
{
  return c.NowTicks();
}

Ticks_t Clock::NowHalf(Clock& c)
{
  return c.NowHalfTicks();
}

Clock::ClockVec_t& Clock::GetClocks()
{
  if (!clocks)
    {
      clocks = new ClockVec_t;
    }
  return *clocks;
}

// Private helpers

} //namespace kernel
} //namespace manifold
  

