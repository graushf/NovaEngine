#include "Common/CommonStd.h"

#include "Initialization/Initialization.h"

App *g_App = nullptr;

App::App()
{

}

HWND App::GetHwnd()
{
	return DXUTGetHWND();
}

// ======================================================================================
//
// InitInstance - this checks system resources, creates your window, and launches the game
//
// preprocessor macro setting in the C++ options of the project provides the base macro
// C preprocessor string concatenation takes care of the rest.
//
// GameCodeApp::InitInstance - Chapter 5, page 136
//
// ======================================================================================
bool App::InitInstance(HINSTANCE hInstance, LPWSTR lpCmdLine, HWND hWnd, int screenWidth, int screenHeight)
{
	// Check for existing instance of the same window
	//
#ifndef _DEBUG
	// Note it can be really useful to debug network code to hace more
	// than one instance of the game up at one time - so feel free to
	// comment these lines in or out as you wish!
	if (!IsOnlyInstance(VGetGameTitle()))
	{
		return false;
	}
#endif

	// We don't need a mouse cursor by default, let the game turn it on
	//SetCursor(NULL);

	// Check for adequate machine resources.
	bool resourceCheck = false;
	while (!resourceCheck)
	{
		const DWORDLONG physicalRAM = 512 * MEGABYTE;
		const DWORDLONG virtualRAM = 1024 * MEGABYTE;
		const DWORDLONG diskSpace = 10 * MEGABYTE;
		if (!CheckStorage(diskSpace))
		{
			return false;
		}

		const DWORD minCpuSpeed = 1300;
		DWORD thisCPU = ReadCPUSpeed();
		if (thisCPU < minCpuSpeed)
		{
			//Nv_ERROR("GetCPUSpeed reports CPU is too slow for this game.");
			return false;
		}

		resourceCheck = true;
	}

	m_hInstance = hInstance;

	// register all events
	//RegisterEngineEvents();
	//VRegisterGameEvents();

	// 
	// Initialize the ResCache - Chapter 5, page 141
	//
	//	Note - this is a little different from the book. Here we have a special resource ZIP file class, DevelopmentResourceZipFile,
	//  that actually reads directly from the source asset files, rather than the ZIP file. This is MUCH better during development, since
	//  you don't have to rebuild the ZIP file every time you make aminor change to an asset.
	//
	
	/*
	IResourceFile *zipFile = (m_bIsEditorRunning || m_Options.m_useDevelopmentDirectories) ?
		Nv_NEW DevelopmentResourceZipFile(L"Assets.zip", DevelopmentResourceZipFile::Editor) :
		Nv_NEW ResourceZipFile(L"Assets.zip");

	m_ResCache = Nv_NEW ResCache(50, zipFile);

	if (!m_ResCache->Init())
	{
		//Nv_ERROR("Failed to initialize resource cache! Are your paths set up correctly?");
		return false;
	}*/

	/*
	extern shared_ptr<IResourceLoader> CreateWAVResourceLoader();
	extern shared_ptr<IResourceLoader> CreateOGGResourceLoader();
	extern shared_ptr<IResourceLoader> CreateDDSResourceLoader();
	extern shared_ptr<IResourceLoader> CreateJPGResourceLoader();
	extern shared_ptr<IResourceLoader> CreateXmlResourceLoader();
	extern shared_ptr<IResourceLoader> CreateSdkMeshResourceLoader();
	extern shared_ptr<IResourceLoader> CreateScriptResourceLoader();
	*/

	// Note - register these in order from least specific to most specific! They get pushed onto a list.
	// RegisterLoader is discussed in Chapter 5, page 142
	/*
	m_ResCache->RegisterLoader(CreateWAVResourceLoader());
	m_ResCache->RegisterLoader(CreateOGGResourceLoader());
	m_ResCache->RegisterLoader(CreateDDSResourceLoader());
	m_ResCache->RegisterLoader(CreateJPGResourceLoader());
	m_ResCache->RegisterLoader(CreateXmlResourceLoader());
	m_ResCache->RegisterLoader(CreateSdkMeshResourceLoader());
	m_ResCache->RegisterLoader(CreateScriptResourceLoader());
	*/

	
	if (!LoadStrings("English"))
	{
		//Nv_ERROR("Failed to load strings");
		return false;
	}
	

	// Up the Lua State manager now, and run the initial script - discussed in Chapter 5, page 144
	/*
	if (!LuaStateManager::Create())
	{
		Nv_ERROR("Failed to initialize Lua");
		return false;
	}
	*/

	// Load the preinit file. This is within braces to create a scope and destroy the resource once it's loaded. We
	// don't need to do anything with it, we just need to load it.
	/*
	{
		Resource resource(SCRIPT_PREINIT_FILE);
		shared_ptr<ResHandle> pResourceHandle = m_ResCache->GetHandle(&resource);	// this actually loads the XML file from the zip file
	} */

	// Register function exported from C++
	/*
	ScriptExports::Register();
	ScriptProcess::RegisterScriptClass();
	BaseScriptComponent::RegisterScriptFunctions(); */

	// The event manager should be created next so that subsystems can hook in as desired.
	// Discussed in Chapter 5, page 144.
	/*
	m_pEventManager = Nv_NEW EventManager("NovaEngine Event Mgr", true);
	if (!m_pEventManager)
	{
		Nv_ERROR("Failed to create EventManager.");
		return false;
	}
	*/

	// DXUTInit, DXUTCreateWindow - Chapter 5, page 145
	DXUTInit(true, true, lpCmdLine, true);	// Parse the command line, handle the default hotkeys, and show msgboxes

	if (hWnd == NULL)
	{
		DXUTCreateWindow(VGetGameTitle(), hInstance, VGetIcon());
	}
	else
	{
		DXUTSetWindow(hWnd, hWnd, hWnd);
	}

	if (!GetHwnd())
	{
		return FALSE;
	}
	SetWindowText(GetHwnd(), VGetGameTitle());

	// initialize the directory location you can store save game files
	//_tcscpy_s(m_saveGameDirectory, GetSaveGameDirectory(GetHwnd(), VGetGameAppDirectory()));

	// DXUTCreateDevice - Chapter 5, page 139
	m_screenSize_x = screenWidth;
	m_screenSize_y = screenHeight;

	DXUTCreateDevice(D3D_FEATURE_LEVEL_11_0, true, screenWidth, screenHeight);

	if (GetRendererImpl() == Renderer_D3D11)
	{
		//m_Renderer = shared_ptr<IRenderer>(Nv_NEW D3DRenderer11());
	}
	//m_Renderer->VSetBackgroundColor(255, 20, 20, 200);
	//m_Renderer->VOnRestore();

	// You usually must have an HWND to initalize your game views...
	//		VCreateGameAndView			- Chapter 5, page 145
	
	/*
	m_pGame = VCreateGameAndView();
	if (!m_pGame)
	{
		return false;
	} */

	// now that all the major systems are initalized, preload resources
	//		Preload calls are discussed in Chapter 5, page 148.
	/*
	m_ResCache->Preload("*.ogg", NULL);
	m_ResCache->Preload("*.dds", NULL);23
	m_ResCache->Preload("*.jpg", NULL);

	if (App::GetRendererImpl() == App::Renderer_D3D11)
	{
		m_ResCache->Preload("*.sdkmesh", NULL);
	}
	*/

	CheckForJoystick(GetHwnd());

	m_bIsRunning = true;

	return TRUE;
}

