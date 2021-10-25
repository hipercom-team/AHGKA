//-----------------------------------------------------------------*- c++ -*-
//                         INRIA-OLSRNet/OLSRBase
//             Cedric Adjih, projet Hipercom, INRIA Rocquencourt
//  Copyright 2003-2006 Institut National de Recherche en Informatique et
//  en Automatique.  All rights reserved.  Distributed only with permission.
//---------------------------------------------------------------------------
// [Oct2006] Comes from: OOLSR/include/scheduler_simulation.h
// [Apr2007] Comes from: OLSRNet-initial/OLSRBase/common/scheduler-core.h
//---------------------------------------------------------------------------

#ifndef _SCHEDULER_CORE_H
#define _SCHEDULER_CORE_H

//---------------------------------------------------------------------------

/// SimulationScheduler: simply an implementation of the AbstractScheduler
/// interface. It doesn't schedule in real time, instead, it schedules
/// sequentially each time the event with the lowest time.

const int MaxSchedulerEvent = 40960;

class SimulationScheduler : public IScheduler
{
public:
  SimulationScheduler();

  // -- implementations of IScheduler
  virtual EventIdentifier addEvent(Time relativeTime, 
				   IEvent* event, void* data);
  virtual EventIdentifier addEventAt(Time absoluteTime, IEvent* event,
				     void* data);
  virtual void removeEvent(EventIdentifier eventIdentifier);
  virtual void runUntil(Time absoluteTime);
  virtual void runUntilNoEvent();
  virtual Time getTime();
  //protected:
  virtual Time getFirstEventTime();
  virtual bool hasEvent() { return getMinTimeIndex() >= 0; /* XXX */ }
  virtual void write(ostream& out);

  // -- end of IScheduler

  void setTime(Time newClock)
  { clock = newClock; }
  
protected:
  // Returns the index of the first event to be scheduled
  int getMinTimeIndex();

  typedef struct {
    double clock;
    IEvent* callback;
    void* data;
  } SchedulerEvent;
  
  SchedulerEvent eventList[MaxSchedulerEvent];
  Time clock;
  int lastEvent;
};

//---------------------------------------------------------------------------

#endif // _SCHEDULER_CORE
