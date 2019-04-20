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
#include <d3dx9tex.h>
#include <dxut.h>
#include <SDKmisc.h>

#include <tinyxml.h>

extern const int MEGABYTE;

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