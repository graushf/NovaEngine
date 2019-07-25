// ================================================================
// Network.cpp : Implementation of the Network classes
// ================================================================

#include "../Common/CommonStd.h"
#include "../App/App.h"

#include <stdio.h>
#include <errno.h>
#include "Network.h"
#include "../EventManager/Events.h"
#include "../EventManager/EventManagerImpl.h"
#include "../Utilities/String.h"

#pragma comment(lib, "Ws2_32")

//#define EXIT_ASSERT Nv_ASSERT(0);

const char* BinaryPacket::g_Type = "BinaryPacket";
const char* TextPacket::g_Type = "TextPacket";

//BaseSocketManager* g_pSocketManager = NULL;

/*******************************************************

void testCode()
{
	// Use of utility and conversion functions.
	unsigned long ipAddress = inet_addr("128.64.16.2");

	struct in_addr addr;
	addr.S_un.S_addr = htonl(0x88482818);

	char ipAddressString[16];
	strcpy(ipAddressString, inet_ntoa(addr));

	char buffer[256];
	sprintf(buffer, "0x%08x 0x%08x %s\n:", ipAddress, addr.S_un.S_addr, ipAddressString);
	OutputDebugStringA(buffer);

	// Use of DNS functions
	struct hostent* pHostEnt;
	const char* host = "ftp.microsoft.com";
	pHostEnt = gethostbyname(host);

	if (pHostEnt == NULL)
	{
		fprintf(stderr, "No such host");
	}
	else
	{
		struct sockaddr_in addr;
		memcpy(&addr.sin_addr, pHostEnt->h_addr, pHostEnt->h_length);

		char buffer[256];
		sprintf(buffer, "Address of %s is 0x%08x\n", host, ntohl(addr.sin_addr.s_addr));
		OutputDebugString(buffer);
	}

	unsigned int netip = inet_addr("207.46.133.140");
	pHostEnt = gethostbyaddr((const char*)&netip, 4, PF_INET);
}

********************************************************/

//
// TextPacket::TextPacket						- not described in the book
//
TextPacket::TextPacket(char const* const text)
	: BinaryPacket(static_cast<u_long>(strlen(text) + 2))
{
	MemCpy(text, strlen(text), 0);
	MemCpy("\r\n", 2, 2);
	*(u_long*)m_Data = 0;
}

// ------------------------------------------------------------------------
// NetSocket Implementation

//
// NetSocket::NetSocket							- Chapter 19, page 668
// 
NetSocket::NetSocket()
{
	m_sock = INVALID_SOCKET;
	m_deleteFlag = 0;
	m_sendOfs = 0;
	m_timeOut = 0;

	m_recvOfs = m_recvBegin = 0;
	m_internal = 0;
	m_bBinaryProtocol = 1;
}

//
// NetSocket::NetSocket							- Chapter 19, page 668
//
NetSocket::NetSocket(SOCKET new_sock, unsigned int hostIP)
{
	m_deleteFlag = 0;
	m_sendOfs = 0;
	m_timeOut = 0;

	m_bBinaryProtocol = 1;

	m_recvOfs = m_recvBegin = 0;
	m_internal = 0;

	m_timeCreated = timeGetTime();

	m_sock = new_sock;
	m_ipaddr = hostIP;

	m_internal = g_pSocketManager->IsInternal(m_ipaddr);

	setsockopt(m_sock, SOL_SOCKET, SO_DONTLINGER, NULL, 0);

	/***
	// Here's how to find the host address of the connection. It is very slow, however.
	if (m_ipaddr)
	{
		TCHAR buffer[128];
		const char* ansiIpaddress = g_pSocketManager->GetHostByAddr(m_ipaddr);
		if (ansiIpaddress)
		{
			TCHAR getIpaddress[64];
			AnsiToGenericCch(
		}
	}
	
	***/
}

//
// NetSocket::~NetSocket						- Chapter 19, page 668
//
NetSocket::~NetSocket()
{
	if (m_sock != INVALID_SOCKET)
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
}