bool App::LoadStrings(std::string language)
{
	//std::string languageFile = "Strings\\";
	//languageFile += language;
	//languageFile += ".xml";

	//TiXmlElement* pRoot = XMLResourceLoader::LoadAndReturnRootXmlElement(languageFile.c_str());
	//if (!pRoot)
	//{
	//	//Nv_ERROR("Strings are missing.");
	//	return false;
	//}

	//// Loop through each child element and load the content
	//for (TiXmlElement* pElem = pRoot->FirstChildElement(); pElem; pElem = pElem->NextSiblingElement())
	//{
	//	const char *pKey = pElem->Attribute("id");
	//	const char *pText = pElem->Attribute("value");
	//	const char *pHotkey = pElem->Attribute("hotkey");

	//	if (pKey && pText)
	//	{
	//		wchar_t wideKey[64];
	//		wchar_t wideText[1024];
	//		AnsiToWideCch(wideKey, pKey, 64);
	//		AnsiToWideCch(wideKey, pText, 1024);
	//		m_textResource[std::wstring(wideKey)] = std::wstring(wideText);

	//		if (pHotkey)
	//		{
	//			m_hotkeys[std::wstring(wideKey)] = MapCharToKeycode(*pHotkey);
	//		}
	//	}
	//}

	return true;
}




