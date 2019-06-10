#pragma once

#include <memory>

//========================================================================
// ActorFactory.h - Defines interface classes defined throughout the book
//========================================================================

class Actor;
class ActorComponent;

typedef unsigned int ActorId;
typedef unsigned int ComponentId;

const ActorId INVALID_ACTOR_ID = 0;
const ComponentId INVALID_COMPONENT_ID = 0;

typedef std::shared_ptr<Actor> StrongActorPtr;
typedef std::weak_ptr<Actor> WeakActorPtr;
typedef std::shared_ptr<ActorComponent> StrongActorComponentPtr;


//
// class IResourceLoader
//

class IResourceLoader
{
public:

};

// 
// class IScreenElement				- Chapter 10, page 285
//
class IScreenElement
{
public:
	virtual HRESULT VOnRestore() = 0;
	virtual HRESULT VOnLostDevice() = 0;
	virtual HRESULT VOnRender(double fTime, float fElapsedTime) = 0;
	virtual void VOnUpdate(int deltaMilliseconds) = 0;

	virtual int VGetZOrder() const = 0;
	virtual void VSetZOrder(int const zOrder) = 0;
	virtual bool VIsVisible() const = 0;
	virtual void VSetVisible(bool visible) = 0;

	virtual HRESULT CALLBACK VOnMsgProc(AppMsg msg) = 0;

	virtual ~IScreenElement() { };
	virtual bool const operator<(IScreenElement const &other) { return VGetZOrder() < other.VGetZOrder(); }
};

class IGamePhysics;

class IGameLogic
{
public:
	virtual WeakActorPtr VGetActor(const ActorId id) = 0;
	virtual StrongActorPtr VCreateActor(const std::string &actorResource, TiXmlElement* overrides, const Mat4x4* initialTransform = nullptr, const ActorId serversActorId = INVALID_ACTOR_ID) = 0;
	virtual void VDestroyActor(const ActorId actorId) = 0;
	virtual bool VLoadGame(const char* levelResource) = 0;
	virtual void VSetProxy() = 0;
	virtual void VOnUpdate(float time, float elapsedTime) = 0;
	virtual void VChangeState(enum BaseGameState newState) = 0;
	virtual void VMoveActor(const ActorId id, Mat4x4 const &mat) = 0;
	virtual std::shared_ptr<IGamePhysics> VGetGamePhysics(void) = 0;
};

enum GameViewType
{
	GameView_Human,
	GameView_Remote,
	GameView_AI,
	GameView_Recorder,
	GameView_Other
};

typedef unsigned int GameViewId;
extern const GameViewId gc_InvalidGameViewId;

class IGameView
{
public:
	virtual HRESULT VOnRestore() = 0;
	virtual void VOnRender(double fTime, float fElapsedTime) = 0;
	virtual HRESULT VOnLostDevice() = 0;
	virtual GameViewType VGetType() = 0;
	virtual GameViewId VGetId() const = 0;
	virtual void VOnAttach(GameViewId vid, ActorId aid) = 0;

	virtual LRESULT CALLBACK VOnMsgProc(AppMsg msg) = 0;
	virtual void VOnUpdate(unsigned long deltaMs) = 0;

	virtual ~IGameView() { };
};

typedef std::list<std::shared_ptr<IScreenElement> > ScreenElementList;
typedef std::list<std::shared_ptr<IGameView> > GameViewList;


// -----------------------------------------------------------------------------
// class IGamePhysics						- Chapter 17, page 589
// 
// The interface definition for a generic physics API.
// -----------------------------------------------------------------------------

class IGamePhysics
{
public:

	// Debugging
	virtual void VRenderDiagnostics() = 0;

	virtual ~IGamePhysics() { };
};