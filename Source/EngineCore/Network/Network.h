#pragma once

// ================================================================
// Network.h : The core classes for creating a multiplayer game
// ================================================================

#include <sys/types.h>
#include <Winsock2.h>
#include "../EventManager/EventManager.h"


#define MAX_PACKET_SIZE (256)
#define RECV_BUFFER_SIZE (MAX_PACKET_SIZE * 512)
#define MAX_QUEUE_PER_PLAYER (10000)

#define MAGIC_NUMBER		( 0x1f2e3d4c)
#define IPMANGLE(a,b,c,d) (((a)<<24)|((b)<<16)|((c)<<8)|((d)))
#define INVALID_SOCKET_ID (-1)

class NetSocket;

// ------------------------------------------------
//
// IPacket Description
//
//	The interface class that defines a public API
//	for packet objects - data that is either about
//  to be sent to or just been received from the
//  network.
//
// ------------------------------------------------

class IPacket
{
public:
	virtual char const* const VGetType() const = 0;
	virtual char const* const VGetData() const = 0;
	virtual u_long VGetSize() const = 0;
	virtual ~IPacket() { }
};

// ------------------------------------------------
//
// class BinaryPacket			-Chapter 19, page 665
//
// A packet object that can be constructed all at 
// once, or with repeated calls to MemCpy
//
// ------------------------------------------------

class BinaryPacket : public IPacket
{
protected:
	char* m_Data;

public:
	inline BinaryPacket(char const* const data, u_long size);
	inline BinaryPacket(u_long size);
	virtual ~BinaryPacket() { SAFE_DELETE(m_Data); }
	virtual char const * const VGetType() const { return g_Type; }
	virtual char const * const VGetData() const { return m_Data; }
	virtual u_long VGetSize() const { return ntohl(*(u_long *)m_Data); }
	inline void MemCpy(char const * const data, size_t size, int destOffset);

	static const char* g_Type;
};

// ----------------------------------------------------------
// BinaryPacket::BinaryPacket		  - Chapter 19, page 666
// ----------------------------------------------------------
inline BinaryPacket::BinaryPacket(char const* const data, u_long size)
{
	m_Data = Nv_NEW char[size + sizeof(u_long)];
	//Nv_ASSERT(m_Data);
	*(u_long*)m_Data = htonl(size + sizeof(u_long));
	memcpy(m_Data + sizeof(u_long), data, size);
}

inline BinaryPacket::BinaryPacket(u_long size)
{
	m_Data = Nv_NEW char[size + sizeof(u_long)];
	//Nv_ASSERT(m_Data);
	*(u_long*)m_Data = htonl(size + sizeof(u_long));
}

// ----------------------------------------------------------
// BinaryPacket::MemCpy		  - Chapter 19, page 666
// ----------------------------------------------------------
inline void BinaryPacket::MemCpy(char const* const data, size_t size, int destOffset)
{
	//Nv_ASSERT(size + destOffset <= VGetSize() - sizeof(u_long));
	memcpy(m_Data + destOffset + sizeof(u_long), data, size);
}