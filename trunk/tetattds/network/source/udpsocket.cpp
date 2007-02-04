#include "network.h"
#include "udpsocket-internal.h"
#include "udpsocket.h"
#include <stdio.h>
#include "ipaddress.h"

UdpSocket::UdpSocket()
:	internal(new UdpSocketInternal)
{
	INTERNAL->socket = ::socket(PF_INET, SOCK_DGRAM, 0);
	if(INTERNAL->socket == -1) { NET_ERROR("socket"); }

	unsigned long i = -1;
	int result = ::ioctl(INTERNAL->socket, FIONBIO, &i);
	if(result == -1) { NET_ERROR("ioctl"); }
}

UdpSocket::~UdpSocket()
{
	int result = ::close(INTERNAL->socket);
	if( result == -1 ) { NET_ERROR("close"); }

	delete INTERNAL;
}

bool UdpSocket::GetAddress(const char* address, int port, IpAddress* ipAddress)
{
	ASSERT(address != NULL);
	ASSERT(ipAddress != NULL);
	
	struct hostent* hostEntry = ::gethostbyname(address);
	if( hostEntry == NULL ) { NET_ERROR("gethostbyname"); return false; }

	if(*hostEntry->h_addr_list == NULL) {
		printf("Could not resolve address\n");
		return false;
	}

	*ipAddress = IpAddress(
		*(unsigned int*)hostEntry->h_addr_list[0],
		port);
	return true;
}

bool UdpSocket::Bind(int port)
{
	struct sockaddr_in sain;
	sain.sin_family = AF_INET;
	sain.sin_port = htons(port);
	sain.sin_addr.s_addr = INADDR_ANY;
	int result = ::bind(INTERNAL->socket, (struct sockaddr *)&sain, sizeof(sain));
	if(result == -1) { NET_ERROR("bind"); return false; }

	return true;
}

int UdpSocket::Send(const void *data, size_t size, IpAddress address)
{
	ASSERT(data != NULL);
	
	struct sockaddr_in sain;
	sain.sin_family = AF_INET;
	sain.sin_port = htons(address.port);
	sain.sin_addr.s_addr = address.address;

	int count = sendto(
		INTERNAL->socket,
		(char*)data,
		size,
		0,
		(struct sockaddr*)&sain,
		sizeof(sain));
	if(count == -1) { NET_ERROR("sendto"); }

	return count;
}

int UdpSocket::Recieve(void *data, size_t size, IpAddress* address)
{
	ASSERT(data != NULL);
	
	struct sockaddr_in sain;
	socklen_t sainlen = sizeof(sain);

	int count = recvfrom(
		INTERNAL->socket,
		(char*)data,
		size,
		0,
		(struct sockaddr *)&sain,
		&sainlen);
	if(count == -1) {
#ifdef WIN32
		if(WSAGetLastError() == WSAEWOULDBLOCK)
#else
		if(errno == EWOULDBLOCK)
#endif
		{
			return -2;
		}

		NET_ERROR("recvfrom");
		return -1;
	}

	*address = IpAddress(sain.sin_addr.s_addr, ntohs(sain.sin_port));
	return count;
}

bool UdpSocket::Wait(int sec, int usec)
{
	fd_set fdSet;
	timeval t;
	t.tv_sec = sec;
	t.tv_usec = usec;

	FD_ZERO(&fdSet);
	FD_SET(INTERNAL->socket, &fdSet);

	if(select(INTERNAL->socket + 1, &fdSet, NULL, NULL, &t ) == -1) { NET_ERROR("select() failed"); }
	
	return true;
}

bool UdpSocket::InitSockets()
{
#ifdef WIN32
	WORD sockVersion;
	WSADATA wsaData;

	// Initialize WinSock
	sockVersion = MAKEWORD( 2, 0 );
	int res = WSAStartup( sockVersion, &wsaData );
	if( res != 0 ) {
		printf("WSAStartup() failed\n");
		return false;
	}
#endif
	return true;
}
