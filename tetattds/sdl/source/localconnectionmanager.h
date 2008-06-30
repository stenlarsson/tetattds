#pragma once

#include "connectionmanager.h"

struct ConnectionInfo;

class LocalConnectionManager : public ConnectionManager
{
public:
	LocalConnectionManager(int maxConnections, MessageReciever* reciever) {}
	~LocalConnectionManager() {}

	bool HostGame() { return false; }
	Connection* JoinGame() { return NULL; }

	virtual void Tick() {}

	virtual void BroadcastMessageImpl(
		unsigned char packetType,
		unsigned char messageId,
		const void* message,
		size_t length) {}

};
