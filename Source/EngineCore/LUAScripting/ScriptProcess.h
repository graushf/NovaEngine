#pragma once

//========================================================================
// ScriptProcess.h : 
//========================================================================

#include "Common/CommonStd.h"
#include "Mainloop/Process.h"
#include "LuaStateManager.h"

// --------------------------------------------------------------------------------------
// ScriptProcess												-Chapter 12, page 370
// --------------------------------------------------------------------------------------
class ScriptProcess : public Process
{
	unsigned long m_frequency, m_time;
	LuaPlus::LuaObject m_scriptInitFunction, m_scriptUpdateFunction;
	LuaPlus::LuaObject m_scriptSuccessFunction, m_scriptFailFunction, m_scriptAbortFunction;
	LuaPlus::LuaObject m_self;

public:
	static void RegisterScriptClass(void);

protected:
	// Process interface
	virtual void VOnInit(void);
	virtual void VOnUpdate(unsigned long deltaMs);
	virtual void VOnSuccess(void);
	virtual void VOnFail(void);
	virtual void VOnAbort(void);

private:
	// private helpers
	static void RegisterScriptClassFunctions(LuaPlus::LuaObject& metaTableObj);
	static LuaPlus::LuaObject CreateFromScript(LuaPlus::LuaObject self, LuaPlus::LuaObject constructionData, LuaPlus::LuaObject originalSubClass);
	virtual bool BuildCppDataFromScript(LuaPlus::LuaObject scriptClass, LuaPlus::LuaObject constructionData);

	// These are needd because the base-class version of these functions are all const and LuaPlus can't deal 
	// with registering const functions
	bool ScriptIsAlive(void) { return IsAlive(); }
	bool ScriptIsDead(void) { return IsDead(); }
	bool ScriptIsPaused(void) { return IsPaused(); }

	// this wrapper function is needed so we can translate a Lua script object to something C++ can use.
	void ScriptAttachChild(LuaPlus::LuaObject child);

	// don't allow construction outside of this class
	explicit ScriptProcess(void);
};