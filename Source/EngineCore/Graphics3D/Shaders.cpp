// -----------------------------------------------------------------------------------
// Shaders.cpp - helper classes to call Nova_VSMain_VS.hlsl and Nova_VSMain_PS.hlsl
// -----------------------------------------------------------------------------------

#include "../Common/CommonStd.h"

#include "../App/App.h"

#include "D3DRenderer.h"
#include "Geometry.h"
#include "Lights.h"
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
	HRESULT hr = S_OK;

	SAFE_RELEASE(m_pVertexLayout11);
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pcbVSMatrices);
	SAFE_RELEASE(m_pcbVSLighting);
	SAFE_RELEASE(m_pcbVSMaterial);

	std::shared_ptr<D3DRenderer11> d3dRenderer11 = static_pointer_cast<D3DRenderer11>(pScene->GetRenderer());

	// =======================================================
	// Set up the vertex shader and related constant buffers

	// Compile the vertex shader using the lowest possible profile for broadest feature level support
	ID3DBlob* pVertexShaderBuffer = NULL;

	std::string hlslFileName = "Effects\\Sh_VSMain_VS.hlsl";
	Resource resource(hlslFileName.c_str());
	std::shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource); // this actually loads the HLSL file from the zip file
	if (FAILED(d3dRenderer11->CompileShader(pResourceHandle->Buffer(), pResourceHandle->Size(), hlslFileName.c_str(), "Nova_VSMain", "vs_4_0_level_11_0", &pVertexShaderBuffer)))
	{
		SAFE_RELEASE(pVertexShaderBuffer);
		return hr;
	}

	if (FAILED(DXUTGetD3D11Device()->CreateVertexShader(pVertexShaderBuffer->GetBufferPointer(), 
															pVertexShaderBuffer->GetBufferSize(), NULL, &m_pVertexShader)))
	{
		SAFE_RELEASE(pVertexShaderBuffer);
		return hr;
	}

	DXUT_SetDebugName(m_pVertexShader, "Nova_VSMain");

	if (SUCCEEDED(DXUTGetD3D11Device()->CreateInputLayout(D3D11VertexLayout_UnlitTextured, ARRAYSIZE(D3D11VertexLayout_UnlitTextured), pVertexShaderBuffer->GetBufferPointer(),
		pVertexShaderBuffer->GetBufferSize(), &m_pVertexLayout11)))
	{
		DXUT_SetDebugName(m_pVertexLayout11, "Primary");

		// Setup constant buffers
		D3D11_BUFFER_DESC Desc;
		Desc.Usage = D3D11_USAGE_DYNAMIC;
		Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		Desc.MiscFlags = 0;

		Desc.ByteWidth = sizeof(ConstantBuffer_Matrices);
		V_RETURN(DXUTGetD3D11Device()->CreateBuffer(&Desc, NULL, &m_pcbVSMatrices));
		DXUT_SetDebugName(m_pcbVSMatrices, "ConstantBuffer_Matrices");

		Desc.ByteWidth = sizeof(ConstantBuffer_Lighting);
		V_RETURN(DXUTGetD3D11Device()->CreateBuffer(&Desc, NULL, &m_pcbVSLighting));
		DXUT_SetDebugName(m_pcbPSLighting, "ConstantBuffer_Lighting");

		Desc.ByteWidth = sizeof(ConstantBuffer_Material);
		V_RETURN(DXUTGetD3D11Device()->CreateBuffer(&Desc, NULL, &m_pcbVSMaterial));
		DXUT_SetDebugName(m_pcbVSMaterial, "ConstantBuffer_Material");
	}

	SAFE_RELEASE(pVertexShaderBuffer);
	return S_OK;
}

HRESULT Nova_Hlsl_VertexShader::SetupRender(Scene* pScene, SceneNode* pNode)
{
	HRESULT hr = S_OK;

	// Set the vertex shader and the vertex layout
	DXUTGetD3D11DeviceContext()->VSSetShader(m_pVertexShader, NULL, 0);
	DXUTGetD3D11DeviceContext()->IASetInputLayout(m_pVertexLayout11);

	// Get the projection & view matrix from the camera class
	Mat4x4 mWorldViewProjection = pScene->GetCamera()->GetWorldViewProjection(pScene);
	Mat4x4 mWorld = pScene->GetTopMatrix();

	D3D11_MAPPED_SUBRESOURCE MappedResource;

	// -------- Transform Matrices --------------
	V(DXUTGetD3D11DeviceContext()->Map(m_pcbVSMatrices, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));

	ConstantBuffer_Matrices* pVSMatrices = (ConstantBuffer_Matrices*)MappedResource.pData;
	D3DXMatrixTranspose(&pVSMatrices->m_WorldViewProj, &mWorldViewProjection);
	D3DXMatrixTranspose(&pVSMatrices->m_World, &mWorld);

	DXUTGetD3D11DeviceContext()->Unmap(m_pcbVSMatrices, 0);

	// ------- Lighting -----------
	V(DXUTGetD3D11DeviceContext()->Map(m_pcbVSLighting, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	ConstantBuffer_Lighting* pLighting = (ConstantBuffer_Lighting*)MappedResource.pData;

	if (m_enableLights) {
		pScene->GetLightManager()->CalcLighting(pLighting, pNode);
	}
	else
	{
		pLighting->m_nNumLights = 0;
		pLighting->m_vLightAmbient = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	DXUTGetD3D11DeviceContext()->Unmap(m_pcbVSLighting, 0);

	// -------- Material -----------
	V(DXUTGetD3D11DeviceContext()->Map(m_pcbVSMaterial, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource));
	ConstantBuffer_Material* pPSMaterial = (ConstantBuffer_Material*)MappedResource.pData;

	Color color = pNode->VGet()->GetMaterial().GetDiffuse();
	pPSMaterial->m_vDiffuseObjectColor = Vec4(color.r, color.g, color.b, color.a);
	color = (m_enableLights) ? pNode->VGet()->GetMaterial().GetAmbient() : Color(1.0f, 1.0f, 1.0f, 1.0f);
	pPSMaterial->m_vAmbientObjectColor = Vec4(color.r, color.g, color.b, color.a);
	// Note - the vertex shader doesn't care about the texture one way or another so we'll just set it to false
	pPSMaterial->m_bHasTexture = false;

	DXUTGetD3D11DeviceContext()->Unmap(m_pcbVSMaterial, 0);

	DXUTGetD3D11DeviceContext()->VSSetConstantBuffers(0, 1, &m_pcbVSMatrices);
	DXUTGetD3D11DeviceContext()->VSSetConstantBuffers(1, 1, &m_pcbVSLighting);
	DXUTGetD3D11DeviceContext()->VSSetConstantBuffers(2, 1, &m_pcbVSMaterial);

	return S_OK;
}