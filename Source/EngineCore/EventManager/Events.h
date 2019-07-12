#pragma once

//========================================================================
// EventManager.h : Defines common game events
//========================================================================

#include "EventManager.h"
#include "Common/CommonStd.h"
#include "App/App.h"


// ------------------------------------------------------------------------------------
// EvtData_DestroyActor - sent when actors are destroyed
// ------------------------------------------------------------------------------------
class EvtData_Destroy_Actor : public BaseEventData
{
	ActorId m_id;

public:
	static const EventType sk_EventType;

	explicit EvtData_Destroy_Actor(ActorId id = INVALID_ACTOR_ID)
		: m_id(id)
	{
		//
	}

	virtual const EventType& VGetEventType(void) const
	{
		return sk_EventType;
	}

	virtual IEventDataPtr VCopy(void) const
	{
		return IEventDataPtr(Nv_NEW EvtData_Destroy_Actor(m_id));
	}

	virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_id;
	}

	virtual void VDeserialize(std::istrstream& in)
	{
		in >> m_id;
	}

	virtual const char* GetName(void) const
	{
		return "EvtData_Destroy_Actor";
	}

	ActorId GetId(void) const { return m_id; }
};


// --------------------------------------------------------------------------------------------------
// EvtData_Move_Actor - sent when actors are moved.
// --------------------------------------------------------------------------------------------------
class EvtData_Move_Actor : public BaseEventData
{
	ActorId m_id;
	Mat4x4 m_matrix;

public:
	static const EventType sk_EventType;

	virtual const EventType& VGetEventType(void) const
	{
		return sk_EventType;
	}

	EvtData_Move_Actor(void)
	{
		m_id = INVALID_ACTOR_ID;
	}

	EvtData_Move_Actor(ActorId id, const Mat4x4& matrix)
		: m_id(id), m_matrix(matrix)
	{
		//
	}

	virtual void VSerialize(std::ostrstream& out) const
	{
		out << m_id << " ";
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				out << m_matrix.m[i][j] << " ";
			}
		}
	}

	virtual void VDeserialize(std::istrstream& in)
	{
		in >> m_id;
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				in >> m_matrix.m[i][j];
			}
		}
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr(Nv_NEW EvtData_Move_Actor(m_id, m_matrix));
	}

	virtual const char* GetName(void) const
	{
		return "EvtData_Move_Actor";
	}

	ActorId GetId(void) const
	{
		return m_id;
	}

	const Mat4x4& GetMatrix(void) const
	{
		return m_matrix;
	}
};





// --------------------------------------------------------------------------------------------------
// EvtData_New_Render_Component - This event is sent out when an actor is *actually* created.
// --------------------------------------------------------------------------------------------------
class EvtData_New_Render_Component : public BaseEventData
{
	ActorId m_actorId;
	std::shared_ptr<SceneNode> m_pSceneNode;

public:
	static const EventType sk_EventType;

	EvtData_New_Render_Component(void)
	{
		m_actorId = INVALID_ACTOR_ID;
	}

	explicit EvtData_New_Render_Component(ActorId actorId, std::shared_ptr<SceneNode> pSceneNode)
		: m_actorId(actorId),
		m_pSceneNode(pSceneNode)
	{
	}

	virtual void VSerialize(std::ostrstream& out) const
	{
		//Nv_ERROR(GetName() + std::string(" should not be serialized!"));
	}

	virtual void VDeserialize(std::istrstream& in)
	{
		//Nv_ERROR(GetName() + std::string(" should not be serialized!"));
	}

	virtual const EventType& VGetEventType(void) const
	{
		return sk_EventType;
	}

	virtual IEventDataPtr VCopy(void) const
	{
		return IEventDataPtr(Nv_NEW EvtData_New_Render_Component(m_actorId, m_pSceneNode));
	}

	virtual const char* GetName(void) const
	{
		return "EvtData_New_Render_Component";
	}

	const ActorId GetActorId(void) const
	{
		return m_actorId;
	}

	std::shared_ptr<SceneNode> GetSceneNode(void) const
	{
		return m_pSceneNode;
	}
};

// --------------------------------------------------------------------------------------------------
// EvtData_Modified_Render_Component - This event is sent out when a render component is changed
//		NOTE: This class is not described in the book!
// --------------------------------------------------------------------------------------------------
class EvtData_Modified_Render_Component : public BaseEventData
{
	ActorId m_id;

public:
	static const EventType sk_EventType;

	virtual const EventType& VGetEventType(void) const
	{
		return sk_EventType;
	}

	EvtData_Modified_Render_Component(void)
	{
		m_id = INVALID_ACTOR_ID;
	}

	EvtData_Modified_Render_Component(ActorId id)
		: m_id(id)
	{
	}

	virtual void VSerialize(std::ostrstream &out) const
	{
		out << m_id;
	}

	virtual void VDeserialize(std::istrstream& in)
	{
		in >> m_id;
	}

	virtual IEventDataPtr VCopy() const
	{
		return IEventDataPtr(Nv_NEW EvtData_Modified_Render_Component(m_id));
	}

	virtual const char* GetName(void) const
	{
		return "EvtData_Modified_Render_Component";
	}

	ActorId GetActorId(void) const
	{
		return m_id;
	}
};