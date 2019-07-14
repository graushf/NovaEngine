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
struct LightProperties
{
	float m_Attenuation[3]; /* Attenuation coefficients */
	float m_Range;
	float m_Falloff;
	float m_Theta;
	float m_Phi;
};


//
// class LightNode						- Chapter 16, 553
//
// Note: In the book this class implements the LightNode in D3D11, but here it is a base 
// class. The derived classes make it possible to run the engine in D3D9 or D3D11. Nova Engine
// only runs on DirectX11 for the moment tho.
class LightNode : public SceneNode
{
protected: 
	LightProperties m_LightProps;

public:
	LightNode(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, const LightProperties &props, const Mat4x4* t);
};


class D3DLightNode11 : public LightNode
{
public:
	D3DLightNode11(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, const LightProperties& lightProps, const Mat4x4* t)
		: LightNode(actorId, renderComponent, lightProps, t) { }

	virtual HRESULT VOnRestore() { return S_OK; }
	virtual HRESULT VOnUpdate(Scene*, DWORD const elapsedMs);
};

struct ConstantBuffer_Lighting;


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
