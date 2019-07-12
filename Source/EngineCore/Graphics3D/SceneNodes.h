#pragma once

// ================================================================================
// SceneNodes.h : Defines the base class for the 3D graphics scene graph, and derived
//					classes like RootNode, Alpha scene nodes, Camera, Grids, etc.
//
// ================================================================================

#include "../Common/CommonStd.h"
#include "Geometry.h"

// Forward declarations
class SceneNode;
class Scene;
class RayCast;
class MovementController;
class IResourceExtraData;
class ActorComponent;
class BaseRenderComponent;

// FUTURE WORK - Smart pointers don't work right... going to use a naked pointer for now
typedef BaseRenderComponent* WeakBaseRenderComponentPtr;

// ----------------------------------------------------------
//  SceneNodeProperties				-Chapter 16, page 527
//
//  This class is contained in the SceneNode class, and gains
//  easy access to common scene node properties such as its
//  ActorId or render pass, with a single ISceneNode::VGet()
//	method.
//
// ----------------------------------------------------------
class SceneNodeProperties
{
	friend class SceneNode;

protected:

	Mat4x4					m_ToWorld, m_FromWorld;
	Material				m_Material;

public:
	SceneNodeProperties(void);

	Mat4x4 const& FromWorld() const { return m_FromWorld; }
	
	void Transform(Mat4x4* toWorld, Mat4x4* fromWorld) const;

	Material GetMaterial() const { return m_Material; }
};


// ----------------------------------------------------------
//  SceneNodeList				-Chapter 16, page 529
//
//  Every scene node has a list of its children - this
//  is implemented as a vector since it is expected that
//  we won't add/delete nodes very often, and we'll want
//  fast random access to each child.
//
// ----------------------------------------------------------
typedef std::vector<std::shared_ptr<ISceneNode>> SceneNodeList;


// ----------------------------------------------------------
//  SceneNode					-Chapter 16, page 529
//
//  Implements ISceneNode. Forms the base class for any node
//  that can exist in the 3D scene graph managed by class 
//  Scene.
//
// ----------------------------------------------------------
class SceneNode : public ISceneNode
{
	friend class Scene;

protected:
	SceneNodeList				m_Children;
	SceneNode					*m_pParent;
	SceneNodeProperties			m_Props;
	WeakBaseRenderComponentPtr	m_RenderComponent;

public:
	SceneNode(ActorId actorId, WeakBaseRenderComponentPtr renderComponent, RenderPass renderPass, const Mat4x4* to, const Mat4x4* from = nullptr);

	virtual ~SceneNode();

	virtual const SceneNodeProperties* const VGet() const { return &m_Props; }

	virtual void VSetTransform(const Mat4x4* toWorld, const Mat4x4* fromWorld = nullptr);
};

// ---------------------------------------------
// 
//
// ---------------------------------------------
class CameraNode : public SceneNode
{
public:
	CameraNode(Mat4x4 const* t, Frustum const& frustum)
		: SceneNode(INVALID_ACTOR_ID, WeakBaseRenderComponentPtr(), RenderPass_0, t),
		m_Frustum(frustum),
		m_bActive(true),
		m_DebugCamera(false),
		m_pTarget(std::shared_ptr<SceneNode>()),
		m_CamOffsetVector(0.0f, 1.0f, -10.0f, 0.0f)
	{
	}

	virtual HRESULT VRender(Scene *pScene);
	virtual HRESULT VOnRestore(Scene *pScene);
	virtual bool VIsVisible(Scene* pScene) const { return m_bActive; }

	const Frustum& GetFrustum() { return m_Frustum; }
	void SetTarget(std::shared_ptr<SceneNode> pTarget)
	{
		m_pTarget = pTarget;
	}
	void ClearTarget() { m_pTarget = std::shared_ptr<SceneNode>(); }
	std::shared_ptr<SceneNode> GetTarget() { return m_pTarget; }

	Mat4x4 GetWorldViewProjection(Scene *pScene);

	Mat4x4 GetProjection() { return m_Projection; }
	Mat4x4 GetView() { return m_View; }
	
	void SetCameraOffset(const Vec4& cameraOffset)
	{
		m_CamOffsetVector = cameraOffset;
	}

protected:

	Frustum							m_Frustum;
	Mat4x4							m_Projection;
	Mat4x4							m_View;
	bool							m_bActive;
	bool							m_DebugCamera;
	std::shared_ptr<SceneNode>		m_pTarget;
	Vec4							m_CamOffsetVector;			// Direction of camera relative to target.
};