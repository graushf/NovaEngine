#pragma once

// -----------------------------------------------------------------------------------
// Shaders.h - helper classes to call Nova_VSMain_VS.hlsl and Nova_VSMain_PS.hlsl
// -----------------------------------------------------------------------------------

// ===================================================================================
// Content References in Game Coding Complete 4th Edition
//
// class Nova_Hlsl_VertexShader					- Chapter 15, page 508
// class Nova_Hlsl_PixelShader						- Chapter 15, page 515
//

#include "../Common/CommonStd.h"
#include <xnamath.h>

#include "Geometry.h"
#include "Material.h"

// Forward declarations
class SceneNode;
class Scene;

class Nova_Hlsl_VertexShader
{
public:
	Nova_Hlsl_VertexShader();
	~Nova_Hlsl_VertexShader();

	HRESULT OnRestore(Scene* pScene);
	HRESULT SetupRender(Scene* pScene, SceneNode* pNode);
	void EnableLights(bool enableLights) { m_enableLights = enableLights; }

protected:
	ID3D11InputLayout*		m_pVertexLayout11;
	ID3D11VertexShader*		m_pVertexShader;
	ID3D11Buffer*			m_pcbVSMatrices;
	ID3D11Buffer*			m_pcbVSLighting;
	ID3D11Buffer*			m_pcbVSMaterial;
	bool					m_enableLights;
};

// class with PixelShader --
class Nova_Hlsl_PixelShader
{
public:
	Nova_Hlsl_PixelShader();
	~Nova_Hlsl_PixelShader();

	HRESULT OnRestore(Scene* pScene);
	HRESULT SetupRender(Scene* pScene, SceneNode* pNode);
	HRESULT SetTexture(const std::string& textureName);
	HRESULT SetTexture(ID3D11ShaderResourceView* const* pDiffuseRV, ID3D11SamplerState* const* ppSamplers);

protected:
	ID3D11PixelShader*			m_pPixelShader;
	ID3D11Buffer*				m_pcbPSMaterial;
	std::string					m_textureResource;
};