//------------------------------------------------------------------------
// Win32 Specific Message Handlers
//
// WndProc - the main message handler for the window class.
//
// OnNcCreate - this is where you can set window data before it is created
// OnMove - called whenever the window moves; used to update members of g_App
// OnDeviceChange - called whenever you eject the CD-ROM.
// OnDisplayChange - called whenever the user changes the desktop settings
// OnPowerBroadcast - called whenever a power message forces a shutdown.
// OnActivate - called whenever windows on the desktop change focus.
//
// Note: pUserContext added to comply with DirectX 9c - June 2005 Update
//
//------------------------------------------------------------------------
LRESULT CALLBACK App::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void *pUserContext)
{
	// Always allow dialog resource manager calls to handle global messages
	// so GUI state is updated correctly
	/**pbNoFurtherProcessing = D3DRenderer::g_DialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
	if (*pbNoFurtherProcessing) {
		return 0;
	}*/

	LRESULT result = 0;

	switch (uMsg)
	{
		case WM_POWERBROADCAST:
		{
			int event = (int)wParam;
			result = g_pApp->OnPowerBroadcast(event);
			break;
		}

		case WM_DISPLAYCHANGE:
		{
			int colorDepth = (int)wParam;
			int width = (int)(short)LOWORD(lParam);
			int height = (int)(short)HIWORD(lParam);

			result = g_pApp->OnDisplayChange(colorDepth, width, height);
			break;
		}

		case WM_SYSCOMMAND:
		{
			result = g_pApp->OnSysCommand(wParam, lParam);
			if (result)
			{
				*pbNoFurtherProcessing = true;
			}
			break;
		}

		case WM_SYSKEYDOWN:
		{
			if (wParam == VK_RETURN)
			{
				*pbNoFurtherProcessing = true;
				return g_pApp->OnAltEnter();
			}
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}

		case WM_CLOSE:
		{
			// DXUT apps choose ESC key as a default exit command.
			// App doesnt't like this so we disable it by checking
			// the m_bQuitting bool, and if we're not really quitting
			// set the "no further processing" parameter to true.
			if (g_pApp->m_bQuitting)
			{
				result = g_pApp->OnClose();
			}
			else
			{
				*pbNoFurtherProcessing = true;
			}
			break;
		}

		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case MM_JOY1BUTTONDOWN:
		case MM_JOY1BUTTONUP:
		case MM_JOY1MOVE:
		case MM_JOY1ZMOVE:
		case MM_JOY2BUTTONDOWN:
		case MM_JOY2BUTTONUP:
		case MM_JOY2MOVE:
		case MM_JOY2ZMOVE:
		{
			//
			// See Chapter 10, page 278 for more explanation of this code.
			//
			/*if (g_pApp->m_pGame)
			{
				BaseGameLogic *pGame = g_pApp->m_pGame;
				// Note the reverse order! User input is grabbed first from the view that is on top, 
				// which is the last one in the list.
				AppMsg msg;
				msg.m_hWnd = hWnd;
				msg.m_uMsg = uMsg;
				msg.m_wParam = wParam;
				msg.m_lParam = lParam;
				for (GameViewList::reverse_iterator i = pGame->m_gameViews.rbegin(); i != pGame->m_gameViews.rend(); ++i)
				{
					if ((*i)->VOnMsgProc(msg))
					{
						result = true;
						break;				// WARNING! This breaks out of the for loop.
					}
				}
			} */
			break;
		}

		/**********************
		WARNING!!!!! You MIGHT think you need this, but if you use the DirectX
		Framework the DefWindowProc is called for you....

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);

		***********************/
	}

	return result;
}


