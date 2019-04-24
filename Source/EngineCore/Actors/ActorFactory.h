#pragma once

#include "Common/CommonStd.h"

//========================================================================
// ActorFactory.h - Defines a factory for creating actors & components
//========================================================================

class ActorFactory
{
	ActorId m_lastActorId;

protected:
	GenericObjectFactory<ActorComponent, ComponentId> m_componentFactory;

public:
	ActorFactory(void);

	StrongActorPtr CreateActor(const char* actorResource, TiXmlElement* overrides, const Mat4x4* initialTransform, const ActorId serversActorId);
	void ModifyActor(StrongActorPtr pActor, TiXmlElement* overrides);

//protected
	// This function can be overriden by a subclass so you can create game-specific C++ components. If you do
	// this, make sure you call the base-class version first. If it returns NULL, you know it's not an engine component.
	virtual StrongActorComponentPtr VCreateComponent(TiXmlElement* pData);

private:
	ActorId GetNextActorId(void) { ++m_lastActorId; return m_lastActorId; }

};