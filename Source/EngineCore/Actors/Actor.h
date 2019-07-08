#pragma once

// =======================================================================
// Actor.h
//
// =======================================================================

#include "../Common/CommonStd.h"

class TiXmlElement;
typedef std::string ActorType;

// -------------------------------------------------------------------------------------------------------------
// Actor class
// -------------------------------------------------------------------------------------------------------------
class Actor
{
	friend class ActorFactory;

public:

	typedef std::map<ComponentId, StrongActorComponentPtr> ActorComponents;

private:
	ActorId m_id;							// unique id for the actor
	ActorComponents m_components;			// all components this actor has
	ActorType m_type;

	// [mrmike] - these were added post press as editor helpers, but will also be great for save game files, if we ever make them.
	std::string m_resource;					// The XML file from which this actor was initialized (considered the "Archetype" file)

public:
	explicit Actor(ActorId id);
	~Actor(void);

	bool Init(TiXmlElement* pData);
	void PostInit(void);
	void Destroy(void);
	void Update(int deltaMs);

	// extra functions
	//bool SaveActorFromEditor(const char* path)
	std::string ToXML();

	// accessors
	ActorId GetId(void) const { return m_id; }
	ActorType GetType(void) const { return m_type; }

	// template function for retrieving components
	template <class ComponentType>
	std::weak_ptr<ComponentType> GetComponent(ComponentId id)
	{
		ActorComponents::iterator findIt = m_components.find(id);
		if (findIt != m_components.end())
		{
			StrongActorComponentPtr pBase(findIt->second);
			std::shared_ptr<ComponentType> pSub(std::static_pointer_cast<ComponentType>(pBase));	// cast to subclass version of the pointer
			std::weak_ptr<ComponentType> pWeakSub(pSub);	// convert strong pointer to weak pointer
			return pWeakSub;	// return the weak pointer
		}
		else
		{
			return std::weak_ptr<ComponentType>();
		}
	}

	template <class ComponentType>
	std::weak_ptr<ComponentType> GetComponent(const char *name)
	{
		ComponentId id = ActorComponent::GetIdFromName(name);
		ActorComponents::iterator findIt = m_components.find(id);
		if (findIt != m_components.end())
		{
			StrongActorComponentPtr pBase(findIt->second);
			std::shared_ptr<ComponentType> pSub(std::static_pointer_cast<ComponentType>(pBase));
			std::weak_ptr<ComponentType> pWeakSub(pSub);
			return pWeakSub;
		}
		else
		{
			return std::weak_ptr<ComponentType>();
		}
	}

	const ActorComponents* GetComponents() { return &m_components; }

	// This is called by the ActorFactory; no one else should be adding components.
	void AddComponent(StrongActorComponentPtr pComponent);
};