void App::FlashWhileMinimized()
{
	// Flash the application on the taskbar
	// until it's restored
	if (!GetHwnd())
	{
		return;
	}

	// Blink the application if we are minimized,
	// waiting until we are no longer minimized
	if (IsIconic(GetHwnd()))
	{
		// Make sure the app is up when creating a new screen
		// this should be the case most of the time, but when
		// we close the app down, minimized, and a confirmation
		// dialog appears, we need to restore
		DWORD now = timeGetTime();
		DWORD then = now;
		MSG msg;

		FlashWindow(GetHwnd(), true);

		while (true)
		{
			if (PeekMessage(&msg, NULL, 0, 0, 0))
			{
				if (msg.message != WM_SYSCOMMAND || msg.wParam != SC_CLOSE)
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}

				// Are we done?
				if (!IsIconic(GetHwnd()))
				{
					FlashWindow(GetHwnd(), false);
					break;
				}
			}
			else
			{
				now = timeGetTime();
				DWORD timeSpan = now > then ? (now - then) : (then - now);
				if (timeSpan > 1000)
				{
					then = now;
					FlashWindow(GetHwnd(), true);
				}
			}
		}
	}
}

// -----------------------------------------------------------------------------------------
// Handles the WM_SYSCOMMAND message
// -----------------------------------------------------------------------------------------
LRESULT App::OnSysCommand(WPARAM wParam, LPARAM lParam)
{
	//switch (wParam)
	//{
	//	case SC_MAXIMIZE:
	//	{
	//		// If windowed and ready...
	//		if (m_bWindowedMode && IsRunning())
	//		{
	//			// Make maximize into FULLSCREEN toggle
	//			OnAltEnter();
	//		}
	//	}
	//	return 0;

	//	case SC_CLOSE:
	//	{
	//		// The quit dialog confirmation would appear once for every
	//		// SC_CLOSE we get - which happens multiple times if modal
	//		// dialogs are up. This now uses the g_QuitNoPrompt
	//		// flag to only prompt when receiving a SC_CLOSE that isn't 
	//		// generated by us (identified by g_QuitNoPrompt).

	//		// If closing, prompt to close if this isn't a forced quit
	//		if (lParam != g_QuitNoPrompt)
	//		{
	//			// Bug
	//			// Begin. We are receiving multiple close dialogs
	//			// when closing again ALT-F4 while the close 
	//			// confirmation dialog was up.
	//			// Eat if already servicing a close.
	//			if (m_bQuitRequested) {
	//				return true;
	//			}

	//			// Wait for the application to be restored
	//			// before going any further with the new screen.
	//			// Flash until the person selects taht they want 
	//			// to restore the game, then reinit the display if
	//			// fullscreen.
	//			// The reinit is necessary otherwise the game will
	//			// switch to windowed mode.

	//			// Quit requested
	//			m_bQuitRequested = true;
	//			// Prompt
	//			if (MessageBox::Ask(QUESTION_QUIT_GAME) == IDNO)
	//			{
	//				// Bail - quit aborted

	//				// Reset quit requested flag
	//				m_bQuitRequested = false;

	//				return true;
	//			}
	//		}
	//		m_bQuitting = true;

	//		// Is there a game modal dialog up?
	//		if (HasModalDialog())
	//		{
	//			// Close the modal
	//			// and keep posting close to the app
	//			ForceModalExit();

	//			// Reissue the close to the app

	//			// Issue the new close after handling the current one,
	//			// but send in g_QuitNoPrompt to differentiate it from a
	//			// regular CLOSE issued by the system.
	//			PostMessage(GetHwnd(), WM_SYSCOMMAND, SC_CLOSE, g_QuitNoPrompt);

	//			m_bQuitRequested = false;

	//			// Eat the close
	//			return true;
	//		}

	//		// Reset the qut after any other dialogs have popped up from this close
	//		m_bQuitRequested = false;
	//	}
	//	return 0;

	//	default:
	//		// return non-zero if we didn't process the SYSCOMMAND message
	//		return DefWindowProc(GetHwnd(), WM_SYSCOMMAND, wParam, lParam);
	//}

	return 0;
}


// -----------------------------------------------------------------------------------------
// Handles the WM_CLOSE message
// -----------------------------------------------------------------------------------------
LRESULT App::OnClose()
{
	// release all the game systems in reverse order from which they were created
	//SAFE_DELETE(m_pGame);

	DestroyWindow(GetHwnd());

	//VDestroyNetworkEventForwarder();

	//SAFE_DELETE(m_pBaseSocketManager);

	//SAFE_DELETE(m_pEventManager);

	//BaseScriptComponent::UnregisterScriptFunctions();
	//ScriptExports::Unregister();
	//LuaStateManager::Destroy();

	//SAFE_DELETE(m_ResCache);

	return 0;
}


