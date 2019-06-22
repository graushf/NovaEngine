// ================================================================================
// D3DRenderer.h : Implements the D3D11 renderer implementation
//
// ================================================================================

#include "Common/CommonStd.h"
#include "D3DRenderer.h"

// You should leave this global - it does wacky things otherwise.
CDXUTDialogResourceManager D3DRenderer::g_DialogResourceManager;
CDXUTTextHelper* D3DRenderer::g_pTextHelper = nullptr;

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

