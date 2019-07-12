#pragma once

// ---------------------------------------------------------------------------
// Lights.h - implements a simple light class for the App scene graph
// ---------------------------------------------------------------------------

#include "../Common/CommonStd.h"
#include "D3DRenderer.h"
#include "Scene.h"
#include "SceneNodes.h"

// Note: Light color is sotred in the Material structure, which is already present in all SceneNodes.

//
// struct LightProperties					- Chapter 16, page 551
//


//
// class LightManager						- Chapter 16, 553
//
class LightManager
{
	friend class Scene;

protected:
	Lights		m_Lights;
	Vec4		m_vLightDir[MAXIMUM_LIGHTS_SUPPORTED];
	Color		m_vLightDiffuse[MAXIMUM_LIGHTS_SUPPORTED];
	Vec4		m_vLightAmbient;

public:
	void CalcLighting(Scene* pScene);
	void CalcLighting(ConstantBuffer_Lighting* pLighting, SceneNode* pNode);
	int GetLightCount(const SceneNode* node) { return m_Lights.size(); }
	const Vec4* GetLightAmbient(const SceneNode* node) { return &m_vLightAmbient; }
	const Vec4* GetLightDirection(const SceneNode* node) { return m_vLightDir; }
	const Color* GetLightDiffuse(const SceneNode* node) { return m_vLightDiffuse; }
};
