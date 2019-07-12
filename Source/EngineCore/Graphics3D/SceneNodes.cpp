#include "../Common/CommonStd.h"
#include "SceneNodes.h"

SceneNodeProperties::SceneNodeProperties(void)
{
	m_ActorId = INVALID_ACTOR_ID;
	m_Radius = 0;
	m_RenderPass = RenderPass_0;
	m_AlphaType = AlphaOpaque;
}

void SceneNodeProperties::Transform(Mat4x4* toWorld, Mat4x4* fromWorld) const
{
	if (toWorld) {
		*toWorld = m_ToWorld;
	}

	if (fromWorld) {
		*fromWorld = m_FromWorld;
	}
}

// ------------------------------------------
//	SceneNode Implementation
// ------------------------------------------

SceneNode::SceneNode(ActorId actorId, WeakBaseRenderComponentPtr renderComponent, RenderPass renderPass, const Mat4x4* to, const Mat4x4* from)
{
	m_pParent = NULL;
	m_Props.m_ActorId = actorId;
	m_Props.m_Name = (renderComponent) ? renderComponent->VGetName() : "SceneNode";
	m_Props.m_RenderPass = renderPass;
	m_Props.m_AlphaType = AlphaOpaque;
	m_RenderComponent = renderComponent;
	VSetTransform(to, from);
	SetRadius(0);
}

SceneNode::~SceneNode()
{
}

HRESULT SceneNode::VOnRestore(Scene* pScene)
{
	Color color = (m_RenderComponent) ? m_RenderComponent->GetColor() : g_White;
	m_Props.m_Material.SetDiffuse(color);

	// This is meant to be called from any class that
	// inherits from SceneNode and overloads
	// VOnRestore()

	SceneNodeList::iterator i = m_Children.begin();
	SceneNodeList::iterator end = m_Children.end();

	while (i != end)
	{
		(*i)->VOnRestore(pScene);
		++i;
	}
	return S_OK;
}

HRESULT SceneNode::VOnLostDevice(Scene* pScene)
{
	// This is meant to be called from any class
	// that inherits from SceneNode and overloads 
	// VOnRestore()

	SceneNodeList::iterator i = m_Children.begin();
	SceneNodeList::iterator end = m_Children.end();

	while (i != end)
	{
		(*i)->VOnLostDevice(pScene);
		++i;
	}
	return S_OK;
}

void SceneNode::VSetTransform(const Mat4x4* toWorld, const Mat4x4* fromWorld)
{
	m_Props.m_ToWorld = *toWorld;
	if (!fromWorld)
	{
		m_Props.m_FromWorld = m_Props.m_ToWorld.Inverse();
	}
	else
	{
		m_Props.m_FromWorld = *fromWorld;
	}
}

HRESULT SceneNode::VPreRender(Scene* pScene)
{
	// This was added post press! It is always ok to read directly from the game logic.
	StrongActorPtr pActor = MakeStrongPtr(g_pApp->GetAppLogic()->VGetActor(m_Props.m_ActorId));
	if (pActor)
	{
		std::shared_ptr<TransformComponent> pTc = MakeStrongPtr(pActor->GetComponent<TransformComponent>(TransformComponent::g_Name));
		if (pTc) 
		{
			m_Props.m_ToWorld = pTc->GetTransform();
		}
	}
	pScene->PushAndSetMatrix(m_Props.m_ToWorld);
	return S_OK;
}

HRESULT SceneNode::VPostRender(Scene* pScene)
{
	pScene->PopMatrix();
	return S_OK;
}

bool SceneNode::VIsVisible(Scene* pScene) const
{
	// transform the location of this node into the camera space
	// of the camera attached to the scene
	Mat4x4 toWorld, fromWorld;
	pScene->GetCamera()->VGet()->Transform(&toWorld, &fromWorld);

	Vec3 pos = GetWorldPosition();

	Vec3 fromWorldPos = fromWorld.Xform(pos);

	Frustum const &frustum = pScene->GetCamera()->GetFrustum();

	bool isVisible = frustum.Inside(fromWorldPos, VGet()->Radius());
	return isVisible;
}

//
// SceneNode::GetWorldPosition				- not described in the book.
//
//		This was added post press to respect any SceneNode ancestors - you have
//		to add all their positions together to get the world position of any 
//		SceneNode
//
const Vec3 SceneNode::GetWorldPosition() const
{
	Vec3 pos = GetPosition();
	if (m_pParent)
	{
		pos += m_pParent->GetWorldPosition();
	}
	return pos;
}

HRESULT SceneNode::VOnUpdate(Scene* pScene, DWORD const elapsedMs)
{
	// This is meant to be called from any class
	// that inherits from SceneNode and overloads
	// VOnUpdate()

	SceneNodeList::iterator i = m_Children.begin();
	SceneNodeList::iterator end = m_Children.end();

	while (i != end)
	{
		(*i)->VOnUpdate(pScene, elapsedMs);
		++i;
	}
	return S_OK;
}


