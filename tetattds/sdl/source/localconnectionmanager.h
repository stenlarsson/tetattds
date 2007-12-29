#pragma once

#include "connection.h"

struct ConnectionInfo;

class LocalConnectionManager
{
public:
	LocalConnectionManager(int maxConnections, MessageReciever* reciever) {}
	~LocalConnectionManager() {}

	bool HostGame() { return false; }
	Connection* JoinGame() { return NULL; }

	void Tick() {}
};
