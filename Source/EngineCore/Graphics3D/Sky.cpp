// ---------------------------------------------------------------------------
// Sky.cpp - Implements a sky box in D3D11
// ---------------------------------------------------------------------------

#include "../Common/CommonStd.h"

#include "../App/App.h"

#include "D3DRenderer.h"
#include "Geometry.h"
#include "SceneNodes.h"
#include "Shaders.h"
#include "Sky.h"

// ----------------------------------------------------------------
// SkyNode Implementation
// ----------------------------------------------------------------

//
// SkyNode::SkyNode									- Chapter 16, page 554
//
SkyNode::SkyNode(const char* pTextureBaseName)
	: SceneNode(INVALID_ACTOR_ID, WeakBaseRenderComponentPtr(), RenderPass_Sky, &Mat4x4::g_Identity)
	, m_bActive(true)
{
	m_textureBaseName = pTextureBaseName;
}

//
// SkyNode::VPreRender								- Chapter 14, page 502
//
HRESULT SkyNode::VPreRender(Scene* pScene)
{
	Vec3 cameraPos = m_camera->VGet()->ToWorld().GetPosition();
	Mat4x4 mat = m_Props.ToWorld();
	mat.SetPosition(cameraPos);
	VSetTransform(&mat);

	return SceneNode::VPreRender(pScene);
}

//
// D3DSkyNode11::D3DSkyNode11						- Chapter 16, page 555
//
D3DSkyNode11::D3DSkyNode11(const char* pTextureBaseName)
	: SkyNode(pTextureBaseName)
{
	m_pVertexBuffer = NULL;
	m_pIndexBuffer = NULL;
	m_VertexShader.EnableLights(false);
}

D3DSkyNode11::~D3DSkyNode11()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);
}


//
// D3DSkyNode11::VOnRestore							- Chapter 16, page 556
//
HRESULT D3DSkyNode11::VOnRestore(Scene* pScene)
{
	HRESULT hr;

	V_RETURN(SceneNode::VOnRestore(pScene));

	m_camera = pScene->GetCamera();

	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pIndexBuffer);

	V_RETURN(m_VertexShader.OnRestore(pScene));
	V_RETURN(m_PixelShader.OnRestore(pScene));

	m_numVerts = 20;

	// Fill the vertex buffer. We are setting the tu and tv texture
	// coordinates, which range from 0.0 to 1.0
	D3D11Vertex_UnlitTextured* pVertices = Nv_NEW D3D11Vertex_UnlitTextured[m_numVerts];
	//Nv_ASSERT(pVertices && "Out of memory in D3DSkyNode11::VOnRestore()");
	if (!pVertices) {
		return E_FAIL;
	}

	// Loop through the grid squares and calculate the values
	// of each index. Each grid square has two triangles:
	//
	//		A - B
	//		| / |
	//		C - D

	D3D11Vertex_UnlitTextured skyVerts[4];
	D3DCOLOR skyVertColor = 0xffffffff;
	float dim = 50.0f;

	skyVerts[0].Pos = Vec3(dim, dim, dim); 
	skyVerts[0].Uv = Vec2(1.0f, 0.0f);
	skyVerts[1].Pos = Vec3(-dim, dim, dim);
	skyVerts[1].Uv = Vec2(0.0f, 0.0f);
	skyVerts[2].Pos = Vec3(dim, -dim, dim);
	skyVerts[2].Uv = Vec2(1.0f, 1.0f);
	skyVerts[3].Pos = Vec3(-dim, -dim, dim);
	skyVerts[3].Uv = Vec2(0.0, 1.0);

	Vec3 triangle[3];
	triangle[0] = Vec3(0.f, 0.f, 0.f);
	triangle[1] = Vec3(5.f, 0.f, 0.f);
	triangle[2] = Vec3(5.f, 5.f, 0.f);

	Vec3 edge1 = triangle[1] - triangle[0];
	Vec3 edge2 = triangle[2] - triangle[0];

	Vec3 normal;
	normal = edge1.Cross(edge2);
	normal.Normalize();

	Mat4x4 rotY;
	rotY.BuildRotationY(Nv_PI / 2.0f);
	Mat4x4 rotX;
	rotX.BuildRotationX(-Nv_PI / 2.0f);

	m_sides = 5;

	for (DWORD side = 0; side < m_sides; side++)
	{
		for (DWORD v = 0; v < 4; v++)
		{
			Vec4 temp;
			if (side < m_sides - 1)
			{
				Vec3 aux = skyVerts[v].Pos;
				temp = rotY.Xform(aux);
			}
			else
			{
				skyVerts[0].Uv = Vec2(1.0f, 1.0f);
				skyVerts[1].Uv = Vec2(1.0f, 1.0f);
				skyVerts[2].Uv = Vec2(1.0f, 1.0f);
				skyVerts[3].Uv = Vec2(1.0f, 1.0f);

				Vec3 aux = skyVerts[v].Pos;

				temp = rotX.Xform(aux);
			}
			skyVerts[v].Pos = Vec3(temp.x, temp.y, temp.z);
		}
		memcpy(&pVertices[side * 4], skyVerts, sizeof(skyVerts));
	}

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(D3D11Vertex_UnlitTextured) * m_numVerts;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = pVertices;
	hr = DXUTGetD3D11Device()->CreateBuffer(&bd, &InitData, &m_pVertexBuffer);
	SAFE_DELETE(pVertices);
	if (FAILED(hr)) {
		return hr;
	}

	// Loop through the grid squares and calculate the values
	// of each index. Each grid square has two triangles:
	//
	//		A - B
	//		| / |
	//		C - D

	WORD* pIndices = Nv_NEW WORD[m_sides * 2 * 3];

	WORD* current = pIndices;
	for (DWORD i = 0; i < m_sides; i++)
	{
		// Triangle #1		ACB
		*(current) = WORD(i * 4);
		*(current + 1) = WORD(i * 4 + 2);
		*(current + 2) = WORD(i * 4 + 1);

		// Triangle #2		BCD
		*(current + 3) = WORD(i * 4 + 1);
		*(current + 4) = WORD(i * 4 + 2);
		*(current + 5) = WORD(i * 4 + 3);
		current += 6;
	}

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * m_sides * 2 * 3;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	InitData.pSysMem = pIndices;
	hr = DXUTGetD3D11Device()->CreateBuffer(&bd, &InitData, &m_pIndexBuffer);
	SAFE_DELETE_ARRAY(pIndices);
	if (FAILED(hr)) {
		return hr;
	}

	return S_OK;
}


