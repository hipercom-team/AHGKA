//-----------------------------------------------------------------*- c++ -*-
//                          INRIA-OLSRNet/OLSRBase
//             Cedric Adjih, projet Hipercom, INRIA Rocquencourt
//  Copyright 2003-2006 Institut National de Recherche en Informatique et
//  en Automatique.  All rights reserved.  Distributed only with permission.
//---------------------------------------------------------------------------
// [Oct2006] Comes from: OOLSR/src/scheduler_simulation.cc
// [Apr2007] Comes from: OLSRNet-initial/OLSRBase/common/scheduler-core.cc
//---------------------------------------------------------------------------
// XXX: This is an inefficient scheduler, but it should be working for now
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

// This means "no index" for an index in the eventList array


const int NoMinTimeIndex = -1;

SimulationScheduler::SimulationScheduler()
{ clock = 0; lastEvent = -1; }

EventIdentifier 
SimulationScheduler::addEvent(Time relativeTime, IEvent* event, void* data)
{ return addEventAt(relativeTime+getTime(), event, data); }

EventIdentifier
SimulationScheduler::addEventAt(Time absoluteTime, IEvent* event, void* data)
{
  int i;
  int limit = lastEvent+1 < MaxSchedulerEvent ?
    lastEvent+1 : MaxSchedulerEvent;
  if (absoluteTime < clock) {
    Warn("Scheduler going back in time: absoluteTime=" 
	 << absoluteTime << ", clock="
	 << clock);
    absoluteTime = clock;
  }
  for(i=0;i<limit;i++) 
    if (eventList[i].callback == NULL)
      break;

  if (i<MaxSchedulerEvent) {
    //    if (!(absoluteTime>=clock))
    // {
    //	int *crash=NULL;
    //	memcpy(crash,&i,sizeof(int));
    // }
    assert( absoluteTime >= clock);
    eventList[i].clock = absoluteTime;
    eventList[i].callback = event;
    eventList[i].data = data;
    if(i>lastEvent) lastEvent = i;
    return i;
  } else Fatal("Scheduler Event overflow\n");
}

void SimulationScheduler::removeEvent(EventIdentifier eventIdentifier)
{
  assert(eventList[eventIdentifier].callback != NULL); // otherwise 
  //delete eventList[i].callback;
  //eventList[i].callback = NULL;
  Fatal("Unimplemented yet XXX");
}

void SimulationScheduler::runUntil(Time absoluteTime)
{

  for(;;) {
    int index = getMinTimeIndex();
    if (index == NoMinTimeIndex) 
      return;
    assert( clock <=  eventList[index].clock ); // Don't back go in time. XXX: warn
    if (eventList[index].clock > absoluteTime)
      return;
    clock = eventList[index].clock;
    eventList[index].callback->handleEvent(eventList[index].data);
    delete eventList[index].callback;
    eventList[index].callback = NULL;
  }
}

void SimulationScheduler::runUntilNoEvent()
{ runUntil(DBL_MAX); }

Time SimulationScheduler::getTime()
{ return clock; }

int SimulationScheduler::getMinTimeIndex()
{
  double minTime = 0.0;
  bool hasMinTime = false;
  int result = NoMinTimeIndex;
  int limit = lastEvent+1 < MaxSchedulerEvent ?
    lastEvent+1 : MaxSchedulerEvent;
  for(int i=0;i<limit;i++) {
    if (eventList[i].callback != NULL) {
      if (!hasMinTime || (eventList[i].clock < minTime)) {
	minTime = eventList[i].clock;
	result = i;
	hasMinTime = true;
      }
    }
  }
  return result;
}

Time SimulationScheduler::getFirstEventTime()
{
  int result = getMinTimeIndex();
  assert( result != NoMinTimeIndex );
  return eventList[result].clock;
}

void SimulationScheduler::write(ostream& out)
{
  int limit = lastEvent+1 < MaxSchedulerEvent ?
    lastEvent+1 : MaxSchedulerEvent;
  out << "[";
  bool isFirst = true;
  for(int i=0;i<limit;i++) {
    if (eventList[i].callback != NULL) {
      if (!isFirst) out << ",";
      else isFirst = false;
      out << eventList[i].clock;
    }
  }  
  out << "]";
}

//---------------------------------------------------------------------------
