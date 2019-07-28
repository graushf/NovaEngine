// ================================================================
// RealtimeProcess.cpp : Defines a process that can run in a thread
// ================================================================

#include "../Common/CommonStd.h"

#include "RealtimeProcess.h"
#include "../EventManager/EventManager.h"

DWORD g_maxLoops = 100000;
DWORD g_ProtectedTotal = 0;
CRITICAL_SECTION g_criticalSection;

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	DWORD maxLoops = *static_cast<DWORD*>(lpParam);
	DWORD dwCount = 0;
	
	while (dwCount < maxLoops)
	{
		++dwCount;

		EnterCriticalSection(&g_criticalSection);
		++g_ProtectedTotal;
		LeaveCriticalSection(&g_criticalSection);
	}
	return TRUE;
}

void CreateThreads()
{
	InitializeCriticalSection(&g_criticalSection);

	for (int i = 0; i < 20; i++)
	{
		HANDLE m_hThread = CreateThread(
										NULL,				// default security attributes
										0,					// default stack size
										(LPTHREAD_START_ROUTINE)ThreadProc,
										&g_maxLoops,		// thrad parameter is how many loops
										0,					// default creation flags
										NULL);				// receive thread identifier
	}
}

RealtimeProcess::RealtimeProcess(int priority)
{
	m_ThreadID = 0;
	m_ThreadPriority = priority;
}

DWORD WINAPI RealtimeProcess::ThreadProc(LPVOID lpParam)
{
	RealtimeProcess* proc = static_cast<RealtimeProcess*>(lpParam);
	proc->VThreadProc();
	return TRUE;
}

// -----------------------------------------------------------------------------------
// RealtimeProcess::VOnInitialize							- Chapter 18, page 667
// -----------------------------------------------------------------------------------
void RealtimeProcess::VOnInit(void)
{
	Process::VOnInit();
	m_hThread = CreateThread(
							NULL,				// default security attributes
							0,					// default stack size
							ThreadProc,			// thread process
							this,				// thread parameter is a pointer to the process
							0,					// default creation flags
							&m_ThreadID);		// receive thread identifier

	if (m_hThread == NULL)
	{
		//Nv_ERROR("Could not create thread!");
		Fail();
		return;
	}

	SetThreadPriority(m_hThread, m_ThreadPriority);
}

class ProtectedProcess : public RealtimeProcess
{
public:

	static DWORD g_ProtectedTotal;
	static CriticalSection g_criticalSection;
	DWORD m_MaxLoops;
	ProtectedProcess(DWORD maxLoops) : RealtimeProcess() { m_MaxLoops = maxLoops; }

	virtual void VThreadProc(void);
};

DWORD ProtectedProcess::g_ProtectedTotal = 0;
CriticalSection ProtectedProcess::g_criticalSection;

void ProtectedProcess::VThreadProc()
{
	DWORD dwCount = 0;

	while (dwCount < m_MaxLoops)
	{
		++dwCount;

		{
			// Request ownership of critical section.
			ScopedCriticalSection locker(g_criticalSection);
			++g_ProtectedTotal;
		}
	}

	Succeed();
}


class UnprotectedProcess : public RealtimeProcess
{
public: 
	static DWORD g_UnprotectedTotal;
	DWORD m_MaxLoops;
	UnprotectedProcess(DWORD maxLoops) : RealtimeProcess() { m_MaxLoops = maxLoops; }
	virtual void VThreadProc();
};

DWORD UnprotectedProcess::g_UnprotectedTotal = 0;

void UnprotectedProcess::VThreadProc(void)
{
	DWORD dwCount = 0;
	while (dwCount < m_MaxLoops)
	{
		++dwCount;
		++g_UnprotectedTotal;
	}
	Succeed();
}


// --------------------------------------------------------------------------------------

int g_ThreadCount = 50;
DWORD g_ThreadLoops = 100000;

void testThreading(ProcessManager* procMgr, bool runProtected)
{
			std::shared_ptr<Process> proc(Nv_NEW UnprotectedProcess(g_ThreadLoops));
			procMgr->AttachProcess(proc);

			return;

	int i;

	//InitializeCriticalSection(&ProtectedProcess::g_criticalSection);
	// Create worker threads

	for (i = 0; i < g_ThreadCount; i++)
	{
		if (runProtected)
		{
			std::shared_ptr<Process> proc(Nv_NEW ProtectedProcess(g_ThreadLoops));
			procMgr->AttachProcess(proc);
		}
		else
		{
			std::shared_ptr<Process> proc(Nv_NEW UnprotectedProcess(g_ThreadLoops));
			procMgr->AttachProcess(proc);
		}
	}
}

// Solution 1
// (note: compiles with gcc 3.2, VC6)
#include "CriticalSection.h"
//#include "SafeScream.h"

// --------------------------------------------------------------------------------------


// /////////////////////////////////////////////////////////////
// Test

#include <stdio.h>
//#include <iostream>
//#include <fstream>
//#include <iomanip>

const int THREADS_COUNT = 20;
const int WRITES_PER_THREAD = 500;

LONG nRemainingThreads = THREADS_COUNT;

#include "../EventManager/Events.h"

