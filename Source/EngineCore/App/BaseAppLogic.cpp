
// ================================================================
// App.h : implements game logic class.
// ================================================================

#include "Common/CommonStd.h"

#include <mmsystem.h>

//#include "../AI/Pathing.h"
#include "../EventManager/Events.h"							// only for EvtData_Game_State
#include "../Initialization/Initialization.h"				// only for GameOptions
#include "../MainLoop/Process.h"
//#include "../Network/Network.h"
#include "../ResourceCache/XmlResource.h"
#include "../Physics/Physics.h"
#include "../Actors/Actor.h"
#include "../Actors/ActorFactory.h"
#include "../Utilities/String.h"
#include "../UserInterface/HumanView.h"						// [rez] not ideal, but the loading sequence needs to know if this is a human view.

#include "BaseAppLogic.h"

// ===============================================================
//
// LevelManager implementation
//
// ===============================================================



// ===============================================================
//
// BaseGameLogic implementation
//
// ===============================================================

BaseAppLogic::BaseAppLogic()
{
	m_LastActorId = 0;
	m_Lifetime = 0;
	m_pProcessManager = Nv_NEW ProcessManager;
	m_random.Randomize();
	m_State = BGS_Initializing;
	m_bProxy = false;
	m_RenderDiagnostics = false;
	m_ExpectedPlayers = 0;
	m_ExpectedRemotePlayers = 0;
	m_ExpectedAI = 0;
	m_HumanPlayersAttached = 0;
	m_AIPlayersAttached = 0;
	m_HumanGamesLoaded = 0;
	m_pPathingGraph = NULL;
	m_pActorFactory = NULL;

	m_pLevelManager = Nv_NEW LevelManager;
	//Nv_ASSERT(m_pProcessManager && m_pLevelManager);
	//m_pLevelManager->Initialize(g_pApp->m_ResCache->Match("world\\*.xml"));

	// register scrip events from the engine
	// [mrmike] this was moved to the constructor post-press, since this function can be called when new levels are loaded by the game or editor
	RegisterEngineScriptEvents();
}

BaseAppLogic::~BaseAppLogic()
{
	// [mrmike]: 12 Apr-2009
	// Added this to explicitly remove views from the game logic list.
	while (!m_gameViews.empty()) {
		m_gameViews.pop_front();
	}

	SAFE_DELETE(m_pLevelManager);
	SAFE_DELETE(m_pProcessManager);
	SAFE_DELETE(m_pActorFactory);

	// destroy all actors
	for (auto it = m_actors.begin(); it != m_actors.end(); ++it)
	{
		it->second->Destroy();
	}
	m_actors.clear();

	IEventManager::Get()->VRemoveListener(fastdelegate::MakeDelegate(this, &BaseAppLogic::RequestDestroyActorDelegate), EvtData_Request_Destroy_Actor::sk_EventType);
}

bool BaseAppLogic::Init(void)
{
	m_pActorFactory = VCreateActorFactory();
	//m_pPathingGraph.reset(CreatePathingGraph());

	IEventManager::Get()->VAddListener(fastdelegate::MakeDelegate(this, &BaseAppLogic::RequestDestroyActorDelegate), EvtData_Request_Destroy_Actor::sk_EventType);

	return true;
}

std::string BaseAppLogic::GetActorXml(const ActorId id)
{
	StrongActorPtr pActor = MakeStrongPtr(VGetActor(id));
	if (pActor) {
		return pActor->ToXML();
	}
	else {
		//Nv_ERROR("Couldn't find actor: " + ToStr(id));
	}

	return std::string();
}

bool BaseAppLogic::VLoadGame(const char* levelResource)
{

}

void BaseAppLogic::VSetProxy()
{

}

StrongActorPtr BaseAppLogic::VCreateActor(const std::string& actorResource, TiXmlElement* overrides, const Mat4x4* initialTransform, const ActorId serversActorId)
{
	Nv_ASSERT(m_pActorFactory);
	if (!m_bProxy && serversActorId != INVALID_ACTOR_ID) {
		return StrongActorPtr();
	}

	if (m_bProxy && serversActorId == INVALID_ACTOR_ID) {
		return StrongActorPtr();
	}

	StrongActorPtr pActor = m_pActorFactory->CreateActor(actorResource.c_str(), overrides, initialTransform, serversActorId);
	if (pActor) {
		m_actors.insert(std::make_pair(pActor->GetId(), pActor));
		if (!m_bProxy && (m_State == BGS_SpawningPlayersActors || m_State == BGS_Running))
		{
			std::shared_ptr<EvtData_Request_New_Actor> pNewActor(Nv_NEW EvtData_Request_New_Actor(actorResource, initialTransform, pActor->GetId()));
			IEventManager::Get()->VTriggerEvent(pNewActor);
		}
		return pActor;
	}
	else
	{
		// FUTURE WORK: Log Error: couldn't create actor
		return StrongActorPtr();
	}

}

void BaseAppLogic::VRenderDiagnostics()
{
	if (m_RenderDiagnostics)
	{
		m_pPhysics->VRenderDiagnostics();
	}
}
