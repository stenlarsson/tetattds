#pragma once

#include <lobby.h>
#include "localconnection.h"
#include "messagebuffer.h"

struct ConnectionInfo;

class LocalConnectionManager
{
public:
	LocalConnectionManager(int maxConnections, MessageReciever* reciever);
	~LocalConnectionManager();

	bool HostGame();
	LocalConnection* JoinGame();
	void JoinLoopback(MessageReciever* client);

	void Tick();
	
	void OnUserInfo(LPLOBBY_USER user, unsigned long reason);
	void OnPacket(unsigned char *data, int length, LPLOBBY_USER from);

private:
	LocalConnection* CreateConnection(LPLOBBY_USER user);
	void DeleteConnection(int index);
	LocalConnection* FindConnection(LPLOBBY_USER user);

	int maxConnections;
	int numConnections;
	ConnectionInfo* connections;
	MessageReciever* reciever;
	LPLOBBY_ROOM room;
	MessageBuffer buffer;
};

extern LocalConnectionManager* g_localConnectionManager;
