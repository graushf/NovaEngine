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

	// resource loading
	static bool LoadAndExecuteScriptResource(const char* scriptResource);

	// actors
	static int CreateActor(const char* actorArchetype, LuaPlus::LuaObject luaPosition, LuaPlus::LuaObject luaYawPitchRoll);

	// event system
	static unsigned long RegisterEventListener(EventType eventType, LuaPlus::LuaObject callbackFunction);
	static void RemoveEventListener(unsigned long listenerId);
	static bool QueueEvent(EventType eventType, LuaPlus::LuaObject eventData);
	static bool TriggerEvent(EventType eventType, LuaPlus::LuaObject eventData);

	// process system
	static void AttachScriptProcess(LuaPlus::LuaObject scriptProcess);

	// math
	static float GetYRotationFromVector(LuaPlus::LuaObject vec3);
	static float WrapPi(float wrapMe);
	static LuaPlus::LuaObject GetVectorFromRotation(float angleRadians);

	// misc.
	static void LuaLog(LuaPlus::LuaObject text);
	static unsigned long GetTickCount(void);

	// physics
	static void ApplyForce(LuaPlus::LuaObject normalDir, float force, int actorId);
	static void ApplyTorque(LuaPlus::LuaObject axis, float force, int actorId);

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

int InternalScriptExports::CreateActor(const char* actorArchetype, LuaPlus::LuaObject luaPosition, LuaPlus::LuaObject luaYawPitchRoll)
{
	if (!luaPosition.IsTable())
	{
		//Nv_ERROR("Invalid object passed to CreateActor(); type = " + std::string(luaPosition.TypeName()));
		return INVALID_ACTOR_ID;
	}

	if (!luaYawPitchRoll.IsTable())
	{
		//Nv_ERROR("Invalid object passed to CreateActor(); type = " + std::string(luaYawPitchRoll.TypeName()));
		return INVALID_ACTOR_ID;
	}

	Vec3 pos(luaPosition["x"].GetFloat(), luaPosition["y"].GetFloat(), luaPosition["z"].GetFloat());
	Vec3 ypr(luaYawPitchRoll["x"].GetFloat(), luaYawPitchRoll["y"].GetFloat(), luaYawPitchRoll["z"].GetFloat());

	Mat4x4 initialTransform;
	initialTransform.BuildYawPitchRoll(ypr.x, ypr.y, ypr.z);
	initialTransform.SetPosition(pos);

	TiXmlElement* overloads = NULL;
	StrongActorPtr pActor = g_pApp->m_pGame->VCreateActor(actorArchetype, overloads, &initialTransform);

	if (pActor)
	{
		std::shared_ptr<EvtData_New_Actor> pNewActorEvent(Nv_NEW EvtData_New_Actor(pActor->GetId()));
		IEventManager::Get()->VQueueEvent(pNewActorEvent);
		return pActor->GetId();
	}

	return INVALID_ACTOR_ID;
}

float InternalScriptExports::WrapPi(float wrapMe)
{
	return ::WrapPi(wrapMe);
}

float InternalScriptExports::GetYRotationFromVector(LuaPlus::LuaObject vec3)
{
	if (vec3.IsTable())
	{
		Vec3 lookAt(vec3["x"].GetFloat(), vec3["y"].GetFloat(), vec3["z"].GetFloat());
		return ::GetYRotationFromVector(lookAt);
	}

	//Nv_ERROR("Invalid object passed to GetYRotationFromVector(); type = " + std::string(vec3.TypeName()));
	return 0;
}


LuaPlus::LuaObject InternalScriptExports::GetVectorFromRotation(float angleRadians)
{
	Vec3 result = ::GetVectorFromYRotation(angleRadians);
	LuaPlus::LuaObject luaResult;

	luaResult.AssignNewTable(LuaStateManager::Get()->GetLuaState());
	luaResult.SetNumber("x", result.x);
	luaResult.SetNumber("y", result.y);
	luaResult.SetNumber("z", result.z);

	return luaResult;
}

