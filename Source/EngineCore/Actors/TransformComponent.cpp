// ===========================================================================================
// TransformComponent.cpp - Component for managing tranforms on actors
// ===========================================================================================

#include "../Common/CommonStd.h"
#include "TransformComponent.h"
#include "../Utilities/Math.h"
#include "../Utilities/String.h"

const char* TransformComponent::g_Name = "TransformComponent";

bool TransformComponent::VInit(TiXmlElement* pData)
{
	//Nv_ASSERT(pData);

	// [mrmike] - this was changed post-press - because changes to the TransformComponents can come in partial definitions,
	//			  such as from the editor, its better to grab the current values rather than clear them out.

	Vec3 yawPitchRoll = m_transform.GetYawPitchRoll();
	yawPitchRoll.x = RADIANS_TO_DEGREES(yawPitchRoll.x);
	yawPitchRoll.y = RADIANS_TO_DEGREES(yawPitchRoll.y);
	yawPitchRoll.z = RADIANS_TO_DEGREES(yawPitchRoll.z);

	Vec3 position = m_transform.GetPosition();

	TiXmlElement* pPositionElement = pData->FirstChildElement("Position");
	if (pPositionElement)
	{
		double x = 0;
		double y = 0;
		double z = 0;
		pPositionElement->Attribute("x", &x);
		pPositionElement->Attribute("y", &y);
		pPositionElement->Attribute("z", &z);
		position = Vec3(x, y, z);
	}

	TiXmlElement* pOrientationElement = pData->FirstChildElement("YawPitchRoll");
	if (pOrientationElement)
	{
		double yaw = 0;
		double pitch = 0;
		double roll = 0;
		pOrientationElement->Attribute("x", &yaw);
		pOrientationElement->Attribute("y", &pitch);
		pOrientationElement->Attribute("z", &roll);
		yawPitchRoll = Vec3(yaw, pitch, roll);
	}

	Mat4x4 translation;
	translation.BuildTranslation(position);

	Mat4x4 rotation;
	rotation.BuildYawPitchRoll((float)DEGREES_TO_RADIANS(yawPitchRoll.x), (float)DEGREES_TO_RADIANS(yawPitchRoll.y), (float)DEGREES_TO_RADIANS(yawPitchRoll.z));

	/**
	// This is not supported yet.
	TiXmlElement* pLookAtElement = pData->FirstChildElement("LookAt");
	if (pLookAtElement)
	{
		double x = 0;
		double y = 0;
		double z = 0;
		pLookAtElement->Attribute("x", &x);
		pLookAtElement->Attribute("y", &y);
		pLookAtElement->Attribute("z", &z);

		Vec3 lookAt((float)x, (float)y, (float)z);
		rotation.BuildRotationLookAt(translation.GetPosition(), lookAt, g_Up);
	}

	TiXmlElement* pScaleElement = pData->FirstChildElement("Scale");
	if (pScaleElement)
	{
		double x = 0;
		double y = 0;
		double z = 0;
		pScaleElement->Attribute("x", &x);
		pScaleElement->Attribute("y", &y);
		pScaleElement->Attribute("z", &z);
		m_scale = Vec3((float)x, (float)y, (float)z);
	}
	**/

	m_transform = rotation * translation;

	return true;
}

TiXmlElement* TransformComponent::VGenerateXml(void)
{
	TiXmlElement* pBaseElement = Nv_NEW TiXmlElement(VGetName());

	// initial transform -> position
	TiXmlElement* pPosition = Nv_NEW TiXmlElement("Position");
	Vec3 pos(m_transform.GetPosition());
	pPosition->SetAttribute("x", ToStr(pos.x).c_str());
	pPosition->SetAttribute("y", ToStr(pos.y).c_str());
	pPosition->SetAttribute("z", ToStr(pos.z).c_str());
	pBaseElement->LinkEndChild(pPosition);

	// initial transform -> LookAt
	TiXmlElement* pDirection = Nv_NEW TiXmlElement("YawPitchRoll");
	Vec3 orient(m_transform.GetYawPitchRoll());
	orient.x = RADIANS_TO_DEGREES(orient.x);
	orient.y = RADIANS_TO_DEGREES(orient.y);
	orient.z = RADIANS_TO_DEGREES(orient.z);
	pDirection->SetAttribute("x", ToStr(orient.x).c_str());
	pDirection->SetAttribute("y", ToStr(orient.y).c_str());
	pDirection->SetAttribute("z", ToStr(orient.z).c_str());
	pBaseElement->LinkEndChild(pDirection);

	/***
	// This is not supported yet
	// initial transform -> position
	TiXmlElement* pScale = Nv_NEW TiXmlElement("Scale");
	pPosition->SetAttribute("x", ToStr(m_scale.x).c_str());
	pPosition->SetAttribute("y", ToStr(m_scale.y).c_str());
	pPosition->SetAttribute("z", ToStr(m_scale.z).c_str());
	pBaseElement->LinkEndChild(pScale);
	**/

	return pBaseElement;
}