class EventSenderProcess : public RealtimeProcess
{
public:
	DWORD m_MaxLoops;
	EventSenderProcess(DWORD maxLoops);
	virtual void VThreadProc();
};

EventSenderProcess::EventSenderProcess(DWORD maxLoops)
	: RealtimeProcess()
{
	m_MaxLoops = maxLoops;
}

void EventSenderProcess::VThreadProc(void)
{
	DWORD dwCount = 0;

	while (dwCount < m_MaxLoops)
	{
		IEventDataPtr e(Nv_NEW EvtData_Update_Tick(timeGetTime()));
		IEventManager::Get()->VThreadSafeQueueEvent(e);
		Sleep(10);
		dwCount++;
	}

	Succeed();
	InterlockedDecrement(&nRemainingThreads);
}


class EventReaderProcess : public RealtimeProcess
{
public:
	EventReaderProcess() : RealtimeProcess()
	{
		IEventManager::Get()->VAddListener(fastdelegate::MakeDelegate(this, &EventReaderProcess::UpdateTickDelegate), EvtData_Update_Tick::sk_EventType);
		m_EventsRead = 0;
	}
	void UpdateTickDelegate(IEventDataPtr pEventData);
	virtual void VThreadProc(void);

	ThreadSafeEventQueue m_RealtimeEventQueue;
	int m_EventsRead;
};

void EventReaderProcess::UpdateTickDelegate(IEventDataPtr pEventData)
{
	// Need to do something here.
	m_RealtimeEventQueue.push(pEventData);
}

void EventReaderProcess::VThreadProc(void)
{
	// wait for all threads to end
	while (true)
	{
		IEventDataPtr e;
		if (m_RealtimeEventQueue.try_pop(e)) {
			++m_EventsRead;
		}
		else {
			InterlockedIncrement(&nRemainingThreads);
			if (InterlockedDecrement(&nRemainingThreads) == 0) {
				break;
			}
		}
	}

	Succeed();
}


void testRealtimeEvents(ProcessManager* procMgr)
{
	for (int idx = 0; idx < THREADS_COUNT; ++idx)
	{
		std::shared_ptr<Process> proc(Nv_NEW EventSenderProcess(g_ThreadLoops));
		procMgr->AttachProcess(proc);
	}
	std::shared_ptr<Process> proc(Nv_NEW EventReaderProcess());
	procMgr->AttachProcess(proc);
}

//
// class DecompressionProcess					- Chapter 18, page 674
//
class DecompressionProcess : public RealtimeProcess
{
protected:

public:
	static void Callback(int progress, bool& cancel);

	DecompressionProcess();
	~DecompressionProcess();

	ThreadSafeEventQueue m_RealtimeEventQueue;

	//event delegates
	void DecompressRequestDelegate(IEventDataPtr pEventData)
	{
		IEventDataPtr pEventClone = pEventData->VCopy();
		m_RealtimeEventQueue.push(pEventClone);
	}

	virtual void VThreadProc(void);
};

DecompressionProcess::DecompressionProcess()
	: RealtimeProcess()
{
	IEventManager::Get()->VAddListener(fastdelegate::MakeDelegate(this, &DecompressionProcess::DecompressRequestDelegate), EvtData_Decompress_Request::sk_EventType);
}

DecompressionProcess::~DecompressionProcess()
{
	IEventManager::Get()->VRemoveListener(fastdelegate::MakeDelegate(this, &DecompressionProcess::DecompressRequestDelegate), EvtData_Decompress_Request::sk_EventType);
}

void DecompressionProcess::VThreadProc()
{
	while (1)
	{
		// check the queue for events we should consume
		IEventDataPtr e;
		if (m_RealtimeEventQueue.try_pop(e))
		{
			// there's an event! Something to do...
			if (EvtData_Decompress_Request::sk_EventType == e->VGetEventType())
			{
				std::shared_ptr<EvtData_Decompress_Request> decomp = static_pointer_cast<EvtData_Decompress_Request>(e);

				ZipFile zipFile;

				bool success = FALSE;

				if (zipFile.Init(decomp->GetZipFilename().c_str()))
				{
					int size = 0;
					int resourceNum = zipFile.Find(decomp->GetFilename().c_str());
					if (resourceNum >= 0)
					{
						// size = zipFile.GetFileLen(*resourceNum);
						char* buffer = Nv_NEW char[size];
						zipFile.ReadFile(resourceNum, buffer);

						// send decompression result event
//						threadSafeQueueEvent(IEventDataPtr( Nv_NEW EvtData_Decompression_Progress (100, decomp->m_zipFileName, decomp->m_fileName, buffer) ) );						
					}
				}
			}
		}
		else
		{
			Sleep(10);
		}
	}

	Succeed();
}

void testRealtimeDecompression(ProcessManager* procMgr)
{
	static void* buffer = NULL;
	SAFE_DELETE(buffer);

	std::shared_ptr<Process> proc(Nv_NEW DecompressionProcess());
	procMgr->AttachProcess(proc);

	std::shared_ptr<EvtData_Decompress_Request> pEventData(Nv_NEW EvtData_Decompress_Request(L"big.zip", "big.dat"));
	IEventManager::Get()->VQueueEvent(pEventData);
}