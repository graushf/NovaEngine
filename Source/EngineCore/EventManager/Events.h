#pragma once

//========================================================================
// EventManager.h : Defines common game events
//========================================================================

#include "EventManager.h"
#include "Common/CommonStd.h"
#include "App/App.h"


// ------------------------------------------------------------------------------------
// EvtData_DestroyActor - sent when actors are destroyed
// ------------------------------------------------------------------------------------
class EvtData_Destroy_Actor : public BaseEventData
{
	ActorId m_id;

public:
	static const EventType sk_EventType;

	explicit EvtData_Destroy_Actor(ActorId id = INVALID_ACTOR_ID)
		: m_id(id)
	{
		//
	}

	virtual const EventType& VGetEventType(void) const
	{
		return sk_EventType;
	}

	virtual IEventDataPtr VCopy(void) const
	{
		return IEventDataPtr(Nv_NEW EvtData_Destroy_Actor(m_id));
	}

	virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_id;
	}

	virtual void VDeserialize(std::istrstream& in)
	{
		in >> m_id;
	}

	virtual const char* GetName(void) const
	{
		return "EvtData_Destroy_Actor";
	}

	ActorId GetId(void) const { return m_id; }
};