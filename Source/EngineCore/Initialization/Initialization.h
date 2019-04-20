#pragma once

//#include "Common/CommonStd.h"

extern bool CheckStorage(const DWORDLONG diskSpaceNeeded);
extern DWORD ReadCPUSpeed();
extern bool CheckMemory(const DWORDLONG physicalRAMNeeded, const DWORDLONG virtualRAMNeeded);
extern bool IsOnlyInstance(LPCTSTR gameTitle);
extern bool CheckForJoystick(HWND hWnd);

struct GameOptions
{
	// Rendering options
	std::string m_Renderer;

	// TODO change for custom Point() class
	float m_ScreenSize_x;
	float m_ScreenSize_y;

	GameOptions();
	~GameOptions() { };

	void Init(const char* xmlFilePath, LPWSTR lpCmdLine);
	void Init();
};