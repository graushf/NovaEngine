//========================================================================
// EventManager.cpp : 
//========================================================================

#include "Common/CommonStd.h"
#include "EventManager.h"

static IEventManager* g_pEventManager = nullptr;
GenericObjectFactory<IEventData, EventType> g_eventFactory;

IEventManager* IEventManager::Get(void)
{
	//Nv_ASSERT(g_pEventManager);
	return g_pEventManager;
}

IEventManager::IEventManager(const char* pName, bool setAsGlobal)
{
	if (setAsGlobal)
	{
		if (g_pEventManager)
		{
			//Nv_ERROR("Attempting to create two global event managers! The old one will be destroyed and overwritten with this one.");
			delete g_pEventManager;
		}

		g_pEventManager = this;
	}
}

IEventManager::~IEventManager(void)
{
	if (g_pEventManager == this)
		g_pEventManager = nullptr;
}