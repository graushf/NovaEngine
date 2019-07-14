#include "../Common/CommonStd.h"

#include "../App/App.h"
#include "../Actors/ActorComponent.h"
#include "../Actors/RenderComponent.h"
#include "../Actors/TransformComponent.h"

#include "D3DRenderer.h"
#include "Geometry.h"
#include "Lights.h"
//#include "Mesh.h"
//#include "Raycast.h"
#include "SceneNodes.h"
#include "Shaders.h"
#include "tchar.h"
#include "../ResourceCache/ResCache.h"

#include <xnamath.h>

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


// ===========================================================
// RootNode Implementation
// ===========================================================

//
// RootNode::RootNode						- Chapter 16, page 545
//
RootNode::RootNode()
	:SceneNode(INVALID_ACTOR_ID, WeakBaseRenderComponentPtr(), RenderPass_0, &Mat4x4::g_Identity)
{
	m_Children.reserve(RenderPass_Last);

	std::shared_ptr<SceneNode> staticGroup(Nv_NEW SceneNode(INVALID_ACTOR_ID, WeakBaseRenderComponentPtr(), RenderPass_Static, &Mat4x4::g_Identity));
	m_Children.push_back(staticGroup); // RenderPass_Static = 0

	std::shared_ptr<SceneNode> actorGroup(Nv_NEW SceneNode(INVALID_ACTOR_ID, WeakBaseRenderComponentPtr(), RenderPass_Actor, &Mat4x4::g_Identity));
	m_Children.push_back(actorGroup);	// RenderPass_Actor = 1

	std::shared_ptr<SceneNode> skyGroup(Nv_NEW SceneNode(INVALID_ACTOR_ID, WeakBaseRenderComponentPtr(), RenderPass_Sky, &Mat4x4::g_Identity));
	m_Children.push_back(skyGroup);	// RenderPass_Sky = 2

	std::shared_ptr<SceneNode> invisibleGroup(Nv_NEW SceneNode(INVALID_ACTOR_ID, WeakBaseRenderComponentPtr(), RenderPass_NotRendered, &Mat4x4::g_Identity));
	m_Children.push_back(invisibleGroup); // RenderPass_NotRendered = 3
}

//
// RootNode::AddChild						- Chapter 16, page 546
//
bool RootNode::VAddChild(std::shared_ptr<ISceneNode> kid)
{
	// The Root node has children that divide the scene graph into render passes.
	// Scene nodes will get added to these children based on the value of the 
	// render pass member variable.

	RenderPass pass = kid->VGet()->RenderPass();
	if ((unsigned)pass >= m_Children.size() || !m_Children[pass])
	{
		//Nv_ASSERT(0 && _T("There is no such render pass"));
		return false;
	}

	return m_Children[pass]->VAddChild(kid);
}

//
// RootNode::VRemoveChild					- not described in the book.
//
//  Returns false if nothing was removed.
//
bool RootNode::VRemoveChild(ActorId id)
{
	bool anythingRemoved = false;
	for (int i = RenderPass_0; i < RenderPass_Last; ++i)
	{
		if (m_Children[i]->VRemoveChild(id))
		{
			anythingRemoved = true;
		}
	}
	return anythingRemoved;
}

// 
// RootNode::VRenderChildren				- Chapter 16, page 547
//
HRESULT RootNode::VRenderChildren(Scene* pScene)
{
	// This code creates fine control of the render passes.

	for (int pass = RenderPass_0; pass < RenderPass_Last; ++pass)
	{
		switch (pass)
		{
			case RenderPass_Static:
			case RenderPass_Actor:
				m_Children[pass]->VRenderChildren(pScene);
				break;
			case RenderPass_Sky:
			{
				std::shared_ptr<IRenderState> skyPass = pScene->GetRenderer()->VPrepareSkyBoxPass();
				m_Children[pass]->VRenderChildren(pScene);
				break;
			}
		}
	}

	return S_OK;
}

// ===========================================================
// CameraNode Implementation
// ===========================================================

//
// CameraNode::VRender						- Chapter 16, page 550
//
HRESULT CameraNode::VRender(Scene* pScene)
{
	if (m_DebugCamera)
	{
		pScene->PopMatrix();

		m_Frustum.Render();

		pScene->PushAndSetMatrix(m_Props.ToWorld());
	}

	return S_OK;
}

//
// CameraNode::VOnRestore					- Chapter 16, page 550
//
HRESULT CameraNode::VOnRestore(Scene* pScene)
{
	m_Frustum.SetAspect(DXUTGetWindowWidth() / (FLOAT)DXUTGetWindowHeight());
	D3DXMatrixPerspectiveFovLH(&m_Projection, m_Frustum.m_Fov, m_Frustum.m_Aspect, m_Frustum.m_Near, m_Frustum.m_Far);
	pScene->GetRenderer()->VSetProjectionTransform(&m_Projection);
	return S_OK;
}

//
// CameraNode::SetViewTransform						- Chapter 16, page 550
//
//	 Note: this is incorrectly called CameraNode::SetView in the book.
//
HRESULT CameraNode::SetViewTransform(Scene* pScene)
{
	// If there is a target, make sure the camera is
	// rigidly attached right behind the target.
	if (m_pTarget)
	{
		Mat4x4 mat = m_pTarget->VGet()->ToWorld();
		Vec4 at = m_CamOffsetVector;
		Vec4 atWorld = mat.Xform(at);
		Vec3 pos = mat.GetPosition() + Vec3(atWorld);
		mat.SetPosition(pos);
		VSetTransform(&mat);
	}

	m_View = VGet()->FromWorld();

	pScene->GetRenderer()->VSetViewTransform(&m_View);
	return S_OK;
}

