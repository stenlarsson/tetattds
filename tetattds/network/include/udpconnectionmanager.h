#pragma once

#include "udpconnection.h"
#include "ipaddress.h"
#include "udpsocket.h"

struct ConnectionInfo;

class UdpConnectionManager
{
public:
	UdpConnectionManager(int maxConnections, UdpSocket* socket, MessageReciever* reciever);
	~UdpConnectionManager();

	UdpConnection* CreateConnection(const char* address, int port);

	void Tick();

private:
	UdpConnection* CreateConnection(IpAddress address);
	void DeleteConnection(int index);
	UdpConnection* FindConnection(IpAddress address);

	int maxConnections;
	int numConnections;
	ConnectionInfo* connections;
	UdpSocket* socket;
	MessageReciever* reciever;
};
