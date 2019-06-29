#pragma once

//========================================================================
// ScriptEvent.h : 
//========================================================================

// --------------------------------------------------------------------------------------------------------
// DOCUMENTATION
// 
// Chapter 12. page 380.
//
// =========================================
// Script Events: Overview
//
// This system provides the glue that allows events to be sent to or received by the script. there is some 
// C++ setup involved but once done, event passing between the boundary becomes completely transparent.
//
// =========================================
// Setup (C++):
//
// In order to make an event able to be seen & processed by the script, you must follow four steps:
//			1) Inherit your event class from ScriptEvent
//			2) Implement VBuildEevntData() and VBuildEventFromScript() if necessary (see the
//				comments above their prototypes for details).
//			3) Call the EXPORT_FOR_SCRIPT_EVENT() macro in your event subclass declaration.
//			4) Call the REGISTER_SCRIPT_EVENT() macro in Application::RegisterScriptEvents() or the 
//				game-specific subclass override, as appropiate.
//
// Once done, your event should be visible to the script. In script, the EventType table will contain
// a member indexed by the event type string containing the event id. This id matches the EventType for the
// appropriate event.
//
// =========================================
// Sending Events from Script:
//
// To send an event from script, you will need to create a table that contains the necessary data, then call
// the exported QueueEvent() or SendEvent() function. The first parameter is the event type (from the 
// EventType table) and the second parameter is the table.
//
// In C++, this table is assigned to the m_eventData variable and used by your VBuildEventFromScript()
// implementation to fill out your C++ members. If something goes wrong, return false and the event won't 
// be sent. If you don't need to receive the event on the C++ side, it doesn't matter and you can keep the
// default implementation of VBuildEventFromScript().
//
// Events are received on the C++ side as normal. Inherit from EventListener and register for the appropriate event
// type. Your receiver doesn't know nor does it care where the event came from.
//
// =========================================
// Receiving Events in Script:
//
// To register an event listener in script, call the exported RegisterEventListener() function. The first parameter
// is the event type (from the EventType table) and the second parameter is the callback function that's called
// every time the event is triggered. The callback function should take a table as its only parameter and return 
// nothing:				function EventCallback(eventData)
//
// RegisterEventListener() returns an id to your listener. When you no longer wish to receive an event, call the 
// exported RemoveEventListener() function and pass it this id. If you don't need to ever stop listening for the
// event, it's safe to discard the id and let C++ clean it up on program exit. If your function goes out of scope,
// you will still incur the performance cost of receiving the event, though no other ill effects will occur.
//
// Internally on the C++ side, a ScriptEventListener object is created and registered as a proxy for each script
// listener. When it receives the event, it calls your ScriptEvent::VBuildEventData() to build the m_eventData
// table. It then calls your registered script function, passing this table in as a parameter. Note that if you
// don't need the event to be able to be received by the script, you don't need to override VBuildEventData().
//
// --------------------------------------------------------------------------------------------------------

#include "EventManager/EventManager.h"
#include "LuaPlus.h"

class ScriptEvent;
typedef ScriptEvent* (*CreateEventForScriptFunctionType)(void); // function ptr typedef to create a script event

// --------------------------------------------------------------------------------------------------------
// These macros implement exporting events to script.
// --------------------------------------------------------------------------------------------------------
#define REGISTER_SCRIPT_EVENT(eventClass, eventType) \
	ScriptEvent::RegisterEventTypeWithScript(#eventClass, eventType); \
	ScriptEvent::AddCreationFunction(eventType, &eventClass::CreateEventForScript)

#define EXPORT_FOR_SCRIPT_EVENT(eventClass) \
	public: \
		static ScriptEvent* CreateEventForScript(void) \
		{ \
			return new eventClass; \
		}

// --------------------------------------------------------------------------------------------------------
// Script event base class. This class is meant to be subclassed by any event that can be sent or received by the
// script. Note that these events are not limited to script and can be received just fine by C++ listeners.
// Furthermore, since the Script data isn't built unless being received by a script listener, there's no worry
// about performance.
// --------------------------------------------------------------------------------------------------------
class ScriptEvent : public BaseEventData
{
	typedef std::map<EventType, CreateEventForScriptFunctionType> CreationFunctions;
	static CreationFunctions s_creationFunctions;

	bool m_eventDataIsValid;

protected:
	LuaPlus::LuaObject m_eventData;

public:
	// construction
	ScriptEvent(void) { m_eventDataIsValid = false; }

	// script event data, which should only be called from the appropriate ScriptExports functions
	LuaPlus::LuaObject GetEventData(void);		// called when event is sent from C++ to script
	bool SetEventData(LuaPlus::LuaObject eventData);	// called when event is sent from script to C++

	// Static helper functions for registering events with the script. You should call the REGISTER_SCRIPT_EVENT()
	// macro instead of calling this function directly. Any class that needs to be exported also needs to call the
	// EXPORT_FOR_SCRIPT_EVENT() inside the class declaration.
	static void RegisterEventTypeWithScript(const char* key, EventType type);
	static void AddCreationFunction(EventType type, CreateEventForScriptFunctionType pCreationFunctionPtr);
	static ScriptEvent* CreateEventFromScript(EventType type);

protected:
	// This function must be overriden if you want to fire this event from C++ and have it received by the script.
	// If you only fire the event from the script side, this function will never be called. It's purpose is to 
	// fill in the m_eventData member, which is then passed to the script callback function in the listener. This
	// is only called the first time GetEventData() is called. If the event is script-only, this function does not need
	// to be overriden.
	virtual void VBuildEventData(void);

	// This function must be overriden if you want to fire this event from Script and have it received by C++. If
	// you only fire this event from script and have it received by the script, it doesn't matter since m_eventData
	// will just be passed straight through. It's purpose is to fill in any C++ member variables using the data in
	// m_eventData (which is valid at the time of the call). It is called when the event is fired from the script.
	// Return false if the data is invalid in some way, which will keep the event from actually firing.
	virtual bool VBuildEventFromScript(void) { return true; }
};