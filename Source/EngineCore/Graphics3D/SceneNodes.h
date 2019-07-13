#pragma once

// ================================================================================
// SceneNodes.h : Defines the base class for the 3D graphics scene graph, and derived
//					classes like RootNode, Alpha scene nodes, Camera, Grids, etc.
//
// ================================================================================

#include "../Common/CommonStd.h"
//#include "../Actors/RenderComponent"

#include "Geometry.h"
#include "Material.h"
#include "Shaders.h"

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
//
// AlphaType						-Chapter X, page Y
//
//	This enum defines the different types of alpha blending
//  types that can be set on a scene node.
//
// ----------------------------------------------------------

enum AlphaType
{
	AlphaOpaque,
	AlphaTexture,
	AlphaMaterial,
	AlphaVertex
};


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
	ActorId					m_ActorId;
	std::string				m_Name;
	Mat4x4					m_ToWorld, m_FromWorld;
	float					m_Radius;
	RenderPass				m_RenderPass;
	Material				m_Material;
	AlphaType				m_AlphaType;

	void SetAlpha(const float alpha)
	{
		m_AlphaType = AlphaMaterial;
		m_Material.SetAlpha(alpha);
	}

public:
	SceneNodeProperties(void);
	const ActorId& ActorId() const { return m_ActorId; }
	Mat4x4 const& ToWorld() const { return m_ToWorld; }
	Mat4x4 const& FromWorld() const { return m_FromWorld; }
	void Transform(Mat4x4* toWorld, Mat4x4* fromWorld) const;

	const char* Name() const { return m_Name.c_str(); }

	bool HasAlpha() const { return m_Material.HasAlpha(); }
	float Alpha() const { return m_Material.GetAlpha(); }
	AlphaType AlphaType() const { return m_AlphaType; }

	RenderPass RenderPass() const { return m_RenderPass; }
	float Radius() const { return m_Radius; }

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

	virtual HRESULT VOnRestore(Scene* pScene);
	virtual HRESULT VOnUpdate(Scene *, DWORD const elapsedMs);

	virtual HRESULT VPreRender(Scene* pScene);
	virtual bool VIsVisible(Scene* pScene) const;
	virtual HRESULT VRender(Scene* pScene) { return S_OK; }
	virtual HRESULT VRenderChildren(Scene* pScene);
	virtual HRESULT VPostRender(Scene* pScene);

	virtual bool VAddChild(std::shared_ptr<ISceneNode> kid);
	virtual bool VRemoveChild(ActorId id);

	virtual HRESULT VOnLostDevice(Scene* pScene);
	virtual HRESULT VPick(Scene* pScene, RayCast* pRayCast);

	void SetAlpha(float alpha);
	float GetAlpha() const { return m_Props.Alpha(); }

	Vec3 GetPosition() const { return m_Props.m_ToWorld.GetPosition(); }
	void SetPosition(const Vec3& pos) { m_Props.m_ToWorld.SetPosition(pos); }

	const Vec3 GetWorldPosition() const;	// [mrmike] added post-press to respect ancestor's position

	Vec3 GetDirection() const { return m_Props.m_ToWorld.GetDirection(); }

	void SetRadius(const float radius) { m_Props.m_Radius = radius; }
	void SetMaterial(const Material& mat) { m_Props.m_Material = mat; }
};


class D3DSceneNode11 : public SceneNode
{
public:
	virtual HRESULT VRender(Scene* pScene) { return S_OK; }
};

// =====================================================================
//
// AlphaSceneNode Description					- Chapter 16, page 535
// AlphaSceneNodes Description					- Chapter 16, page 535
//
// A list of scene nodes that need to be drawn in the alpha pass;
// the list is defined as an STL list.
//
// =====================================================================
struct AlphaSceneNode
{
	std::shared_ptr<ISceneNode> m_pNode;
	Mat4x4 m_Concat;
	float m_ScreenZ;

	// For the STL sort...
	bool const operator < (AlphaSceneNode const& other) { return m_ScreenZ < other.m_ScreenZ; }
};

typedef std::list<AlphaSceneNode*> AlphaSceneNodes;

// ========================================================
//
// SceneActorMap Description
//
//		An STL map that allows fast lookup of a scene node given an ActorId.
//
// ========================================================

typedef std::map<ActorId, std::shared_ptr<ISceneNode>> SceneActorMap;

// ========================================================
//
// Scene Description
//
// A hierarchical container of scene nodes, which
// are classes that implement the ISceneNode interface
//
// ========================================================

class CameraNode;
class SkyNode;


// ========================================================
//
// class RootNode								- Chapter 16, page 545
//
// This is the root node of the scene graph.
//
// ========================================================

class RootNode : public SceneNode
{
public:
	RootNode();
	virtual bool VAddChild(std::shared_ptr<ISceneNode> kid);
	virtual HRESULT VRenderChildren(Scene* pScene);
	virtual bool VRemoveChild(ActorId id);
	virtual bool VIsVisible(Scene* pScene) const { return true; }
};

// ---------------------------------------------
// 
// class CameraNode								- Chapter 16, page 548
// 
// A camra node controls the D3D view transform and holds the view
// frustum definition.
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