//========================================================================
// ScriptExports.cpp : 
//========================================================================

#include "Common/CommonStd.h"
#include "ScriptExports.h"
#include "ScriptEvent.h"
#include "LuaStateManager.h"
#include "EventManager/Events.h"
#include "ResourceCache/ResCache.h"
#include <set>



// ---------------------------------------------------------------------------------------------------------------------
// This is the C++ listener proxy for script event listeners. It pairs a single event type with a Lua callback
// function. Note that this event can be defined in C++ or Lua. It may also be sent from C++ or Lua.
//
// The Lua callback function should take in a table with the event data. The return value is ignored.
// function Callback(eventData)
//
// Chapter 12, page 384
// ---------------------------------------------------------------------------------------------------------------------
class ScriptEventListener
{
	EventType m_eventType;
	LuaPlus::LuaObject m_scriptCallbackFunction;

public:
	explicit ScriptEventListener(const EventType& eventType, const LuaPlus::LuaObject& scriptCallbackFunction);
	~ScriptEventListener(void);
	EventListenerDelegate GetDelegate(void) { return fastdelegate::MakeDelegate(this, &ScriptEventListener::ScriptEventDelegate); }
	void ScriptEventDelegate(IEventDataPtr pEventPtr);
};

// ---------------------------------------------------------------------------------------------------------------------
// This class manages the C++ ScriptListener objects needed for script event listeners.
// Chapter 12, page 385
// ---------------------------------------------------------------------------------------------------------------------
class ScriptEventListenerMgr
{
	typedef std::set<ScriptEventListener*> ScriptEventListenerSet;
	ScriptEventListenerSet m_listeners;

public:
	~ScriptEventListenerMgr(void);
	void AddListener(ScriptEventListener* pListener);
	void DestroyListener(ScriptEventListener* pListener);
};


// -------------------------------------------------------------------------------------------
// Prototypes for the functions to export.						- Chapter 12, page 368
// -------------------------------------------------------------------------------------------
class InternalScriptExports
{
	static ScriptEventListenerMgr* s_pScriptEventListenerMgr;

public:
	// initialization
	static bool Init(void);
	static void Destroy(void);

	// These are exported to Lua (resource loading)
	static bool LoadAndExecuteScriptResource(const char* scriptResource);

	// event system
	static unsigned long RegisterEventListener(EventType eventType, LuaPlus::LuaObject callbackFunction);
	static void RemoveEventListener(unsigned long listenerId);
	static bool QueueEvent(EventType eventType, LuaPlus::LuaObject eventData);
	static bool TriggerEvent(EventType eventType, LuaPlus::LuaObject eventData);



	// process system
	static void AttachScriptProcess(LuaPlus::LuaObject scriptProcess);

private:
	static std::shared_ptr<ScriptEvent> BuildEvent(EventType eventType, LuaPlus::LuaObject& eventData);
};

ScriptEventListenerMgr* InternalScriptExports::s_pScriptEventListenerMgr = nullptr;

// --------------------------------------------------------------------------------------------------------
// Destructor
// --------------------------------------------------------------------------------------------------------
ScriptEventListenerMgr::~ScriptEventListenerMgr(void)
{
	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		ScriptEventListener* pListener = (*it);
		delete pListener;
	}
	m_listeners.clear();
}

// --------------------------------------------------------------------------------------------------------
// Adds a new listener
// --------------------------------------------------------------------------------------------------------
void ScriptEventListenerMgr::AddListener(ScriptEventListener* pListener)
{
	m_listeners.insert(pListener);
}

// --------------------------------------------------------------------------------------------------------
// Destroys a listener
// --------------------------------------------------------------------------------------------------------
void ScriptEventListenerMgr::DestroyListener(ScriptEventListener* pListener)
{
	ScriptEventListenerSet::iterator findIt = m_listeners.find(pListener);
	if (findIt != m_listeners.end())
	{
		m_listeners.erase(findIt);
		delete pListener;
	}
	else
	{
		//Nv_ERROR("Couldn't find script listener in set; this will probably cause a memory leak");
	}
}


// --------------------------------------------------------------------------------------------------------
// Event Listener
// --------------------------------------------------------------------------------------------------------
ScriptEventListener::ScriptEventListener(const EventType& eventType, const LuaPlus::LuaObject& scriptCallbackFunction)
	: m_scriptCallbackFunction(scriptCallbackFunction)
{
	m_eventType = eventType;
}

ScriptEventListener::~ScriptEventListener(void)
{
	IEventManager* pEventMgr = IEventManager::Get();
	if (pEventMgr)
		pEventMgr->VRemoveListener(GetDelegate(), m_eventType);
}

void ScriptEventListener::ScriptEventDelegate(IEventDataPtr pEvent)
{
	//Nv_ASSERT(m_scriptCallbackFunction.IsFunction());	// this should never happen since it's validated before even creating the object.

	// call the Lua function
	std::shared_ptr<ScriptEvent> pScriptEvent = static_pointer_cast<ScriptEvent>(pEvent);
	LuaPlus::LuaFunction<void> Callback = m_scriptCallbackFunction;
	Callback(pScriptEvent->GetEventData());
}

// --------------------------------------------------------------------------------------------------------
// Initializes the script export system
// --------------------------------------------------------------------------------------------------------
bool InternalScriptExports::Init(void)
{
	//Nv_ASSERT(s_pScriptEventListenerMgr == nullptr);
	s_pScriptEventListenerMgr = Nv_NEW ScriptEventListenerMgr;

	return true;
}

