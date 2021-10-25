//-----------------------------------------------------------------*- c++ -*-
//                         INRIA-OLSRNet/OLSRBase
//             Cedric Adjih, projet Hipercom, INRIA Rocquencourt
//  Copyright 2003-2006 Institut National de Recherche en Informatique et
//  en Automatique.  All rights reserved.  Distributed only with permission.
//---------------------------------------------------------------------------
// [Oct2006] Comes from: OOLSR/include/scheduler_generic.h
// [Apr2007] Comes from: OLSRNet-initial/OLSRBase/common/api-scheduler.h
//---------------------------------------------------------------------------

#ifndef _API_SCHEDULER_H
#define _API_SCHEDULER_H

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

/// It is a base class inherited by any class which want to be called
/// back by the scheduler. When the event put in the scheduler
/// is to be called, its method 'handleEvent' is called.
class IEvent
{
public:
  // Method which is called by the IScheduler, when the event is the
  // first in line.
  virtual void handleEvent(void* data) = 0;
  virtual ~IEvent() { }
};

/// An event identifier:
/// When an event is inserted, an event identifier is returned which
/// can be used to remove the event from the scheduler.
typedef int EventIdentifier;

/// IScheduler, a generic scheduler interface.
/// IEvent/s can be added, removed. The IScheduler schedules them in time
/// order.
class IScheduler
{
public:
  /// Add an event which is to be scheduler in current time plus relativeTime. 
  /// event.handleEvent will then be called with the 'data' which is
  /// passed here. The EventIdentifier which is returned can be used
  /// for removing an event before its expiration with the method removeEvent
  /// (event is owned)
  virtual EventIdentifier addEvent(Time relativeTime, IEvent* event,
				   void* data) = 0;

  /// Same as the method addEvent except that the time is absolute, not
  /// relative. 
  /// Requirement: absoluteTime must be greater or equal to the current time
  /// (event is owned)
  virtual EventIdentifier addEventAt(Time absoluteTime, IEvent* event,
				     void* data) = 0;

  /// Removes an event which was added, based on an identifier returned by
  /// addEvent or addEventAt
  /// Requirement: the Event must not have been scheduled yet.
  virtual void removeEvent(EventIdentifier eventIdentifier) = 0;

  /// Runs all the events until the current time is greater or equal
  /// to absoluteTime.
  virtual void runUntil(Time absoluteTime) = 0;

  /// Runs all the events until there is no event left in the queue
  /// (something which might never happen in some designs).
  virtual void runUntilNoEvent() = 0;

  /// Get the current time. It is greater or equal to the last event
  /// scheduled.
  virtual Time getTime() = 0;

  //protected:
  /// Returns the time at which the first event should be scheduled
  virtual Time getFirstEventTime() = 0;

  // Returns whether the scheduler has still events to be scheduled
  virtual bool hasEvent() = 0;

  // Write the content of the scheduler on the output
  virtual void write(ostream& out) = 0;

  virtual ~IScheduler() { }
};

//---------------------------------------------------------------------------

template <class Object>
class ObjectMethodCallback : public IEvent
{
public:

  typedef void (Object::*ObjectMethod)();
  typedef void (Object::*ObjectMethodArgVoid)(void*);
  

  ObjectMethodCallback(Object* anObject, ObjectMethodArgVoid aMethod)
    : object(anObject), method(aMethod) {}

  virtual void handleEvent(void* data) { (object->*method)(data); }
  virtual ~ObjectMethodCallback() {};

  Object* object;
  ObjectMethodArgVoid method;
};

template <class Object>
static inline IEvent* 
makeCallback(Object* object,
	     typename ObjectMethodCallback<Object>::ObjectMethodArgVoid method)
{ return new ObjectMethodCallback<Object>(object, method); }

//---------------------------------------------------------------------------

#endif // _API_SCHEDULER_H
