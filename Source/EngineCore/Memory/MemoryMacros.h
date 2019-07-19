#pragma once

// ================================================================
// MemoryMacros.h
// ================================================================

#include "../Common/CommonStd.h"
#include "MemoryPool.h"

// ===================================================================================================================
// These macros are designed to allow classes to easily take advantage of memory pools. To use, follow this steps:
// 1) Call Nv_MEMORYPOOL_DECLARATION() in the class declaration
// 2) Call Nv_MEMORYPOOL_DEFINITION() in the cpp file
// 3) Call Nv_MEMORYPOOL_AUTOINIT() or Nv_MEMORYPOOL_AUTOINIT_DEBUGNAME() in the cpp file.
//
// That's it! Objects of your class will now be allocated through the memory pool! You can see examples of its
// usage in Pathing.h and Pathing.cpp. Check out the PathingNode class.
// ===================================================================================================================


// ------------------------------------------------------------------------------------------------------------------
// This macro is placed inside the body of the class that you want to use a memory pool with. It declarates the
// overloaded new and delete operators as well as the static MemoryPool object.
//
// IMPORTANT: InitMemoryPool() and DestroyMemoryPool() must be called manually unless you use the Nv_MEMORYPOOL_AUTOINIT()
// macro below.
// ------------------------------------------------------------------------------------------------------------------
#define Nv_MEMORYPOOL_DECLARATION(__defaultNumChunks__) \
	public: \
		static MemoryPool* s_pMemoryPool; \
		static void InitMemoryPool(unsigned int numChunks = __defaultNumChunks__, const char* debugName = 0); \
		static void DestroyMemoryPool(void); \
		static void* operator new(size_t size); \
		static void operator delete(void* pPtr); \
		static void* operator new[](size_t size); \
		static void operator delete[](void* pPtr); \
	private: \

// ------------------------------------------------------------------------------------------------------------------
// This macro defines the definition for the overloaded new & delete operators on a class meant to be pooled with a
// memory pool. It is meant to work specifically with the MemoryPool class. To use it, call this macro from the cpp
// file where your class function definitions are.
//	- _className_:		the name of this class.
// ------------------------------------------------------------------------------------------------------------------
#define Nv_MEMORYPOOL_DEFINITION(_className_) \
	MemoryPool* _className_::s_pMemoryPool = NULL;\
	void _className_::InitMemoryPool(unsigned int numChunks, const char* debugName) \
	{ \
		if (s_pMemoryPool != NULL) \
		{ \
			/*Nv_ERROR("s_pMemoryPool is not NULL. All data will be destroyed. (Ignorable)" */ \
			SAFE_DELETE(s_pMemoryPool); \
		} \
		s_pMemoryPool = Nv_NEW MemoryPool; \
		if (debugName) \
		{ \
			s_pMemoryPool->SetDebugName(debugName); \
		} \
		else \
		{ \
			s_pMemoryPool->SetDebugName(#_className_); \
		} \
	} \
	void _className_::DestroyMemoryPool(void) \
	{\
		/* Nv_ASSERT(s_pMemoryPool != NULL); */ \
		SAFE_DELETE(s_pMemoryPool); \
	}\
	void* _className_::operator new(size_t size) \
	{ \
		/* Nv_ASSERT(s_pMemoryPool); */ \
		void* pMem = s_pMemoryPool->Alloc(); \
		return pMem; \
	} \
	void _className_::operator delete(void* pPtr) \
	{ \
		/* Nv_ASSERT(s_pMemoryPool);*/\
		s_pMemoryPool->Free(pPtr); \
	} \
	void* _className_::operator new[](size_t size) \
	{ \
		/* Nv_ASSERT(s_pMemoryPool);*/ \
		void* pMem = s_pMemoryPool->Alloc(); \
		return pMem; \
	} \
	void _className_::operator delete(void* pPtr) \
	{ \
		/* Nv_ASSERT(s_pMemoryPool); */\
		s_pMemoryPool->Free(pPtr); \
	} \

//---------------------------------------------------------------------------------------------------------------------
// This macro defines a static class that automatically initializes a memory pool at global startup and destroys it at
// global destruction time.  Using this gets around the requirement of manually initializing and destroying the memory
// pool yourself.
//---------------------------------------------------------------------------------------------------------------------
#define Nv_MEMORYPOOL_AUTOINIT_DEBUGNAME(_className_, _numChunks_, _debugName_) \
class _className_ ## _AutoInitializedMemoryPool \
{ \
public: \
	_className_ ## _AutoInitializedMemoryPool(void); \
	~_className_ ## _AutoInitializedMemoryPool(void); \
}; \
_className_ ## _AutoInitializedMemoryPool::_className_ ## _AutoInitializedMemoryPool(void) \
{ \
	_className_::InitMemoryPool(_numChunks_, _debugName_); \
} \
_className_ ## _AutoInitializedMemoryPool::~_className_ ## _AutoInitializedMemoryPool(void) \
{ \
	_className_::DestroyMemoryPool(); \
} \
static _className_ ## _AutoInitializedMemoryPool s_ ## _className_ ## _AutoInitializedMemoryPool; \

#define Nv_MEMORYPOOL_AUTOINIT(_className_, _numChunks_) Nv_MEMORYPOOL_AUTOINIT_DEBUGNAME(_className_, _numChunks_, #_className_)