void InternalScriptExports::LuaLog(LuaPlus::LuaObject text)
{
	if (text.IsConvertibleToString()) 
	{
		//Nv_LOG("Lua", text.ToString());
	}
	else
	{
		//Nv_LOG("Lua", "<" + std::string(text.TypeName()) + ">");
	}
}

unsigned long InternalScriptExports::GetTickCount(void)
{
	return ::GetTickCount();
}

// ----------------------------------------------------------------------------------------------------------
// Script exports for the physics system
// ----------------------------------------------------------------------------------------------------------
void InternalScriptExports::ApplyForce(LuaPlus::LuaObject normalDirLua, float force, int actorId)
{
	if (normalDirLua.IsTable()) {
		Vec3 normalDir(normalDirLua["x"].GetFloat(), normalDirLua["y"].GetFloat(), normalDirLua["z"].GetFloat());
		g_pApp->m_pGame->VGetGamePhysics()->VApplyForce(normalDir, force, actorId);
		return;
	}
	//Nv_ERROR("Invalid object passed to ApplyForce(); type = " + std::string(normalDirLua.TypeName()));
}

void InternalScriptExports::ApplyTorque(LuaPlus::LuaObject axisLua, float force, int actorId)
{
	if (axisLua.IsTable())
	{
		Vec3 axis(axisLua["x"].GetFloat(), axisLua["y"].GetFloat(), axisLua["z"].GetFloat());
		g_pApp->m_pGame->VGetGamePhysics()->VApplyTorque(axis, force, actorId);
		return;
	}
	//Nv_ERROR("Invalid object passed to ApplyTorque(); type = " + std::string(axisLua.TypeName()));
}

void ScriptExports::Register(void)
{
	LuaPlus::LuaObject globals = LuaStateManager::Get()->GetGlobalVars();

	// init
	InternalScriptExports::Init();

	// resource loading
	globals.RegisterDirect("LoadAndExecuteScriptResource", InternalScriptExports::LoadAndExecuteScriptResource);

	// actors
	globals.RegisterDirect("CreateActor", &InternalScriptExports::CreateActor);

	// event system
	globals.RegisterDirect("RegisterEventListener", &InternalScriptExports::RegisterEventListener);
	globals.RegisterDirect("RemoveEventListener", &InternalScriptExports::RemoveEventListener);
	globals.RegisterDirect("QueueEvent", &InternalScriptExports::QueueEvent);
	globals.RegisterDirect("TriggerEvent", &InternalScriptExports::TriggerEvent);

	// process system
	globals.RegisterDirect("AttachProcess", &InternalScriptExports::AttachScriptProcess);

	// math
	LuaPlus::LuaObject mathTable = globals.GetByName("NvMath");
	//Nv_ASSERT(mathTable.IsTable());
	mathTable.RegisterDirect("GetYRotationFromVector", &InternalScriptExports::GetYRotationFromVector);
	mathTable.RegisterDirect("WrapPi", &InternalScriptExports::WrapPi);
	mathTable.RegisterDirect("GetVectorFromRotation", &InternalScriptExports::GetVectorFromRotation);

	// misc.
	globals.RegisterDirect("Log", &InternalScriptExports::LuaLog);
	globals.RegisterDirect("GetTickCount", &InternalScriptExports::GetTickCount);

	// Physics
	globals.RegisterDirect("ApplyForce", &InternalScriptExports::ApplyForce);
	globals.RegisterDirect("ApplyTorque", &InternalScriptExports::ApplyTorque);
}


// ------------------------------------------------------------------------------------------------------------------------
// This function unregisters all the ScriptExports functions and gives any underlaying systems a chance to destroy
// themselves. It is called in the Application destructor.
// ------------------------------------------------------------------------------------------------------------------------
void ScriptExports::Unregister(void)
{
	InternalScriptExports::Destroy();
}
