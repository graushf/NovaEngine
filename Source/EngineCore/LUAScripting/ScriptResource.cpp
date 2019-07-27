// ================================================================
// ScriptResource.cpp : Defines a simple resource cache 
// ================================================================

#include "../Common/CommonStd.h"
#include "ScriptResource.h"
#include "LuaStateManager.h"

bool ScriptResourceLoader::VLoadResource(char* rawBuffer, unsigned int rawSize, std::shared_ptr<ResHandle> handle)
{
	if (rawSize <= 0) {
		return false;
	}

	if (!g_pApp->m_pGame || g_pApp->m_pGame->CanRunLua()) {
		LuaStateManager::Get()->VExecuteString(rawBuffer);
	}

	return true;
}

std::shared_ptr<IResourceLoader> CreateScriptResourceLoader()
{
	return std::shared_ptr<IResourceLoader>(Nv_NEW ScriptResourceLoader());
}