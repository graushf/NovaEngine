#pragma once

//========================================================================
// EventManager.h : Implements a multi-listener multi-sender event system
//========================================================================

#include <strstream>

#include "Multicore/CriticalSection.h"
#include "ThirdParty/FastDelegate/FastDelegate.h"
#include "Common/CommonStd.h"



// ----------------------------------------------
// Forward declarations & typedefs
// ----------------------------------------------
class IEventData;

typedef unsigned long EventType;
typedef std::shared_ptr<IEventData> IEventDataPtr;
typedef fastdelegate::FastDelegate1<IEventDataPtr> EventListenerDelegate;
typedef concurrent_queue<IEventDataPtr> ThreadSafeEventQueue;


// ----------------------------------------------
// Macro for event registration
// ----------------------------------------------
extern GenericObjectFactory<IEventData, EventType> g_eventFactory;
#define REGISTER_EVENT(eventClass) g_eventFactory.Register<eventclass>(eventClass::sk_EventType)
#define CREATE_EVENT(eventType) g_eventFactory.Create(eventType)


// ------------------------------------------------------------------------------
// IEventData							- Chapter 11, page 310
//
// Base type for event object hierarchy, may be used itself for simplest event
// notifications such as those that do not carry additional payload data. If
// any event needs to propagate with payload data it must be defined 
// separately.
// ------------------------------------------------------------------------------
class IEventData
{
public:
	virtual ~IEventData(void) { }
	virtual const EventType& VGetEventType(void) const = 0;
	virtual float GetTimeStamp(void) const = 0;
	virtual void VSerialize(std::ostringstream& out) const = 0;
	virtual void VDeserialize(std::istrstream& in) = 0;
	virtual IEventDataPtr VCopy(void) const = 0;
	virtual const char* GetName(void) const = 0;
};

// ------------------------------------------------------------------------------
// class BaseEventData						- Chapter 11, page 311
// ------------------------------------------------------------------------------
class BaseEventData : public IEventData
{
	const float m_timeStamp;

public:
	explicit BaseEventData(const float timeStamp = 0.0f) : m_timeStamp(timeStamp) { }

	// Returns the type of the event
	virtual const EventType& VGetEventType(void) const = 0;

	float GetTimeStamp(void) const { return m_timeStamp; }

	// Serializing for network input / output
	virtual void VSerialize(std::ostringstream& out) const { }
	virtual void VDeserialize(std::istrstream& in) { }
};