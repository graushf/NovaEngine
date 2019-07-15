#pragma once

// ================================================================
// Mesh.h : Classes to render meshes in D3D11
// ================================================================

#include <SDKMesh.h>

#include "Geometry.h"
#include "../ResourceCache/ResCache.h"

//
// class D3DSdkMeshResourceExtraData11					- Chapter 16, page 561
//
class D3DSdkMeshResourceExtraData11 : public IResourceExtraData
{
	friend class SdkMeshResourceLoader;

public:
	D3DSdkMeshResourceExtraData11() { };
	virtual ~D3DSdkMeshResourceExtraData11() { }
	virtual std::string VToString() { return "D3DSdkMeshResourceExtraData11"; }

	CDXUTSDKMesh			m_Mesh11;
};

//
// class SdkMeshResourceLoader							- Chapter 16, page  561
//
class SdkMeshResourceLoader : public IResourceLoader
{
public:
	virtual bool VUseRawFile() { return false; }
	virtual bool VDiscardRawBufferAfterLoad() { return false; }
	virtual unsigned int VGetLoadedResourceSize(char* rawBuffer, unsigned int rawSize);
	virtual bool VLoadResource(char* rawBuffer, unsigned int rawSize, std::shared_ptr<ResHandle> handle);
	virtual std::string VGetPattern() { return "*.sdkmesh"; }
};


class D3DShaderMeshNode11 : public SceneNode
{
public:
	D3DShaderMeshNode11(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent,
		std::string sdkMeshFileName,
		RenderPass renderPass,
		const Mat4x4* t);

	virtual HRESULT VOnRestore(Scene* pScene);
	virtual HRESULT VOnLostDevice(Scene* pScene) { return S_OK; }
	virtual HRESULT VRender(Scene* pScene);
	virtual HRESULT VPick(Scene* pScene, RayCast* pRayCast);

protected:
	std::string		m_sdkMeshFileName;

	Nova_Hlsl_VertexShader	m_VertexShader;
	Nova_Hlsl_PixelShader	m_PixelShader;

	float CalcBoundingSphere(CDXUTSDKMesh* mesh11);				// this was added post-press.
};