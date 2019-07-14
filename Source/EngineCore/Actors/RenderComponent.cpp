// ---------------------------------------------------------------------------
// RenderComponent.cpp - classes that define renderable components of actors
//						 like Meshes, Skyboxes, Lights, etc.
// ---------------------------------------------------------------------------

#include "../Common/CommonStd.h"

#include "../Utilities/String.h"

#include "../EventManager/Events.h"

#include "RenderComponent.h"
#include "TransformComponent.h"

const char* LightRenderComponent::g_Name = "LightRenderComponent";

// =====================================================================
// RenderComponent
// =====================================================================
bool BaseRenderComponent::VInit(TiXmlElement* pData)
{
	// color
	TiXmlElement* pColorNode = pData->FirstChildElement("Color");
	if (pColorNode) {
		m_color = LoadColor(pColorNode);
	}

	return VDelegateInit(pData);
}

void BaseRenderComponent::VPostInit(void)
{
	std::shared_ptr<SceneNode> pSceneNode(VGetSceneNode());
	std::shared_ptr<EvtData_New_Render_Component> pEvent(Nv_NEW EvtData_New_Render_Component(m_pOwner->GetId(), pSceneNode));
	IEventManager::Get()->VTriggerEvent(pEvent);
}

void BaseRenderComponent::VOnChanged(void)
{
	std::shared_ptr<EvtData_Modified_Render_Component> pEvent(Nv_NEW EvtData_Modified_Render_Component(m_pOwner->GetId()));
	IEventManager::Get()->VTriggerEvent(pEvent);
}

TiXmlElement* BaseRenderComponent::VGenerateXml(void)
{
	TiXmlElement* pBaseElement = VCreateBaseElement();

	// color
	TiXmlElement* pColor = Nv_NEW TiXmlElement("Color");
	pColor->SetAttribute("r", ToStr(m_color.r).c_str());
	pColor->SetAttribute("g", ToStr(m_color.r).c_str());
	pColor->SetAttribute("b", ToStr(m_color.r).c_str());
	pColor->SetAttribute("a", ToStr(m_color.r).c_str());
	pBaseElement->LinkEndChild(pColor);

	// create XML for inherited classes
	VCreateInheritedXmlElements(pBaseElement);

	return pBaseElement;
}


std::shared_ptr<SceneNode> BaseRenderComponent::VGetSceneNode(void)
{
	if (!m_pSceneNode) {
		m_pSceneNode = VCreateSceneNode();
	}

	return m_pSceneNode;
}

Color BaseRenderComponent::LoadColor(TiXmlElement* pData)
{
	Color color;

	double r = 1.0;
	double g = 1.0;
	double b = 1.0;
	double a = 1.0;

	pData->Attribute("r", &r);
	pData->Attribute("g", &g);
	pData->Attribute("b", &b);
	pData->Attribute("a", &a);

	color.r = (float)r;
	color.g = (float)g;
	color.b = (float)b;
	color.a = (float)a;

	return color;
}

// =================================================================================
// LightRenderComponent
// =================================================================================
LightRenderComponent::LightRenderComponent(void)
{
}

bool LightRenderComponent::VDelegateInit(TiXmlElement* pData)
{
	TiXmlElement* pLight = pData->FirstChildElement("Light");

	double temp;
	TiXmlElement* pAttenuationNode = NULL;

	pAttenuationNode = pLight->FirstChildElement("Attenuation");
	if (pAttenuationNode)
	{
		double temp;
		pAttenuationNode->Attribute("const", &temp);
		m_Props.m_Attenuation[0] = (float)temp;

		pAttenuationNode->Attribute("linear", &temp);
		m_Props.m_Attenuation[1] = (float)temp;

		pAttenuationNode->Attribute("exp", &temp);
		m_Props.m_Attenuation[2] = (float)temp;
	}

	TiXmlElement* pShapeNode = NULL;
	pShapeNode = pLight->FirstChildElement("Shape");
	if (pShapeNode)
	{
		pShapeNode->Attribute("range", &temp);
		m_Props.m_Range = (float)temp;
		pShapeNode->Attribute("falloff", &temp);
		m_Props.m_Falloff = (float)temp;
		pShapeNode->Attribute("theta", &temp);
		m_Props.m_Theta = (float)temp;
		pShapeNode->Attribute("phi", &temp);
		m_Props.m_Phi = (float)temp;
	}
	return true;
}

std::shared_ptr<SceneNode> LightRenderComponent::VCreateSceneNode(void)
{
	std::shared_ptr<TransformComponent> pTransformComponent = MakeStrongPtr(m_pOwner->GetComponent<TransformComponent>(TransformComponent::g_Name));
	if (pTransformComponent)
	{
		WeakBaseRenderComponentPtr weakThis(this);

		switch (App::GetRendererImpl())
		{
			case App::Renderer_D3D11:
				return std::shared_ptr<SceneNode>(Nv_NEW D3DLightNode11(m_pOwner->GetId(), weakThis, m_Props, &(pTransformComponent->GetTransform())));

			default:
				//Nv_ASSERT(0 && "Unknown Renderer Implementation in GridRenderComponent");
		}
	}
	return std::shared_ptr<SceneNode>();
}

void LightRenderComponent::VCreateInheritedXmlElements(TiXmlElement* pBaseElement)
{
	TiXmlElement* pSceneNode = Nv_NEW TiXmlElement("Light");

	// attenuation
	TiXmlElement* pAttenuation = Nv_NEW TiXmlElement("Attenuation");
	pAttenuation->SetAttribute("const", ToStr(m_Props.m_Attenuation[0]).c_str());
	pAttenuation->SetAttribute("linear", ToStr(m_Props.m_Attenuation[1]).c_str());
	pAttenuation->SetAttribute("exp", ToStr(m_Props.m_Attenuation[2]).c_str());
	pSceneNode->LinkEndChild(pAttenuation);

	// shape
	TiXmlElement* pShape = Nv_NEW TiXmlElement("Shape");
	pShape->SetAttribute("range", ToStr(m_Props.m_Range).c_str());
	pShape->SetAttribute("falloff", ToStr(m_Props.m_Falloff).c_str());
	pShape->SetAttribute("theta", ToStr(m_Props.m_Theta).c_str());
	pShape->SetAttribute("phi", ToStr(m_Props.m_Phi).c_str());
	pSceneNode->LinkEndChild(pShape);

	pBaseElement->LinkEndChild(pSceneNode);
}
