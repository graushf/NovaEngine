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

	port = portnum;
}

//
// NetListenSocket::AcceptConnection							- Chapter 19, page 675
//
SOCKET NetListenSocket::AcceptConnection(unsigned int* pAddr)
{
	SOCKET new_sock;
	struct sockaddr_in sock;
	int size = sizeof(sock);

	if ((new_sock = accept(m_sock, (struct sockaddr*)&sock, &size)) == INVALID_SOCKET)
	{
		return INVALID_SOCKET;
	}

	if (getpeername(new_sock, (struct sockaddr*)&sock, &size) == SOCKET_ERROR)
	{
		closesocket(new_sock);
		return INVALID_SOCKET;
	}
	*pAddr = ntohl(sock.sin_addr.s_addr);
	return new_sock;
}

//
// BaseSocketManager::BaseSocketManager							- Chapter 19, page 677
//
BaseSocketManager::BaseSocketManager()
{
	m_Inbound = 0;
	m_Outbound = 0;
	m_MaxOpenSockets = 0;
	m_SubnetMask = 0;
	m_Subnet = 0xffffffff;
	m_NextSocketId = 0;

	g_pSocketManager = this;
	ZeroMemory(&m_WsaData, sizeof(WSADATA));
}

//
// BaseSocketManager:Init					- Chapter 19, page 677
//
bool BaseSocketManager::Init()
{
	if (WSAStartup(0x0202, &m_WsaData) == 0)
	{
		return true;
	}
	else
	{
		//Nv_ERROR("WSAStartup failure!");
		return false;
	}
}

//
// BaseSocketManager::Shutdown						- Chapter 19, page 678
//
void BaseSocketManager::Shutdown()
{
	// Get rid of all those pesky kids
	while (!m_SockList.empty())
	{
		delete *m_SockList.begin();
		m_SockList.pop_front();
	}

	WSACleanup();
}

//
// BaseSocketManager:AddSocket					- Chapter 19, page 677
//
int BaseSocketManager::AddSocket(NetSocket* socket)
{
	socket->m_id = m_NextSocketId;
	m_SockMap[m_NextSocketId] = socket;
	++m_NextSocketId;

	m_SockList.push_front(socket);
	if (m_SockList.size() > m_MaxOpenSockets)
	{
		++m_MaxOpenSockets;
	}

	return socket->m_id;
}

//
// BaseSocketManager::RemoveSocket					- not described in the book
//
// Removes a sock for the socket list - done once it is not connected anymore
//
void BaseSocketManager::RemoveSocket(NetSocket* socket)
{
	m_SockList.remove(socket);
	m_SockMap.erase(socket->m_id);
	SAFE_DELETE(socket);
}

//
// BaseSocketManager::FindSocket					- Chapter 19, page 679
//
NetSocket* BaseSocketManager::FindSocket(int sockId)
{
	SocketIdMap::iterator i = m_SockMap.find(sockId);
	if (i == m_SockMap.end())
	{
		return NULL;
	}
	return i->second;
}

int BaseSocketManager::GetIpAddress(int sockId)
{
	NetSocket* socket = FindSocket(sockId);
	if (socket)
	{
		return socket->GetIpAddress();
	}
	else
	{
		return 0;
	}
}

//
// BaseSocketManager::Send					- Chapter 19, page 679
//
bool BaseSocketManager::Send(int sockId, std::shared_ptr<IPacket> packet)
{
	NetSocket* sock = FindSocket(sockId);
	if (!sock) {
		return false;
	}
	sock->Send(packet);
	return true;
}