// -----------------------------------------------------------------------------------------
// Called right before creating a D3D9 or D3D11 device, allowing the app to modify the device settings as needed
// -----------------------------------------------------------------------------------------
bool CALLBACK App::ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void *pUserContext)
{
	// For the first device created if its a REF device, optionally display a warning dialog box
	static bool s_bFirstTime = true;
	if (s_bFirstTime)
	{
		s_bFirstTime = false;
		if ((DXUT_D3D11_DEVICE == pDeviceSettings->ver && pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE))
		{
			DXUTDisplaySwitchingToREFWarning(pDeviceSettings->ver);
		}
	}

	return true;
}

// -----------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not
// intended to contain actual rendering calls, which should instead be placed in the
// OnFrameRender callback.
//
// See Game Coding Complete - 4th Edition - 
// -----------------------------------------------------------------------------------------
void CALLBACK App::OnUpdateGame(double fTime, float fElapsedTime, void *pUserContext)
{
	/*
	if (g_pApp->HasModalDialog())
	{
		// don't update the game if a modal is dialog is up.
		return;
	}

	if (g_pApp->m_bQuitting)
	{
		PostMessage(g_pApp->GetHwnd(), WM_CLOSE, 0, 0);
	}

	if (g_pApp->m_pGame)
	{
		IEventManager::Get()->VUpdate(20);	// allow event queue to process for up to 20 ms

		if (g_pApp->m_pBaseSocketManager)
		{
			g_pApp->m_pBaseSocketManager->DoSelect(0);	// pause 0 microseconds
		}
		
		g_pApp->m_pGame->VOnUpdate(float(fTime), fElapsedTime);
	}
	*/
}

App::Renderer App::GetRendererImpl()
{
	if (DXUTGetDeviceSettings().ver == DXUT_D3D11_DEVICE)
	{
		return Renderer_D3D11;
	}
	else {
		return Renderer_Unknown;
	}
}

// -----------------------------------------------------------------------------------------
// Reject any D3D11 devices that aren't acceptable by returning false
// -----------------------------------------------------------------------------------------
bool CALLBACK App::IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo, DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
{
	return true;
}

// -----------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
// -----------------------------------------------------------------------------------------
HRESULT CALLBACK App::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	HRESULT hr;

	//ID3D11DeviceContext* pd3dImmediateContext = DXUTGetD3D11DeviceContext();
	//V_RETURN(D3DRenderer::g_DialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dImmediateContext));

	return S_OK;
}

// -----------------------------------------------------------------------------------------
// Create any D3D11 resources that depend on the back buffer
// -----------------------------------------------------------------------------------------
HRESULT CALLBACK App::OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	HRESULT hr;

	/*
	V_RETURN(D3DRenderer::g_DialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

	if (g_pApp->m_pGame)
	{
		BaseGameLogic* pGame = g_pApp->m_pGame;
		for (GameViewList::iterator i = pGame->m_gameViews.begin(); i != pGame->m_gameViews.end(); ++i)
		{
			(*i)->VOnRestore();
		}
	}*/

	return S_OK;
}

// -----------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11ResizedSwapChain
// -----------------------------------------------------------------------------------------
void CALLBACK App::OnD3D11ReleasingSwapChain(void* pUserContext)
{
	//D3DRenderer::g_DialogResourceManager.OnD3D11ReleasingSwapChain();
}

// -----------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice
// -----------------------------------------------------------------------------------------
void CALLBACK App::OnD3D11DestroyDevice(void* pUserContext)
{
	/*
	if (g_pApp->m_Renderer) // Fixfor multi-monitor issue when target monitor is portrait
	{
		g_pApp->m_Renderer->VShutdown();
		D3DRenderer::g_DialogResourceManager.OnD3D11DestroyDevice();
		g_pApp->m_Renderer = shared_ptr<IRenderer>(NULL);
	} */
}


// -----------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
// -----------------------------------------------------------------------------------------
void CALLBACK App::OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext)
{
	/*
	BaseGameLogic *pGame = g_pApp->m_pGame;

	for (GameViewList::iterator i = pGame->m_gameViews.begin(), end = pGame->m_gameViews.end(); i != end; ++i)
	{
		(*i)->VOnRender(fTime, fElapsedTime);
	}

	g_pApp->m_pGame->VRenderDiagnostics();
	*/
}