#include <direct.h>
#include "Common/CommonStd.h"
#include "Initialization.h"

bool CheckStorage(const DWORDLONG diskSpaceNeeded)
{
	// Check for enough free space disk space on the current disk.
	int const drive = _getdrive();
	struct _diskfree_t diskfree;

	_getdiskfree(drive, &diskfree);

	unsigned __int64 neededClusters = diskSpaceNeeded / (diskfree.sectors_per_cluster * diskfree.bytes_per_sector);

	if (diskfree.avail_clusters < neededClusters)
	{
		// if you get here you don't have enough disk space!
		//Nv_ERROR("CheckStorage Failure: Not enough physical storage.");
		return false;
	}
	return true;
}

bool CheckMemory(const DWORDLONG physicalRAMNeeded, const DWORDLONG virtualRAMNeeded)
{
	MEMORYSTATUSEX status;
	GlobalMemoryStatusEx(&status);
	if (status.ullTotalPhys < physicalRAMNeeded)
	{
		// you don't have enough physical memory. Tell the player to go get a real
		// computer and give this one to his mother.
		//Nv_ERROR("Check Memory Failure: Not enough physical memory");
		return false;
	}

	// Check for enough free memory.
	if (status.ullAvailVirtual < virtualRAMNeeded)
	{
		// you don't have enough virtual memory available.
		//Nv_ERROR("CheckMemory Failure: Not enough virtual memory");
		return false;
	}

	char *buff = Nv_NEW char[(unsigned int)virtualRAMNeeded];
	if (buff) {
		delete[] buff;
	}
	else
	{
		// even though there is enough memory, it isn't available in one
		// block, which can be critical for games that manage their own memory
		//Nv_ERROR("CheckMemory Failure: Not enough contiguous available memory");
		return false;
	}
	return true;
}

DWORD ReadCPUSpeed()
{
	DWORD BufSize = sizeof(DWORD);
	DWORD dwMHz = 0;
	DWORD type = REG_DWORD;
	HKEY hKey;

	// open the key where the proc speed is hidden:
	long lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
		L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
		0, KEY_READ, &hKey);

	if (lError == ERROR_SUCCESS)
	{
		// query the key:
		RegQueryValueEx(hKey, L"~MHz", NULL, &type, (LPBYTE)&dwMHz, &BufSize);
	}

	return dwMHz;
}

extern bool IsOnlyInstance(LPCTSTR gameTitle)
{
	// Find the window. If active, set and return false
	// Only one game instance may have this mutex at a time...

	HANDLE handle = CreateMutex(NULL, TRUE, gameTitle);

	if (GetLastError() != ERROR_SUCCESS)
	{
		HWND hWnd = FindWindow(gameTitle, NULL);
		if (hWnd)
		{
			// An instance of your game is already running.
			ShowWindow(hWnd, SW_SHOWNORMAL);
			SetFocus(hWnd);
			SetForegroundWindow(hWnd);
			SetActiveWindow(hWnd);
			return false;
		}
	}
	return true;
}


bool CheckForJoystick(HWND hWnd)
{
	JOYINFO joyinfo;
	UINT wNumDevs;
	BOOL bDev1Attached, bDev2Attached;

	if ((wNumDevs = joyGetNumDevs()) == 0)
	{
		return false;
	}

	bDev1Attached = joyGetPos(JOYSTICKID1, &joyinfo) != JOYERR_UNPLUGGED;
	bDev2Attached = joyGetPos(JOYSTICKID2, &joyinfo) != JOYERR_UNPLUGGED;

	if (bDev1Attached)
	{
		joySetCapture(hWnd, JOYSTICKID1, 1000 / 30, true);
	}
	if (bDev2Attached)
	{
		joySetCapture(hWnd, JOYSTICKID2, 1000 / 30, true);
	}

	return true;
}

GameOptions::GameOptions()
{
	// set all the options to decent default values
	m_Renderer = "Direct3D 11";

	m_ScreenSize_x = 800.0f;
	m_ScreenSize_y = 600.0f;
}

void GameOptions::Init()
{
	m_Renderer = "Direct3D 11";

	m_ScreenSize_x = 800.0f;
	m_ScreenSize_y = 600.0f;
}

void GameOptions::Init(const char* xmlFilePath, LPWSTR lpCmdLine)
{

}