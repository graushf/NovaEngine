//========================================================================
// Events.cpp 
//========================================================================

#include "Common/CommonStd.h"

#include "Events.h"
#include "../Physics/PhysicsEventListener.h"
#include "../LUAScripting/ScriptEvent.h"

// To define a new event - you need a 32-bit GUID.
// In Visual Studio, go to Tools->Create GUID and grab the first bit

// {8D7EB248-BC69-4D9F-B294-6DC53D748CED}

const EventType EvtData_Destroy_Actor::sk_EventType(0x8d7eb248);

const EventType EvtData_PlaySound::sk_EventType(0xace8a1c1);

bool EvtData_PlaySound::VBuildEventFromScript(void)
{
	if (m_eventData.IsString())
	{
		m_soundResource = m_eventData.GetString();
		return true;
	}
	return false;
}

void RegisterEngineScriptEvents(void)
{
	REGISTER_SCRIPT_EVENT(EvtData_Request_Destroy_Actor, EvtData_Request_Destroy_Actor::sk_EventType);
	REGISTER_SCRIPT_EVENT(EvtData_PhysCollision, EvtData_PhysCollision::sk_EventType);
	REGISTER_SCRIPT_EVENT(EvtData_PlaySound, EvtData_PlaySound::sk_EventType);
}