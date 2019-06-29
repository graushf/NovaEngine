#pragma once

//========================================================================
// LuaStateManager.h : Manages the state of the Lua scripting system
//========================================================================

#include "Common/CommonStd.h"
#include "LuaPlus.h"

// -----------------------------------------------------------------------------
// LuaStateManager										- Chapter 12, page 367
// -----------------------------------------------------------------------------
class LuaStateManager : public IScriptManager
{
	static LuaStateManager* s_pSingleton;
	LuaPlus::LuaState* m_pLuaState;
	std::string m_lastError;

public:
	// Singleton functions
	static bool Create(void);
	static void Destroy(void);
	static LuaStateManager* Get(void) {
		//sNv_ASSERT(s_pSingleton);
		return s_pSingleton;
	}

	// IScriptManager interface
	virtual bool VInit(void) override;
	virtual void VExecuteFile(const char* resource) override;
	virtual void VExecuteString(const char* str) override;

	LuaPlus::LuaObject GetGlobalVars(void);
	LuaPlus::LuaState* GetLuaState(void) const;

	// public helpers
	LuaPlus::LuaObject CreatePath(const char* pathString, bool toIgnoreLastElement = false);
	void ConvertVec3ToTable(const Vec3& vec, LuaPlus::LuaObject& outLuaTable) const;
	void ConvertTableToVec3(const LuaPlus::LuaObject& luaTable, Vec3& outVec3) const;

private:
	void SetError(int errorNum);
	void ClearStack(void);

	// private constructor & destructor; call the static Create() and Destroy() functions instead
	explicit LuaStateManager(void);
	virtual ~LuaStateManager(void);
};