//
// CameraNode::GetWorldViewProjection				- not described in the book.
//
//	 Returns the concatenation of the world and view projection, which is generally sent into vertex shaders.
//
Mat4x4 CameraNode::GetWorldViewProjection(Scene* pScene)
{
	Mat4x4 world = pScene->GetTopMatrix();
	Mat4x4 view = VGet()->FromWorld();
	Mat4x4 worldView = world * view;

	return worldView * m_Projection;
}




D3DGrid11::D3DGrid11(ActorId actorId, WeakBaseRenderComponentPtr renderComponent, const Mat4x4* pMatrix)
	: SceneNode(actorId, renderComponent, RenderPass_0, pMatrix)
{
	m_bTextureHasAlpha = false;
	m_numVerts = m_numPolys = 0;
	m_pVertexBuffer = NULL;
	m_pIndexBuffer = NULL;
}

D3DGrid11::~D3DGrid11()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
}

HRESULT D3DGrid11::VOnRestore(Scene* pScene)
{
	HRESULT hr;

	V_RETURN(SceneNode::VOnRestore(pScene));

	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);

	V_RETURN(m_VertexShader.OnRestore(pScene));
	V_RETURN(m_PixelShader.OnRestore(pScene));

	GridRenderComponent* grc = static_cast<GridRenderComponent*>(m_RenderComponent);

	int squares = grc->GetDivision();

	SetRadius(sqrt(squares * squares / 2.0f));

	// Create the vertex buffer - we'll need enough verts
	// to populate the grid. If we want a 2x2 grid, we'll
	// need a 3x3 set of verts.
	m_numVerts = (squares + 1)*(squares + 1);	// Create vertex buffer

	// Fill the vertex buffer. We are setting the tu and tv texture
	// coordinates, which range from 0.0 to 1.0
	D3D11Vertex_UnlitTextured* pVerts = Nv_NEW D3D11Vertex_UnlitTextured[m_numVerts];
	//Nv_ASSERT(pVerts && "Out of memory in D3DGrid11::VOnRestore()");
	if (!pVerts) {
		return E_FAIL;
	}

	for (int j = 0; j < (squares + 1); j++)
	{
		for (int i = 0; i < (squares + 1); i++)
		{
			// Which vertex are we setting?
			int index = i + (j * (squares + 1));
			D3D11Vertex_UnlitTextured* vert = &pVerts[index];

			// Default position of the grid is centered on the origin, flat on
			// the XZ plane.
			float x = (float)i - (squares / 2.0f);
			float y = (float)j - (squares / 2.0f);
			vert->Pos = Vec3(x, 0.f, y);
			vert->Normal = Vec3(0.0f, 1.0f, 0.0f);

			// The texture coordinates are set to x,y to make the 
			// texture tile along with units - 1.0, 2.0, 3.0, etc.
			vert->Uv.x			= x;
			vert->Uv.y			= y;
		}
	}

	//The number of indices equals the number of polygons times 3 
	// since there are 3 indices per polygon. Each grid square contains
	// two polygons. The indices are 16 bit, since our grids won't be that
	// big.
	m_numPolys = squares * squares * 2;

	WORD* pIndices = Nv_NEW WORD[m_numPolys * 3];

	//Nv_ASSERT(pIndices && "Out of memory in D3DGrid11::VOnRestore()");
	if (!pIndices) {
		return E_FAIL;
	}

	// Loop through the grid squares and calculate the values
	// of each index. Each grid square has two triangles:
	//
	//		A - B
	//		| / |
	//		C - D
	//

	WORD* current = pIndices;
	for (int j = 0; j < squares; j++)
	{
		for (int i = 0; i < squares; i++)
		{
			// Triangle #1	ACB
			*(current) = WORD(i + (j*(squares + 1)));
			*(current + 1) = WORD(i + ((j + 1)*(squares + 1)));
			*(current + 2) = WORD((i + 1) + (j * (squares + 1)));

			// Triangle #2		BCD
			*(current + 3) = WORD((i + 1) + (j * (squares + 1)));
			*(current + 4) = WORD(i + ((j + 1)*(squares + 1)));
			*(current + 5) = WORD((i + 1) + ((j + 1)*(squares + 1)));

			current += 6;
		}
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(D3D11Vertex_UnlitTextured) * m_numVerts;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = pVerts;
	hr = DXUTGetD3D11Device()->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
	if (SUCCEEDED(hr))
	{
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(WORD) * m_numPolys * 3;	// 36 vertices needed for 12 triangles in a triangle list.
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		InitData.pSysMem = pIndices;
		hr = DXUTGetD3D11Device()->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
	}

	SAFE_DELETE_ARRAY(pVerts);
	SAFE_DELETE_ARRAY(pIndices);

	return hr;
}

HRESULT D3DGrid11::VRender(Scene* pScene)
{
	HRESULT hr;

	GridRenderComponent* grc = static_cast<GridRenderComponent*>(m_RenderComponent);
	m_PixelShader.SetTexture(grc->GetTextureResource());

	V_RETURN(m_VertexShader.SetupRender(pScene, this));
	V_RETURN(m_PixelShader.SetupRender(pScene, this));

	// Set vertex buffer
	UINT stride = sizeof(D3D11Vertex_UnlitTextured);
	UINT offset = 0;
	DXUTGetD3D11DeviceContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// Set index buffer
	DXUTGetD3D11DeviceContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	DXUTGetD3D11DeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DXUTGetD3D11DeviceContext()->DrawIndexed(m_numPolys * 3, 0, 0);

	return S_OK;
}
