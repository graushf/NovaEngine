#pragma once

#include "Common/CommonStd.h"

// ==============================================================
// Process.h : defines common game events
// ==============================================================

class Process;
typedef std::shared_ptr<Process> StrongProcessPtr;
typedef std::weak_ptr<Process> WeakProcessPtr;

// -------------------------------------------------------------------------------------------------------------
// Process class
//
// Processes are ended by one of three methods: Success, Failure, or Aborted.
//			- Success means the process completed succesfully. If the process has a child, it will
//			  be attached to the process manager.
//			- Failure means the process started but failed in some way. If the process has a child, it
//			  will be aborted.
//			- Aborted processes are processes that are canceled while not submitted to the process manager.
//			  Depending on the circumstances, they may or may not have gotten an OnInit() call. For example,
//			  a process can spawn another process and call AttachToParent() on itself. If the new
//			  process fails, the child will get an Abort() call on it, even though its status is
//			  RUNNING.
// -------------------------------------------------------------------------------------------------------------
class Process
{
	friend class ProcessManager;

public:
	enum State
	{
		// Processes that are neither dead nor alive
		UNITIALIZED = 0,	// created but not running
		REMOVED,			// removed from the process list but not destroyed; this can happen when a process that is already running is parented to another process
		
		// Living processes
		RUNNING,		// initialized and running
		PAUSED,			// initialized but paused

		// Dead processes
		SUCCEEDED,		// completed succesfully
		FAILED,			// failed to complete
		ABORTED,		// aborted; may not have started
	};

private:
	State m_state;
	StrongProcessPtr m_pChild;

public:
	// construction
	Process(void);
	virtual ~Process(void);

protected:
	// interface; these functions should be overriden by the subclass as needed.
	virtual void VOnInit(void) { m_state = RUNNING; }	// called during the first update; responsible for setting the inital state (typically RUNNING)
	virtual void VOnUpdate(unsigned long deltaMs) = 0;	// called every frame
	virtual void VOnSuccess(void) { }					// called if the process succeeds (see below)
	virtual void VOnFail(void) { }						// called if the process fails (see below)
	virtual void VOnAbort(void) { }						// called if the process is aborted (see below)

public:
	// Functions for ending the process.
	inline void Succeed(void);
	inline void Fail(void);

	// pause
	inline void Pause(void);
	inline void UnPause(void);

	// accessors
	State GetState(void) const { return m_state; }
	bool IsAlive(void) const { return (m_state == RUNNING || m_state == PAUSED); }
	bool IsDead(void) const { return (m_state == SUCCEEDED || m_state == FAILED || m_state == ABORTED); }
	bool IsRemoved(void) const { return (m_state == REMOVED); }
	bool IsPaused(void) const { return m_state == PAUSED; }

	// child functions
	inline void AttachChild(StrongProcessPtr pChild);
	StrongProcessPtr RemoveChild(void);				// releases ownership of the child
	StrongProcessPtr PeekChild(void) { return m_pChild; } // doesn't release ownership of the child

private:
	void SetState(State newState) { m_state = newState; }

};

// -----------------------------------------------------------------------------------------
// Inline function definitions
// -----------------------------------------------------------------------------------------
inline void Process::Succeed(void)
{
	//Nv_ASSERT(m_state == RUNNING || m_state == PAUSED);
	m_state = SUCCEEDED;
}

inline void Process::Fail(void)
{
	//Nv_ASSERT(m_state == RUNNING || m_state == PAUSED);
	m_state = FAILED;
}

inline void Process::AttachChild(StrongProcessPtr pChild)
{
	if (m_pChild)
		m_pChild->AttachChild(pChild);
	else
		m_pChild = pChild;
}

inline void Process::Pause(void)
{
	if (m_state == RUNNING) {
		m_state = PAUSED;
	} else {
		//Nv_WARNING("Attempting to pause a process that isn't running");
	}
}

inline void Process::UnPause(void)
{
	if (m_state == PAUSED) {
		m_state = RUNNING;
	}
	else {
		//Nv_WARNING("Attempting to unpause a process that isn't paused");
	}
}

/*
inline StrongProcessPtr Process::GetTopLevelProcess(void)
{
	if (m_pParent)
		return m_pParent->GetTopLevelProcess();
	else
		return this;
}
*/