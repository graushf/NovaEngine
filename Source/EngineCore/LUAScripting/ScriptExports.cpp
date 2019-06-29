//========================================================================
// ScriptExports.cpp : 
//========================================================================

#include "Common/CommonStd.h"
#include "ScriptExports.h"
#include "LuaStateManager.h"
#include "ResourceCache/ResCache.h"
#include <set>

// -------------------------------------------------------------------------------------------
// Prototypes for the functions to export.						- Chapter 12, page 368
// -------------------------------------------------------------------------------------------
class InternalScriptExports
{
public:
	// initialization
	static bool Init(void);
	static void Destroy(void);

	// These are exported to Lua
	static bool LoadAndExecuteScriptResource(const char* scriptResource);
};

bool InternalScriptExports::LoadAndExecuteScriptResource(const char* scriptResource)
{
	Resource resource(scriptResource);

	std::shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);

	if (pResourceHandle)
		return true;

	return false;
}

void ScriptExports::Register(void)
{
	LuaPlus::LuaObject globals = LuaStateManager::Get()->GetGlobalVars();

	// init
	InternalScriptExports::Init();

	// resource loading
	globals.RegisterDirect("LoadAndExecuteScriptResource", InternalScriptExports::LoadAndExecuteScriptResource);
}

void ScriptExports::Unregister(void)
{
	InternalScriptExports::Destroy();
}