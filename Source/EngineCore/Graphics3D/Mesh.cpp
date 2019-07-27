// ================================================================
// Mesh.cpp : Classes to render meshes in D3D11
// ================================================================

#include "../Common/CommonStd.h"

#include <SDKmisc.h>

#include "../App/App.h"
#include "../ResourceCache/ResCache.h"
#include "D3DRenderer.h"
#include "Lights.h"
#include "Mesh.h"
#include "Raycast.h"
#include "SceneNodes.h"


std::shared_ptr<IResourceLoader> CreateSdkMeshResourceLoader()
{
	return std::shared_ptr<IResourceLoader>(Nv_NEW SdkMeshResourceLoader());
}

unsigned int SdkMeshResourceLoader::VGetLoadedResourceSize(char* rawBuffer, unsigned int rawSize)
{
	// the raw data of the SDK Mesh file is needed by the CDXUTMesh class, so we're going to keep it around.
	return rawSize;
}

//
// SdkMeshResourceLoader::VLoadResource								- Chapter 16, page 561
//
bool SdkMeshResourceLoader::VLoadResource(char* rawBuffer, unsigned int rawSize, std::shared_ptr<ResHandle> handle)
{
	App::Renderer renderer = App::GetRendererImpl();
	if (renderer == App::Renderer_D3D11) 
	{
		std::shared_ptr<D3DSdkMeshResourceExtraData11> extra = std::shared_ptr<D3DSdkMeshResourceExtraData11>(Nv_NEW D3DSdkMeshResourceExtraData11());

		// Load the Mesh
		if (SUCCEEDED(extra->m_Mesh11.Create(DXUTGetD3D11Device(), (BYTE*)rawBuffer, (UINT)rawSize, true)))
		{
			handle->SetExtra(std::shared_ptr<D3DSdkMeshResourceExtraData11>(extra));
		}

		return true;
	}

	//Nv_ASSERT(0 && "Unsupported Renderer in SdkMeshResourceLoader::VLoadResource");
	return false;
}

D3DShaderMeshNode11::D3DShaderMeshNode11(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent,
	std::string sdkMeshFileName,
	RenderPass renderPass,
	const Mat4x4* t)
: SceneNode(actorId, renderComponent, renderPass, t)
{
	m_sdkMeshFileName = sdkMeshFileName;
}


// 
// D3DShaderMeshNode11::VOnRestore						- Chapter 16, page 563
//
HRESULT D3DShaderMeshNode11::VOnRestore(Scene* pScene)
{
	HRESULT hr;

	V_RETURN(SceneNode::VOnRestore(pScene));

	V_RETURN(m_VertexShader.OnRestore(pScene));
	V_RETURN(m_PixelShader.OnRestore(pScene));

	// Force the Mesh to reload
	Resource resource(m_sdkMeshFileName);
	std::shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);
	std::shared_ptr<D3DSdkMeshResourceExtraData11> extra = static_pointer_cast<D3DSdkMeshResourceExtraData11>(pResourceHandle->GetExtra());

	SetRadius(CalcBoundingSphere(&extra->m_Mesh11));

	return S_OK;
}

//
// D3DShaderMeshNode11::VRender							- Chapter 16, page 564
//
HRESULT D3DShaderMeshNode11::VRender(Scene* pScene)
{
	HRESULT hr;

	V_RETURN(m_VertexShader.SetupRender(pScene, this));
	V_RETURN(m_PixelShader.SetupRender(pScene, this));

	// Get the Mesh
	Resource resource(m_sdkMeshFileName);
	std::shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);
	std::shared_ptr<D3DSdkMeshResourceExtraData11> extra = static_pointer_cast<D3DSdkMeshResourceExtraData11>(pResourceHandle->GetExtra());

	// FUTURE WORK - this code WON'T be able to find texture resources referred to by the sdkmesh file
	// in the Resource cache.

	// IA stup
	UINT Strides[1];
	UINT Offsets[1];
	ID3D11Buffer* pVB[1];
	pVB[0] = extra->m_Mesh11.GetVB11(0, 0);
	Strides[0] = (UINT)extra->m_Mesh11.GetVertexStride(0, 0);
	Offsets[0] = 0;
	DXUTGetD3D11DeviceContext()->IASetVertexBuffers(0, 1, pVB, Strides, Offsets);
	DXUTGetD3D11DeviceContext()->IASetIndexBuffer(extra->m_Mesh11.GetIB11(0), extra->m_Mesh11.GetIBFormat11(0), 0);

	// Render
	D3D11_PRIMITIVE_TOPOLOGY PrimType;
	for (UINT subset = 0; subset < extra->m_Mesh11.GetNumSubsets(0); ++subset)
	{
		// Get the subset
		SDKMESH_SUBSET* pSubset = extra->m_Mesh11.GetSubset(0, subset);

		PrimType = CDXUTSDKMesh::GetPrimitiveType11((SDKMESH_PRIMITIVE_TYPE)pSubset->PrimitiveType);
		DXUTGetD3D11DeviceContext()->IASetPrimitiveTopology(PrimType);

		ID3D11ShaderResourceView* pDiffuseRV = extra->m_Mesh11.GetMaterial(pSubset->MaterialID)->pDiffuseRV11;
		DXUTGetD3D11DeviceContext()->PSSetShaderResources(0, 1, &pDiffuseRV);

		DXUTGetD3D11DeviceContext()->DrawIndexed((UINT)pSubset->IndexCount, 0, (UINT)pSubset->VertexStart);
	}

	return S_OK;
}

HRESULT D3DShaderMeshNode11::VPick(Scene* pScene, RayCast* pRayCast)
{
	if (SceneNode::VPick(pScene, pRayCast) == E_FAIL) {
		return E_FAIL;
	}

	pScene->PushAndSetMatrix(m_Props.ToWorld());

	// Get the Mesh
	Resource resource(m_sdkMeshFileName);
	std::shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);
	std::shared_ptr<D3DSdkMeshResourceExtraData11> extra = static_pointer_cast<D3DSdkMeshResourceExtraData11>(pResourceHandle->GetExtra());

	HRESULT hr = pRayCast->Pick(pScene, m_Props.ActorId(), &extra->m_Mesh11);
	pScene->PopMatrix();

	return hr;
}

float D3DShaderMeshNode11::CalcBoundingSphere(CDXUTSDKMesh* mesh11)
{
	float radius = 0.0f;
	for (UINT subset = 0; subset < mesh11->GetNumSubsets(0); ++subset)
	{
		D3DXVECTOR3 aux = mesh11->GetMeshBBoxExtents(subset);
		Vec3 extents = aux;
		extents.x = abs(extents.x);
		extents.y = abs(extents.y);
		extents.z = abs(extents.z);
		radius = (radius > extents.x) ? radius : extents.x;
		radius = (radius > extents.y) ? radius : extents.y;
		radius = (radius > extents.z) ? radius : extents.z;
	}
	return radius;
}