#pragma once

#include "connection.h"
#include "ipaddress.h"
#include "udpsocket.h"

struct ConnectionInfo;

class ConnectionManager
{
public:
	ConnectionManager(int maxConnections, UdpSocket* socket, MessageReciever* reciever);
	~ConnectionManager();

	Connection* CreateConnection(const char* address, int port);

	void Tick();

private:
	Connection* CreateConnection(IpAddress address);
	void DeleteConnection(int index);
	Connection* FindConnection(IpAddress address);

	int maxConnections;
	int numConnections;
	ConnectionInfo* connections;
	UdpSocket* socket;
	MessageReciever* reciever;
};
