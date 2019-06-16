#pragma once

#include "Common/CommonStd.h"
#include "Initialization/Initialization.h"
#include "BaseAppLogic.h"

// ================================================================
// App.h : Defines the entry point for the application.
// ================================================================

class App {
protected:

	HINSTANCE m_hInstance;			// the module instance
	bool m_bWindowedMode;			// true if the app is windowed, false if fullscreen
	bool m_bQuitting;				// true if the app is running the exit sequence.
	bool m_bIsRunning;				// true if everything is initialized and the game is in the main loop
	float m_screenSize_x;
	float m_screenSize_y;

public:

protected:
	std::map<std::wstring, std::wstring> m_textResource;

	int m_HasModalDialog;					// determines if a modal dialog is up
	void FlashWhileMinimized();

public:
	App(); // Constructor

	// Game Application Data
	// You must define these in an inherited
	// class
	virtual TCHAR *VGetGameTitle()=0;
	virtual TCHAR *VGetGameAppDirectory()=0;
	virtual HICON VGetIcon()=0;

	// Win32 Specific Stuff
	HWND GetHwnd();

	virtual bool InitInstance(HINSTANCE hInstance, LPWSTR lpCmdLine, HWND hWnd = NULL, int screenWidth = SCREEN_WIDTH, int screenHeight = SCREEN_HEIGHT);

	static LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void *pUserContext);
	bool HasModalDialog() { return m_HasModalDialog != 0; }

	LRESULT OnDisplayChange(int colorDepth, int width, int height);
	LRESULT OnPowerBroadcast(int event);
	LRESULT OnSysCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnClose();

	// Game Application actions
	LRESULT OnAltEnter();

	bool LoadStrings(std::string language);

	enum Renderer
	{
		Renderer_Unknown,
		Renderer_D3D11
	};

	static Renderer GetRendererImpl();


	// DirectX 11 Specific Stuff
	static bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo, DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext);
	static HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
	static HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
	static void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext);
	static void CALLBACK OnD3D11DestroyDevice(void* pUserContext);
	static void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext, double fTime, float fElapsedTime, void* pUserContext);

	static bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void *pUserContext);
	static void CALLBACK OnUpdateGame(double fTime, float fElapsedTime, void *pUserContext);

	// App Specific Stuff

	BaseAppLogic *m_pGame;
	struct GameOptions m_Options;


	class ResCache* m_ResCache;

protected:

public:
	// Main loop processing
	int GetExitCode() { return DXUTGetExitCode(); }
	bool IsRunning() { return m_bIsRunning; }


protected:

private:

};

extern App *g_pApp;