HRESULT SceneNode::VRenderChildren(Scene* pScene)
{
	// Iterate through the children...
	SceneNodeList::iterator i = m_Children.begin();
	SceneNodeList::iterator end = m_Children.end();

	while (i != end)
	{
		if ((*i)->VPreRender(pScene) == S_OK)
		{
			// You could short-circuit rendering
			// if an object returns E_FAIL from
			// VPreRender()

			// Don't render this node if you can't see it.
			if ((*i)->VIsVisible(pScene))
			{
				float alpha = (*i)->VGet()->m_Material.GetAlpha();
				if (alpha == fOPAQUE)
				{
					(*i)->VRender(pScene);
				}
				else if (alpha != fTRANSPARENT)
				{
					// The object isn't totally transparent...
					AlphaSceneNode* asn = Nv_NEW AlphaSceneNode;
					//Nv_ASSERT(asn);
					asn->m_pNode = *i;
					asn->m_Concat = pScene->GetTopMatrix();

					Vec4 worldPos(asn->m_Concat.GetPosition());

					Mat4x4 fromWorld = pScene->GetCamera()->VGet()->FromWorld();

					Vec4 screenPos = fromWorld.Xform(worldPos);

					asn->m_ScreenZ = screenPos.z;

					pScene->AddAlphaSceneNode(asn);
				}

				// [mrmike] see comment just below...
				(*i)->VRenderChildren(pScene);
			}

			// [mrmike] post-press fix - if the parent is not visible, the children
			//					shouldn't be visible either.
			//(*i)->VRenderChildren(pScene);
		}
		(*i)->VPostRender(pScene);
		++i;
	}

	return S_OK;
}

bool SceneNode::VAddChild(std::shared_ptr<ISceneNode> ikid)
{
	m_Children.push_back(ikid);

	std::shared_ptr<SceneNode> kid = static_pointer_cast<SceneNode>(ikid);

	kid->m_pParent = this;			// [mrmike] Post-press fix - the parent was never set!
		
	// The radius of the sphere should be fixed right here
	Vec3 kidPos = kid->VGet()->ToWorld().GetPosition();

	// [mrmike] - Post-press fix. This was not correct! subtracting the parent's position from the kidPos
	//			  created a HUGE radius, depending on the location of the parent, which could be anywhere in
	//			  game world.

	//Vec3 dir = kidPos - m_Props.ToWorld().GetPosition();
	//float newRadius = dir.Length() + kid->VGet()->Radius();

	float newRadius = kidPos.Length() + kid->VGet()->Radius();

	if (newRadius > m_Props.m_Radius)
	{
		m_Props.m_Radius = newRadius;
	}

	return true;
}

//
// SceneNode::VRemoveChild								- not in the book
//
//	If an actor is destroyed it should be removed from the scene graph.
//  Generally the HumanView will receive a message saying the actor has been
//  destroyed, and it will then call Scene::RemoveChild which will traverse
//  the scene graph to find the child that needs removing.
//
bool SceneNode::VRemoveChild(ActorId id)
{
	for (SceneNodeList::iterator i = m_Children.begin(); i != m_Children.end(); ++i)
	{
		const SceneNodeProperties* pProps = (*i)->VGet();
		if (pProps->ActorId() != INVALID_ACTOR_ID && id == pProps->ActorId())
		{
			i = m_Children.erase(i);		// this can be expensive for vectors
			return true;
		}
	}
	return false;
}

HRESULT SceneNode::VPick(Scene* pScene, RayCast* raycast)
{
	for (SceneNodeList::const_iterator i = m_Children.begin(); i != m_Children.end(); ++i)
	{
		HRESULT hr = (*i)->VPick(pScene, raycast);

		if (hr == E_FAIL) {
			return E_FAIL;
		}
	}
	return S_OK;
}

// This was changed post press - it was convenient to be able to set alpha on a parent node and
// have it propagate to all its children, since materials are set in the SceneNode's properties,
// and not as a SceneNode that changes renderstate by itself.
void SceneNode::SetAlpha(float alpha)
{
	m_Props.SetAlpha(alpha);
	for (SceneNodeList::const_iterator i = m_Children.begin(); i != m_Children.end(); ++i) {
		std::shared_ptr<SceneNode> sceneNode = static_pointer_cast<SceneNode>(*i);
		sceneNode->SetAlpha(alpha);
	}
}

Mat4x4 CameraNode::GetWorldViewProjection(Scene* pScene)
{
	Mat4x4 world = pScene->GetTopMatrix();
	Mat4x4 view = VGet()->FromWorld();
	Mat4x4 worldView = world * view;
	
	return worldView * m_Projection;
}

