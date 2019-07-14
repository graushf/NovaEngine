// ---------------------------------------------------------------------------
// Lights.cpp - implements a simple light class for the App scene graph
// ---------------------------------------------------------------------------

#include "../Common/CommonStd.h"

#include "../App/App.h"
#include "../Actors/RenderComponent.h"
#include "Lights.h"

LightNode::LightNode(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, const LightProperties& props, const Mat4x4* t)
	: SceneNode(actorId, renderComponent, RenderPass_NotRendered, t)
{
	m_LightProps = props;
}

HRESULT D3DLightNode11::VOnUpdate(Scene *, DWORD const elapsedMs)
{
	// light color can change anytime! Check the BaseRenderComponent!
	LightRenderComponent* lrc = static_cast<LightRenderComponent*>(m_RenderComponent);
	m_Props.GetMaterial().SetDiffuse(lrc->GetColor());
	return S_OK;
}

//
// LightManager::CalcLighting					- Chapter 16, page 554
//
void LightManager::CalcLighting(Scene* pScene)
{
	// FUTURE WORK: There might be all kinds of things you'd want to do here for optimization, especially turning off lights on actors that can't be seen, etc.
	pScene->GetRenderer()->VCalcLighting(&m_Lights, MAXIMUM_LIGHTS_SUPPORTED);

	int count = 0;

	//Nv_ASSERT(m_Lights.size() < MAXIMUM_LIGHTS_SUPPORTED);
	for (Lights::iterator i = m_Lights.begin(); i != m_Lights.end(); ++i, ++count)
	{
		std::shared_ptr<LightNode> light = *i;

		if (count == 0)
		{
			// Light 0 is the only one we use for ambient lighting. The rest are ignored in the simple shaders used for NovaEngine.
			Color ambient = light->VGet()->GetMaterial().GetAmbient();
			D3DXVECTOR4 ambientAux = D3DXVECTOR4(ambient.r, ambient.g, ambient.b, 1.0f);
			m_vLightAmbient.x = ambientAux.x;
			m_vLightAmbient.y = ambientAux.y;
			m_vLightAmbient.z = ambientAux.z;
			m_vLightAmbient.w = ambientAux.w;

		}

		Vec3 lightDir = light->GetDirection();
		D3DXVECTOR4 lightDirAux = D3DXVECTOR4(lightDir.x, lightDir.y, lightDir.z, 1.0f);
		m_vLightDir[count].x = lightDirAux.x;
		m_vLightDir[count].y = lightDirAux.y;
		m_vLightDir[count].z = lightDirAux.z;
		m_vLightDir[count].w = lightDirAux.w;

		m_vLightDiffuse[count] = light->VGet()->GetMaterial().GetDiffuse();
	}
}

void LightManager::CalcLighting(ConstantBuffer_Lighting* pLighting, SceneNode* pNode)
{
	int count = GetLightCount(pNode);
	if (count)
	{
		pLighting->m_vLightAmbient = *GetLightAmbient(pNode);
		memcpy(pLighting->m_vLightDir, GetLightDirection(pNode), sizeof(Vec4) * count);
		memcpy(pLighting->m_vLightDiffuse, GetLightDiffuse(pNode), sizeof(Vec4) * count);
		pLighting->m_nNumLights = count;
	}
}