//
// NetSocket::Connect							- Chapter 19, page 669
//
bool NetSocket::Connect(unsigned int ip, unsigned int port, bool forceCoalesce)
{
	struct sockaddr_in sa;
	int x = 1;

	if ((m_sock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		return false;
	}

	if (!forceCoalesce)
	{
		setsockopt(m_sock, IPPROTO_TCP, TCP_NODELAY, (char*)&x, sizeof(x));
	}

	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(ip);
	sa.sin_port = htons(port);

	if (connect(m_sock, (struct sockaddr*)&sa, sizeof(sa)))
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		return false;
	}

	return true;
}

//
// NetSocket::Send								- Chapter 19, page 670
//
void NetSocket::Send(std::shared_ptr<IPacket> pkt, bool clearTimeOut)
{
	if (clearTimeOut) {
		m_timeOut = 0;
	}
	m_OutList.push_back(pkt);
}

//
// NetSocket::SetBlocking						- Chapter 19, page 670
//
void NetSocket::SetBlocking(bool blocking)
{
	#ifdef WIN32
		unsigned long val = blocking ? 0 : 1;
		ioctlsocket(m_sock, FIONBIO, &val);
	#else
		int val = fcntl(m_sock, F_GETFL, 0);
		if (blocking) {
			val &= ~(O_NONBLOCK);
		}
		else {
			val |= O_NONBLOCK;
		}

		fcntl(m_sock, F_SETFL, val);
	#endif
}

//
// NetSocket::VHandleOutput						- Chapter 19, page 670
//
void NetSocket::VHandleOutput()
{
	int fSent = 0;
	do
	{
		//Nv_ASSERT(!m_OutList.empty());
		PacketList::iterator i = m_OutList.begin();

		std::shared_ptr<IPacket> pkt = *i;
		const char* buf = pkt->VGetData();
		int len = static_cast<int>(pkt->VGetSize());

		int rc = send(m_sock, buf + m_sendOfs, len - m_sendOfs, 0);
		if (rc > 0)
		{
			g_pSocketManager->AddToOutbound(rc);
			m_sendOfs += rc;
			fSent = 1;
		}
		else if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			HandleException();
			fSent = 0;
		}
		else
		{
			fSent = 0;
		}

		if (m_sendOfs == pkt->VGetSize())
		{
			m_OutList.pop_front();
			m_sendOfs = 0;
		}
	} while (fSent && !m_OutList.empty());
}

//
// NetSocket::VHandleInput						- Chapter 19. page 671 
//
void NetSocket::VHandleInput()
{
	bool bPktReceived = false;
	u_long packetSize = 0;
	int rc = recv(m_sock, m_recvBuf + m_recvBegin + m_recvOfs, RECV_BUFFER_SIZE - (m_recvBegin + m_recvOfs), 0);

	char metrics[1024];
	sprintf_s(metrics, 1024, "Incoming: %6d bytes. Begin %6d Offset %4d\n", rc, m_recvBegin, m_recvOfs);
	//Nv_LOG("Network", metrics);

	if (rc == 0)
	{
		return;
	}

	if (rc == SOCKET_ERROR)
	{
		m_deleteFlag = 1;
		return;
	}

	const int hdrSize = sizeof(u_long);
	unsigned int newData = m_recvOfs + rc;
	int processedData = 0;

	while (newData > hdrSize)
	{
		// There are two types of packets at the lowest level of our design:
		// BinaryPacket - Sends the size as a positive 4 byte integer.
		// TextPacket - Sends 0 for the size, the parser will search for a CR

		packetSize = *(reinterpret_cast<u_long*>(m_recvBuf + m_recvBegin));
		packetSize = ntohl(packetSize);

		if (m_bBinaryProtocol)
		{
			// we don't have enough new data to grab the next packet
			if (newData < packetSize) {
				break;
			}
			
			if (packetSize > MAX_PACKET_SIZE)
			{
				// prevent nasty buffer overruns!
				HandleException();
				return;
			}

			if (newData >= packetSize)
			{
				// we know how big the packet is... and we have the whole thing
				//
				// [mrmike] - a little code to aid debugging network packets here!
				//char test[1024];
				//memcpy(test, &m_recvBuf[m_recvBegin+hdrSize], packetSize);
				//test[packetSize+1] = '\r';
				//test[packetSize+2] = '\n';
				//test[packetSize+3] = 0;
				//Nv_LOG("Network", test);
				std::shared_ptr<BinaryPacket> pkt(Nv_NEW BinaryPacket(&m_recvBuf[m_recvBegin + hdrSize], packetSize - hdrSize));
				m_InList.push_back(pkt);
				bPktReceived = true;
				processedData += packetSize;
				newData -= packetSize;
				m_recvBegin += packetSize;
			}
		}
		else
		{
			// the text protocol waits for a carriage return and creates a string
			char* cr = static_cast<char*>(memchr(&m_recvBuf[m_recvBegin], 0x0a, rc));
			if (cr)
			{
				*(cr + 1) = 0;
				std::shared_ptr<TextPacket> pkt(Nv_NEW TextPacket(&m_recvBuf[m_recvBegin]));
				m_InList.push_back(pkt);
				packetSize = cr - &m_recvBuf[m_recvBegin];
				bPktReceived = true;

				processedData += packetSize;
				newData -= packetSize;
				m_recvBegin += packetSize;
			}
		}
	}

	g_pSocketManager->AddToInbound(rc);
	m_recvOfs = newData;

	if (bPktReceived)
	{
		if (m_recvOfs == 0)
		{
			m_recvBegin = 0;
		}
		else if (m_recvBegin + m_recvOfs + MAX_PACKET_SIZE > RECV_BUFFER_SIZE)
		{
			// we don't want to overrun the buffer - so we copy the leftover bits
			// to the beginning of the received buffer and start over
			int leftover = m_recvOfs;
			memcpy(m_recvBuf, &m_recvBuf[m_recvBegin], m_recvOfs);
			m_recvBegin = 0;
		}
	}
}

