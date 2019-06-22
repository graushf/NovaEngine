#pragma once

#define WIN32_LEAN_AND_MEAN

// Windoesws header files:

#define NOMINMAX
#include <windows.h>
//#include <windowsx.h>

#include <crtdbg.h>

// C Runtime Header files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <mmsystem.h>

#include <algorithm>
#include <string>
#include <list>
#include <vector>
#include <queue>
#include <map>

#if _MSC_VER >= 1700
#include <memory>
using std::shared_ptr;
using std::weak_ptr;
using std::static_pointer_cast;
using std::dynamic_pointer_cast;
#elif
using std::tr1::shared_ptr;
using std::tr1::weak_ptr;
using std::tr1::static_pointer_cast;
using std::tr1::dynamic_pointer_cast;
#endif

// Game Code Complete - Chapter 12, page 446-447
#if defined(_DEBUG)
#	define Nv_NEW new(_NORMAL_BLOCK,__FILE__, __LINE__)
#else
#	define Nv_NEW new
#endif

#define DXUT_AUTOLIB

// DirectX Includes
#include <dxut.h>
//#include "../ThirdParty/DXUT11/Core/DXUT.h"
#include <d3dx9tex.h>
#include <dxut.h>
#include <SDKmisc.h>

#include <tinyxml.h>


// App #includes
#include "Common/templates.h"
#include "Graphics3D/Geometry.h"




typedef D3DXCOLOR Color;

extern Color g_White;
extern Color g_Black;
extern Color g_Cyan;
extern Color g_Red;
extern Color g_Green;
extern Color g_Blue;
extern Color g_Yellow;
extern Color g_Gray40;
extern Color g_Gray25;
extern Color g_Gray65;
extern Color g_Transparent;

extern Vec3 g_Up;
extern Vec3 g_Right;
extern Vec3 g_Forward;

extern Vec4 g_Up4;
extern Vec4 g_Right4;
extern Vec4 g_Forward4;


// AppMsg					- Chapter 9, page 248
struct AppMsg
{
	HWND m_hWnd;
	UINT m_uMsg;
	WPARAM m_wParam;
	LPARAM m_lParam;
};

#include "Common/interfaces.h"

// Useful #defines

extern const float fOPAQUE;
extern const int iOPAQUE;
extern const float fTRANSPARENT;
extern const int iTRANSPARENT;

extern const int MEGABYTE;
extern const float SIXTY_HERTZ;

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

//#include "App/AppInterfaces.h"
#include "App/App.h"

extern INT WINAPI AppInst
(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR    lpCmdLine,
	int       nCmdShow
);