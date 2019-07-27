#pragma once

//#include "Common/CommonStd.h"

extern bool CheckStorage(const DWORDLONG diskSpaceNeeded);
extern DWORD ReadCPUSpeed();
extern bool CheckMemory(const DWORDLONG physicalRAMNeeded, const DWORDLONG virtualRAMNeeded);
extern bool IsOnlyInstance(LPCTSTR gameTitle);
extern bool CheckForJoystick(HWND hWnd);

struct GameOptions
{
	// Level option
	std::string m_Level;

	// Rendering options
	std::string m_Renderer;

	// TODO change for custom Point() class
	float m_ScreenSize_x;
	float m_ScreenSize_y;

	// Sound options

	// Multiplayer options
	int m_expectedPlayers;
	int m_listenPort;
	std::string m_gameHost;
	int m_numAIs;
	int m_maxAIs;
	int m_maxPlayers;

	// resource cache options


	// TiXmlElement - look at this to find other options added by the developer
	TiXmlDocument* m_pDoc;

	GameOptions();
	~GameOptions() { };

	void Init(const char* xmlFilePath, LPWSTR lpCmdLine);
	void Init();
};