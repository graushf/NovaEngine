// ================================================================================
// D3DRenderer.h : Implements the D3D11 renderer implementation
//
// ================================================================================

#include "Common/CommonStd.h"
#include "D3DRenderer.h"

// You should leave this global - it does wacky things otherwise.
CDXUTDialogResourceManager D3DRenderer::g_DialogResourceManager;
CDXUTTextHelper* D3DRenderer::g_pTextHelper = nullptr;


//
// class D3DRendererAlphaPass11				- Chapter 16, page 543
//
class D3DRendererAlphaPass11 : public IRenderState
{
protected:
	ID3D11BlendState* m_pOldBlendState;
	FLOAT m_OldBlendFactor[4];
	UINT m_OldSampleMask;

	ID3D11BlendState* m_pCurrentBlendState;

public:
	D3DRendererAlphaPass11();
	~D3DRendererAlphaPass11();
	std::string VToString() { return "D3DRendererAlphaPass11"; }
};

//
// D3DRendererAlphaPass11::D3DRendererAlphaPass11					- Chapter 16, page 544
//
D3DRendererAlphaPass11::D3DRendererAlphaPass11()
{
	DXUTGetD3D11DeviceContext()->OMGetBlendState(&m_pOldBlendState, m_OldBlendFactor, &m_OldSampleMask);
	m_pCurrentBlendState = NULL;

	D3D11_BLEND_DESC BlendState;
	ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC));

	BlendState.AlphaToCoverageEnable = false;
	BlendState.IndependentBlendEnable = false;
	BlendState.RenderTarget[0].BlendEnable = TRUE;
	BlendState.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendState.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BlendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	BlendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	DXUTGetD3D11Device()->CreateBlendState(&BlendState, &m_pCurrentBlendState);
	DXUTGetD3D11DeviceContext()->OMSetBlendState(m_pCurrentBlendState, 0, 0xffffffff);
}

//
// D3DRendererAlphaPass11:~D3DRendererAlphaPass11					- Chapter 16, page 544
//
D3DRendererAlphaPass11::~D3DRendererAlphaPass11()
{
	DXUTGetD3D11DeviceContext()->OMSetBlendState(m_pOldBlendState, m_OldBlendFactor, m_OldSampleMask);
	SAFE_RELEASE(m_pCurrentBlendState);
	SAFE_RELEASE(m_pOldBlendState);
}

//
// clss D3DRendererSkyBoxPass11										- Chapter 16, page 548
//
class D3DRendererSkyBoxPass11 : public IRenderState
{
protected:
	ID3D11DepthStencilState* m_pOldDepthStencilState;
	ID3D11DepthStencilState* m_pSkyboxDepthStencilState;

public:
	D3DRendererSkyBoxPass11();
	~D3DRendererSkyBoxPass11();
	std::string VToString() { return "D3DRendererSkyBoxPass11"; }
};

//
// D3DRendererSkyBoxPass11::D3DRendererSkyBoxPass11()				- Chapter 16, page 548
//
D3DRendererSkyBoxPass11::D3DRendererSkyBoxPass11()
{
	// Depth stencil state
	D3D11_DEPTH_STENCIL_DESC DSDesc;
	ZeroMemory(&DSDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	DSDesc.DepthEnable = true;
	DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	DSDesc.DepthFunc = D3D11_COMPARISON_LESS;
	DSDesc.StencilEnable = FALSE;
	DXUTGetD3D11Device()->CreateDepthStencilState(&DSDesc, &m_pSkyboxDepthStencilState);
	DXUT_SetDebugName(m_pSkyboxDepthStencilState, "SkyboxDepthStencil");

	UINT StencilRef;
	DXUTGetD3D11DeviceContext()->OMGetDepthStencilState(&m_pOldDepthStencilState, &StencilRef);
	DXUTGetD3D11DeviceContext()->OMSetDepthStencilState(m_pSkyboxDepthStencilState, 0);
}

//
// D3DRendererSkyBoxPass11::~D4DRendererSkyBoxPass11()					- Chapter 16, page 548
//
D3DRendererSkyBoxPass11::~D3DRendererSkyBoxPass11()
{
	DXUTGetD3D11DeviceContext()->OMSetDepthStencilState(m_pOldDepthStencilState, 0);
	SAFE_RELEASE(m_pOldDepthStencilState);
	SAFE_RELEASE(m_pSkyboxDepthStencilState);
}

// -----------------------------------------------
// D3DRenderer11 Implementation
//
//		Not descibed in the book - but it abstracts
//		some of the calls to get the game engine
//		to run under DX11.
//
// -----------------------------------------------

HRESULT D3DRenderer11::VOnRestore() 
{
	HRESULT hr;
	V_RETURN(D3DRenderer::VOnRestore());
	SAFE_DELETE(D3DRenderer::g_pTextHelper);
	D3DRenderer::g_pTextHelper = Nv_NEW CDXUTTextHelper(DXUTGetD3D11Device(), DXUTGetD3D11DeviceContext(), &g_DialogResourceManager, 15);

	return S_OK;
}

// ------------------------------------------------------
// Helper for compiling shaders with D3DX11
// ------------------------------------------------------
HRESULT D3DRenderer11::CompileShader(LPCSTR pSrcData, SIZE_T SrcDataLen, LPCSTR pFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined (DEBUG) || defined(_DEBUG)
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows
	// the shaders to be optimized and to run exactly the way they will run in
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromMemory(pSrcData, SrcDataLen, pFileName, NULL, NULL, szEntryPoint, szShaderModel,
					dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);

	if (FAILED(hr))
	{
		if (pErrorBlob != NULL) {
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		}
		if (pErrorBlob) {
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) {
		pErrorBlob->Release();
	}

	return S_OK;
}

HRESULT D3DRenderer11::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)


#endif

	ID3DBlob* pErrorBlob;
	hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
			dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);

	if (FAILED(hr))
	{
		if (pErrorBlob != NULL) {
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		}
		if (pErrorBlob) {
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) {
		pErrorBlob->Release();
	}

	return S_OK;
}

std::shared_ptr<IRenderState> D3DRenderer11::VPrepareAlphaPass()
{
	return std::shared_ptr<IRenderState>(Nv_NEW D3DRendererAlphaPass11());
}

std::shared_ptr<IRenderState> D3DRenderer11::VPrepareSkyBoxPass()
{
	return std::shared_ptr<IRenderState>(Nv_NEW D3DRendererSkyBoxPass11());
}