#pragma once

//#include "Common/CommonStd.h"
#include "MainLoop/ProcessManager.h"
#include "Utilities/Math.h"
#include "Actors/Actor.h"

class PathingGraph;
class ActorFactory;
class LevelManager;

enum BaseGameState
{
	BGS_Invalid,
	BGS_Initializing,
	BGS_MainMenu,
	BGS_WaitingForPlayers,
	BGS_LoadingGameEnvironment,
	BGS_WaitingForPlayersToLoadEnvironment,
	BGS_SpawningPlayersActors,
	BGS_Running
};

typedef std::map<ActorId, StrongActorPtr> ActorMap;
typedef std::string Level;

class LevelManager
{

};

class BaseAppLogic : public IGameLogic
{
	
	friend class App;								// This is only to gain access to the view list
	
protected:
	float m_Lifetime;
	ProcessManager* m_pProcessManager;
	NvRandom m_random;
	ActorMap m_actors;
	ActorId m_LastActorId;
	BaseGameState m_State;
	int m_ExpectedPlayers;
	int m_ExpectedRemotePlayers;
	int m_ExpectedAI;
	int m_HumanPlayersAttached;
	int m_AIPlayerAttached;
	int m_HumanGamesLoaded;
	GameViewList m_gameViews;
	std::shared_ptr<PathingGraph> m_pPathingGraph;
	ActorFactory* m_pActorFactory;

	bool m_bProxy;
	int m_remotePlayerId;

	bool m_RenderDiagnostics;
	std::shared_ptr<IGamePhysics> m_pPhysics;

	LevelManager* m_pLevelManager;
	
public:

	BaseAppLogic();
	virtual ~BaseAppLogic();
	bool Init(void);
	
	/*
	void SetProxy(bool isProxy)
	{
		m_bProxy = isProxy;
	}
	
	const bool IsProxy() const { return m_bProxy; } */
	
	// rest of code


	// Render Diagnostics
	virtual void VRenderDiagnostics();
};