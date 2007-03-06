#pragma once

#include <sys/types.h>

class IpAddress;

class UdpSocket
{
public:
	UdpSocket();
	~UdpSocket();

	bool GetAddress(const char* address, int port, IpAddress* ipAddress);

	bool Bind(int port);
	int Send(const void* data, size_t size, IpAddress address);
	int Recieve(void* data, size_t size, IpAddress* address);
	bool Wait(int sec, int usec);
	static bool InitSockets();

private:
	void* internal;
};