// ----------------------------------------------------------------------------
// NetListenSocket::NetListenSocket						- Chapter 19, page 674
//
// ----------------------------------------------------------------------------
NetListenSocket::NetListenSocket(int portnum)
{
	port = 0;
	Init(portnum);
}

//
// NetListenSocket::Init							- Chapter 19, page 674
//
void NetListenSocket::Init(int portnum)
{
	struct sockaddr_in sa;
	int value = 1;

	if ((m_sock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		//Nv_ASSERT("NetListenSocket Error: Init failed to create a socket handle");
	}

	if (setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&value, sizeof(value)) == SOCKET_ERROR)
	{
		perror("NetListenSocket::Init: setsockopt");
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		//Nv_ASSERT("NetListenSocket Error: Init failed to set socket options");
	}

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = ADDR_ANY;
	sa.sin_port = htons(portnum);

	// bind to port
	if (bind(m_sock, (struct sockaddr*)&sa, sizeof(sa)) == SOCKET_ERROR)
	{
		perror("NetListenSocket::Init: bind");
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		//Nv_ASSERT("NetListenSocket Error: Init failed to bind");
	}

	// set nonblocking - accept() blocks under some odd circumstances otherwise
	SetBlocking(false);

	// start listening
	if (listen(m_sock, 256) == SOCKET_ERROR)
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		//Nv_ASSERT("NetListenSocket Error: Init failed to listen");
	}

	port = portnum;
}

//
// NetListenSocket::InitScan					- Chapter X, page Y
//		Opens multiple ports to listen for connections.
//
void NetListenSocket::InitScan(int portnum_min, int portnum_max)
{
	struct sockaddr_in sa;
	int portnum, x = 1;

	if ((m_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		//EXIT_ASSERT
		exit(1);
	}

	if (setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&x, sizeof(x)) == SOCKET_ERROR)
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		//EXIT_ASSERT
		exit(1);
	}

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	for (portnum = portnum_min; portnum < portnum_max; portnum++)
	{
		sa.sin_port = htons(portnum);
		// bind to port
		if (bind(m_sock, (struct sockaddr*)&sa, sizeof(sa)) != SOCKET_ERROR) {
			break;
		}
	}

	if (portnum == portnum_max)
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		//EXIT_ASSERT
		exit(1);
	}

	// set nonblocking - accept() blocks under some odd circumstances otherwise
	SetBlocking(false);

	// start listening
	if (listen(m_sock, 8) == SOCKET_ERROR)
	{
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
		//EXIT_ASSERT
		exit(1);
	}
}