// --------------------------------------------------------------------------------------------------------
// Destroys the script export system.
// --------------------------------------------------------------------------------------------------------
void InternalScriptExports::Destroy(void)
{
	//Nv_ASSERT(s_pScriptEventListenerMgr != nullptr);
	SAFE_DELETE(s_pScriptEventListenerMgr);
}

// --------------------------------------------------------------------------------------------------------
// Loads a script resource then executes it. This is used by the require() function in script (defined in
// PreInit.lua).
// --------------------------------------------------------------------------------------------------------
bool InternalScriptExports::LoadAndExecuteScriptResource(const char* scriptResource)
{
	if (!g_pApp->m_ResCache->IsUsingDevelopmentDirectories())
	{
		Resource resource(scriptResource);
		std::shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);	// this actually loads the Lua file from the zip file
		if (pResourceHandle) {
			return true;
		}
		return false;
	}
	else
	{
		// If we're using development directories, have Lua execute the file directly instead of going through
		// the resource cache. This allows Decoda to see the file for debugging purposes.
		std::string path("..\\Scripts\\");
		path += scriptResource;
		LuaStateManager::Get()->VExecuteFile(path.c_str());
		return true;
	}
}

// ----------------------------------------------------------------------------------------------------------
// Instantiates a C++ ScriptListener object, inserts it into the manager, and returns a handle to it. The
// script should maintain the handle if it needs to remove the listener at some point. Otherwise, the
// listener will be destroyed when the program exits.
// ----------------------------------------------------------------------------------------------------------
unsigned long InternalScriptExports::RegisterEventListener(EventType eventType, LuaPlus::LuaObject callbackFunction)
{
	//Nv_ASSERT(s_pScriptEventListenerMgr);

	if (callbackFunction.IsFunction())
	{
		// create the C++ listener proxy and set it to listen for the event
		ScriptEventListener* pListener = Nv_NEW ScriptEventListener(eventType, callbackFunction);
		s_pScriptEventListenerMgr->AddListener(pListener);
		IEventManager::Get()->VAddListener(pListener->GetDelegate(), eventType);

		// convert the pointer to an unsigned long to use as the handle
		unsigned long handle = reinterpret_cast<unsigned long>(pListener);
		return handle;
	}

	//Nv_ERROR("Attempting to register script event listener with invalid callback function");
	return 0;
}

// ----------------------------------------------------------------------------------------------------------
// Removes a script listener.
// ----------------------------------------------------------------------------------------------------------
void InternalScriptExports::RemoveEventListener(unsigned long listenerId)
{
	//Nv_ASSERT(s_pScriptEventListenerMgr);
	//Nv_ASSERT(listenerId != 0);

	// convert the listenerId back into a pointer
	ScriptEventListener* pListener = reinterpret_cast<ScriptEventListener*>(listenerId);
	s_pScriptEventListenerMgr->DestroyListener(pListener);  // the destructor will remove the listener
}


void InternalScriptExports::AttachScriptProcess(LuaPlus::LuaObject scriptProcess)
{
	LuaPlus::LuaObject temp = scriptProcess.Lookup("__object");
	if (!temp.IsNil())
	{
		std::shared_ptr<Process> pProcess(static_cast<Process*>(temp.GetLightUserData()));
		g_pApp->m_pGame->AttachProcess(pProcess);
	}
	else
	{
		//Nv_ERROR("Couldn't find __object in script process");
	}
}

// ----------------------------------------------------------------------------------------------------------
// Queue's an event from the script. Returns true if the event was sent, false if not.
// ----------------------------------------------------------------------------------------------------------
bool InternalScriptExports::QueueEvent(EventType eventType, LuaPlus::LuaObject eventData)
{
	std::shared_ptr<ScriptEvent> pEvent(BuildEvent(eventType, eventData));
	if (pEvent)
	{
		IEventManager::Get()->VQueueEvent(pEvent);
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------------------------------------------
// Sends an event from the script. Returns true if the event was sent, false if not.
// ----------------------------------------------------------------------------------------------------------
bool InternalScriptExports::TriggerEvent(EventType eventType, LuaPlus::LuaObject eventData)
{
	std::shared_ptr<ScriptEvent> pEvent(BuildEvent(eventType, eventData));
	if (pEvent) {
		return IEventManager::Get()->VTriggerEvent(pEvent);
	}
	return false;
}

// ----------------------------------------------------------------------------------------------------------
// Builds the event to be sent or queued
// ----------------------------------------------------------------------------------------------------------
std::shared_ptr<ScriptEvent> InternalScriptExports::BuildEvent(EventType eventType, LuaPlus::LuaObject& eventData)
{
	// create the event from the event type
	std::shared_ptr<ScriptEvent> pEvent(ScriptEvent::CreateEventFromScript(eventType));
	if (!pEvent) {
		return std::shared_ptr<ScriptEvent>();
	}

	// set the event data that was passed in
	if (!pEvent->SetEventData(eventData))
	{
		return std::shared_ptr<ScriptEvent>();
	}

	return pEvent;
}



void ScriptExports::Register(void)
{
	LuaPlus::LuaObject globals = LuaStateManager::Get()->GetGlobalVars();

	// init
	InternalScriptExports::Init();

	// resource loading
	globals.RegisterDirect("LoadAndExecuteScriptResource", InternalScriptExports::LoadAndExecuteScriptResource);
}

void ScriptExports::Unregister(void)
{
	InternalScriptExports::Destroy();
}