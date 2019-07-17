
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

}

bool BaseAppLogic::Init(void)
{
	return true;
}

void BaseAppLogic::VRenderDiagnostics()
{
	if (m_RenderDiagnostics)
	{
		m_pPhysics->VRenderDiagnostics();
	}
}
