#pragma once

// ---------------------------------------------------------------------------
// Sky.h - Implements a sky box in D3D11
// ---------------------------------------------------------------------------

#include "Geometry.h"
#include "Material.h"
#include "Shaders.h"

// Forward declarations
class SceneNode;
class Scene;

// ------------------------------------------------------------
//
// class SkyNode						- Chapter 16, page 554
//
//	Implements a believable sky that follows
//  the camera around - this is a base class that the D3D11 class
//	inherits from.
//
// ------------------------------------------------------------
class SkyNode : public SceneNode
{
protected:
	DWORD			m_numVerts;
	DWORD			m_sides;
	const char*		m_textureBaseName;
	std::shared_ptr<CameraNode>	m_camera;
	bool						m_bActive;

	std::string GetTextureName(const int side);

public:
	SkyNode(const char* textureFile);
	virtual ~SkyNode() { }
	HRESULT VPreRender(Scene* pScene);
	bool VIsVisible(Scene* pScene) const { return m_bActive; }
};

class D3DSkyNode11 : public SkyNode
{
public:
	D3DSkyNode11(const char* pTextureBaseName);
	virtual ~D3DSkyNode11();
	HRESULT VOnRestore(Scene* pScene);
	HRESULT VRender(Scene* pScene);

protected:
	ID3D11Buffer*				m_pIndexBuffer;
	ID3D11Buffer*				m_pVertexBuffer;

	Nova_Hlsl_VertexShader			m_VertexShader;
	Nova_Hlsl_PixelShader			m_PixelShader;
};