//
// BaseSocketManager::DoSelect				- Chapter 19, page 679
//
void BaseSocketManager::DoSelect(int pauseMicroSecs, bool handleInput)
{
	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = pauseMicroSecs;		// 100 microseconds is 0.1 milliseconds or .0001 seconds

	fd_set inp_set, out_set, exc_set;
	int maxdesc;

	FD_ZERO(&inp_set);
	FD_ZERO(&out_set);
	FD_ZERO(&exc_set);

	maxdesc = 0;

	// set everything up for the select
	for (SocketList::iterator i = m_SockList.begin(); i != m_SockList.end(); ++i)
	{
		NetSocket* pSock = *i;
		if ((pSock->m_deleteFlag & 1) || pSock->m_sock == INVALID_SOCKET)
		{
			continue;
		}

		if (handleInput) {
			FD_SET(pSock->m_sock, &inp_set);
		}

		FD_SET(pSock->m_sock, &exc_set);

		if (pSock->VHasOutput()) {
			FD_SET(pSock->m_sock, &out_set);
		}

		if ((int)pSock->m_sock > maxdesc) {
			maxdesc = (int)pSock->m_sock;
		}
	}

	int selRet = 0;

	// do the select (duration passed in as tv, NULL to block until event)
	selRet = select(maxdesc + 1, &inp_set, &out_set, &exc_set, &tv);
	if (selRet == SOCKET_ERROR)
	{
		PrintError();
		return;
	}

	// handle input, output and exceptions

	if (selRet)
	{
		for (SocketList::iterator i = m_SockList.begin(); i != m_SockList.end(); ++i)
		{
			NetSocket* pSock = *i;

			if ((pSock->m_deleteFlag & 1) || pSock->m_sock == INVALID_SOCKET)
			{
				continue;
			}

			if (FD_ISSET(pSock->m_sock, &exc_set))
			{
				pSock->HandleException();
			}

			if (!(pSock->m_deleteFlag & 1) && FD_ISSET(pSock->m_sock, &out_set))
			{
				pSock->VHandleOutput();
			}

			if (handleInput
				&& !(pSock->m_deleteFlag & 1) && FD_ISSET(pSock->m_sock, &inp_set))
			{
				pSock->VHandleInput();
			}
		}
	}

	unsigned int timeNow = timeGetTime();

	// handle deleting any sockets
	SocketList::iterator i = m_SockList.begin();
	while (i != m_SockList.end())
	{
		NetSocket* pSock = *i;
		if (pSock->m_timeOut)
		{
			if (pSock->m_timeOut < timeNow)
			{
				pSock->VTimeOut();
			}
		}

		if (pSock->m_deleteFlag & 1)
		{
			switch (pSock->m_deleteFlag)
			{
				case 1:
					g_pSocketManager->RemoveSocket(pSock);
					i = m_SockList.begin();
					break;
				case 3:
					pSock->m_deleteFlag = 2;
					if (pSock->m_sock != INVALID_SOCKET)
					{
						closesocket(pSock->m_sock);
						pSock->m_sock = INVALID_SOCKET;
					}
					break;
			}
		}

		++i;

	}
}

//
// BaseSocketManager::IsInternal				- Chapter 19, page 682
//
bool BaseSocketManager::IsInternal(unsigned int ipaddr)
{
	if (!m_SubnetMask) {
		return false;
	}

	if ((ipaddr & m_SubnetMask) == m_Subnet) {
		return false;
	}

	return true;
}

// 
//
//
unsigned int BaseSocketManager::GetHostByName(const std::string& hostName)
{
	// This will retrieve the ip details and put it into pHostEnt structure
	struct hostent* pHostEnt = gethostbyname(hostName.c_str());
	struct sockaddr_in tmpSockAddr;	// placeholder for the ip address

	if (pHostEnt == NULL) {
		//Nv_ASSERT(0 && _T("Error occurred"));
		return 0;
	}

	memcpy(&tmpSockAddr.sin_addr, pHostEnt->h_addr, pHostEnt->h_length);
	
	return ntohl(tmpSockAddr.sin_addr.s_addr);
}

//
//
//
const char* BaseSocketManager::GetHostByAddr(unsigned int ip)
{
	static char host[256];

	int netip = htonl(ip);
	struct hostent* lpHostEnt = gethostbyaddr((const char*)&netip, 4, PF_INET);

	if (lpHostEnt)
	{
		strcpy_s(host, 256, lpHostEnt->h_name);
		return host;
	}
	return NULL;
}

