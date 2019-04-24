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