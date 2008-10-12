#pragma once

#include "connectionmanager.h"
#include "udpconnection.h"
#include "ipaddress.h"
#include "udpsocket.h"

struct ConnectionInfo;

class UdpConnectionManager : public ConnectionManager
{
public:
	UdpConnectionManager(int maxConnections, UdpSocket* socket, MessageReciever* reciever);
	virtual ~UdpConnectionManager();

	UdpConnection* CreateConnection(const char* address, int port);

	virtual void Tick();

	virtual void BroadcastMessageImpl(
		unsigned char packetType,
		unsigned char messageId,
		const void* message,
		size_t length);

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
