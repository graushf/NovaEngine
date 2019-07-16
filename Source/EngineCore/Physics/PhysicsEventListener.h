#pragma once

// ------------------------------------------------------------------------------
// PhysicsEventListener.h - Implements the events sent FROM the physics system
// ------------------------------------------------------------------------------

#include "../Common/CommonStd.h"

#include "../EventManager/EventManager.h"
#include "../App/App.h"
#include "../LUAScripting/ScriptEvent.h"

//
// Physics event implementation
//

class EvtData_PhysTrigger_Enter : public BaseEventData
{
	int m_triggerID;
	ActorId m_other;

public:
	static const EventType sk_EventType;

	virtual const EventType& VGetEventType(void) const
	{
		return sk_EventType;
	}

	EvtData_PhysTrigger_Enter()
	{
		m_triggerID = -1;
		m_other = INVALID_ACTOR_ID;
	}

	explicit EvtData_PhysTrigger_Enter(int triggerID, ActorId other)
		: m_triggerID(triggerID),
		  m_other(other)
	{}

	IEventDataPtr VCopy() const
	{
		return IEventDataPtr(Nv_NEW EvtData_PhysTrigger_Enter(m_triggerID, m_other));
	}

	virtual const char* GetName(void) const
	{
		return "EvtData_PhysTrigger_Enter";
	}

	int GetTriggerId(void) const
	{
		return m_triggerID;
	}

	ActorId GetOtherActor(void) const
	{
		return m_other;
	}

};

class EvtData_PhysTrigger_Leave : public BaseEventData
{

};

class EvtData_PhysCollision : public ScriptEvent
{

};

class EvtData_PhysSeparation : public BaseEventData
{

};