void BaseSocketManager::PrintError()
{
	int realError = WSAGetLastError();
	const char* reason;

	switch (realError)
	{
		case WSANOTINITIALISED: 
			reason = "A succesful WSAStartup must occur before using this API."; 
			break;
		case WSAEFAULT: 
			reason = "The Windows Sockets implementation was unable to allcoate needed resources for its internal operations, or the readfds, writefds, exceptfds, or timeval parameters are not part of the user address space."; 
			break;
		case WSAENETDOWN:
			reason = "The network subsystem has failed.";
			break;
		case WSAEINVAL:
			reason = "The timeout value is not valid, or all three descriptor parameters were NULL.";
			break;
		case WSAEINTR:
			reason = "The (blocking) call was canceled via WSACancelBlockingCall.";
			break;
		case WSAEINPROGRESS:
			reason = "A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.";
			break;
		case WSAENOTSOCK:
			reason = "One of the descriptor sets contains an entry which is not a socket.";
			break;
		default:
			reason = "Unknown.";
	}

	char buffer[256];
	sprintf(buffer, "SOCKET error: %s", reason);
	
	//Nv_LOG("Network", buffer);
}


//
// ClientSocketManager::Connect			- Chapter 19, page 684
//
bool ClientSocketManager::Connect()
{
	if (!BaseSocketManager::Init()) {
		return false;
	}

	RemoteEventSocket* pSocket = Nv_NEW RemoteEventSocket;
	
	if (!pSocket->Connect(GetHostByName(m_HostName), m_Port))
	{
		SAFE_DELETE(pSocket);
		return false;
	}
	AddSocket(pSocket);
	return true;
}

//
// GameServerListenSocket::VHandleInput				- Chapter 19, page 685
//
void GameServerListenSocket::VHandleInput()
{
	unsigned int theipaddr;
	SOCKET new_sock = AcceptConnection(&theipaddr);

	int value = 1;
	setsockopt(new_sock, SOL_SOCKET, SO_DONTLINGER, (char*)&value, sizeof(value));

	if (new_sock != INVALID_SOCKET)
	{
		RemoteEventSocket* sock = Nv_NEW RemoteEventSocket(new_sock, theipaddr);
		int sockId = g_pSocketManager->AddSocket(sock);
		int ipAddress = g_pSocketManager->GetIpAddress(sockId);
		std::shared_ptr<EvtData_Remote_Client> pEvent(Nv_NEW EvtData_Remote_Client(sockId, ipAddress));
		IEventManager::Get()->VQueueEvent(pEvent);
	}
}

//
// RemoteEventSocket::VHandleInput					- Chapter 19, page 688
//
void RemoteEventSocket::VHandleInput()
{
	NetSocket::VHandleInput();

	// traverse the list of m_InList packets and do something useful with them
	while (!m_InList.empty())
	{
		std::shared_ptr<IPacket> packet = *m_InList.begin();
		m_InList.pop_front();
		if (!strcmp(packet->VGetType(), BinaryPacket::g_Type))
		{
			const char* buf = packet->VGetData();
			int size = static_cast<int>(packet->VGetSize());

			std::istrstream in(buf + sizeof(u_long), (size - sizeof(u_long)));

			int type;
			in >> type;
			switch (type)
			{
				case NetMsg_Event:

				case NetMsg_PlayerLoginOk:
				{
					int serverSockId, actorId;
					in >> serverSockId;
					in >> actorId;
					in >> g_pApp->m_Options.m_Level;			// [mrmike] - This was the best spot to set the level!
					std::shared_ptr<EvtData_Network_Player_Actor_Assignment> pEvent(Nv_NEW EvtData_Network_Player_Actor_Assignment(actorId, serverSockId));
					IEventManager::Get()->VQueueEvent(pEvent);
					break;
				}

				default:
					//Nv_ERROR("Unknown message type.");

			}
		}
		else if (!strcmp(packet->VGetType(), TextPacket::g_Type))
		{
			//Nv_LOG("Network", packet->VGetData() + sizeof(u_long));
		}
	}
}

//
// RemoteEventSocket::CreateEvent						- Chapter 19, page 689
//
void RemoteEventSocket::CreateEvent(std::istrstream& in)
{
	// Note: We can improve the efficiency of this by comparing the hash values instead of strings.
	// But strings are easier to debug!
	EventType eventType;
	in >> eventType;

	IEventDataPtr pEvent(CREATE_EVENT(eventType));
	if (pEvent)
	{
		pEvent->VDeserialize(in);
		IEventManager::Get()->VQueueEvent(pEvent);
	}
	else 
	{
		//Nv_ERROR("ERROR Unknown event type from remote: 0x" + ToStr(eventType, 16));
	}
}