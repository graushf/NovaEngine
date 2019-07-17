// ------------------------------------------------------------------------------
// PhysicsEventListener.cpp - Implements the events sent FROM the physics system
// ------------------------------------------------------------------------------

//
// Basic, simple, physics event listener construct
//

#include "../Common/CommonStd.h"

#include "PhysicsEventListener.h"

#include "Physics.h"
#include "../EventManager/Events.h"
#include "../LUAScripting/LuaStateManager.h"

const EventType EvtData_PhysTrigger_Enter::sk_EventType(0x62e7edd8);
const EventType EvtData_PhysTrigger_Leave::sk_EventType(0xbb2647c7);
const EventType EvtData_PhysCollision::sk_EventType(0xa08f65a8);
const EventType EvtData_PhysSeparation::sk_EventType(0xc39086f7);

void EvtData_PhysCollision::VBuildEventData(void)
{
	m_eventData.AssignNewTable(LuaStateManager::Get()->GetLuaState());
	m_eventData.SetInteger("actorA", m_ActorA);
	m_eventData.SetInteger("actorB", m_ActorB);
}
