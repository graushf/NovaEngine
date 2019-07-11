// -----------------------------------------------------------------------------------
// Shaders.cpp - helper classes to call Nova_VSMain_VS.hlsl and Nova_VSMain_PS.hlsl
// -----------------------------------------------------------------------------------

#include "../Common/CommonStd.h"

#include "../App/App.h"

#include "D3DRenderer.h"
#include "Geometry.h"
//#include "Lights.h"
//#include "Mesh.h"
#include "SceneNodes.h"
#include "../ResourceCache/ResCache.h"

#include <xnamath.h>

#include "Shaders.h"

#pragma comment(lib, "effects11.lib")		// [mrmike] Note, you can remove this if you don't want the D3DX11CreateEffectFromMemory API.
#include "d3dx11effect.h"

//
// Nova_Hlsl_VertexShader::Nova_Hlsl_VertexShader			- Chapter 15, page 508
//
Nova_Hlsl_VertexShader::Nova_Hlsl_VertexShader()
{
	m_pVertexLayout11 = NULL;
	m_pVertexShader = NULL;
	m_pcbVSMatrices = NULL;
	m_pcbVSLighting = NULL;
	m_pcbVSMaterial = NULL;
	m_enableLights = true;
}

Nova_Hlsl_VertexShader::~Nova_Hlsl_VertexShader()
{
	SAFE_RELEASE(m_pVertexLayout11);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pcbVSMatrices);
	SAFE_RELEASE(m_pcbVSLighting);
	SAFE_RELEASE(m_pcbVSMaterial);
}

HRESULT Nova_Hlsl_VertexShader::OnRestore(Scene* pScene)
{

}

HRESULT Nova_Hlsl_VertexShader::SetupRender(Scene* pScene, SceneNode* pNode)
{

}