//
// D3DSkyNode11::VRender								- Chapter 16, page 559
//
HRESULT D3DSkyNode11::VRender(Scene* pScene)
{
	HRESULT hr;

	V_RETURN(m_VertexShader.SetupRender(pScene, this));
	V_RETURN(m_PixelShader.SetupRender(pScene, this));

	// Set vertex buffer
	UINT stride = sizeof(D3D11Vertex_UnlitTextured);
	UINT offset = 0;
	DXUTGetD3D11DeviceContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// Set index buffer
	DXUTGetD3D11DeviceContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	DXUTGetD3D11DeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (DWORD side = 0; side < m_sides; side++)
	{
		// FUTURE WORK: A good optimization would be to transform the camera's
		// world look vector into local space, and do a dot product. If the
		// result is positive, we shouldn't draw the side since it has to be
		// behind the camera!

		// Sky boxes aren't culled by the normal mechanism

		/***
		// [mrmike] - This was slightly changed post press, look at the lines below this commented out code
		const char *suffix[] = {"_n.jpg", "_e.jpg", "_s.jpg", "_w.jpg", "_u.jpg" };
		name += suffix[side];
		****/

		std::string name = GetTextureName(side);
		m_PixelShader.SetTexture(name.c_str());

		DXUTGetD3D11DeviceContext()->DrawIndexed(6, side * 6, 0);
	}

	return S_OK;
}

std::string SkyNode::GetTextureName(const int side)
{
	std::string name = m_textureBaseName;
	const char* letters[] = { "n", "e", "s","w","u" };
	unsigned int index = name.find_first_of("_");
	//Nv_ASSERT(index >= 0 && index < name.length() - 1);
	if (index >= 0 && index < name.length() - 1)
	{
		name[index + 1] = *letters[side];
	}
	return name;
}