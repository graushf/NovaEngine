#pragma once

//========================================================================
// ActorComponent.h - Defines the base class for a component that can be attached to an actor.
//========================================================================

#include "Common/CommonStd.h"
#include "../Utilities/String.h"

class TiXmlElement;

class ActorComponent
{
	friend class ActorFactory;

protected:
	StrongActorPtr m_pOwner;

public:
	virtual ~ActorComponent(void) { m_pOwner.reset(); }

	// These functions are meant to be overriden by the implementation classes of the components.
	virtual bool VInit(TiXmlElement* pData) = 0;
	virtual void VPostInit(void) { }
	virtual void VUpdate(int deltaMs) { }
	virtual void VOnChanged(void) { }

	// for the editor
	virtual TiXmlElement* VGenerateXml(void) = 0;

	// This function should be overriden by the interface class.
	virtual ComponentId VGetId(void) const { return GetIdFromName(VGetName()); }
	virtual const char *VGetName() const = 0;
	static ComponentId GetIdFromName(const char* componentStr)
	{
		void* rawId = HashedString::hash_name(componentStr);
		return reinterpret_cast<ComponentId>(rawId);
	}

private:
	void SetOwner(StrongActorPtr pOwner) { m_pOwner = pOwner; }

};