#include "tetattds.h"
#include "localconnectionmanager.h"
#include <nds.h>
#include "tetattds.h"
#include "ds.h"
#include <MessageQueue.h>
#include <802.11.h>
#include <lobby.h>

#define GAMECODE 13687

LocalConnectionManager* g_localConnectionManager = NULL;

struct ConnectionInfo
{
	LPLOBBY_USER user;
	LocalConnection* connection;
};

LocalConnectionManager::LocalConnectionManager(int maxConnections, MessageReciever* reciever)
:	maxConnections(maxConnections),
	numConnections(0),
	connections(new ConnectionInfo[maxConnections]),
	reciever(reciever),
	room(room)
{
	ASSERT(reciever != NULL);
	g_localConnectionManager = this;
}

LocalConnectionManager::~LocalConnectionManager()
{
	delete connections;
	g_localConnectionManager = NULL;
}

bool LocalConnectionManager::HostGame()
{
	room = LOBBY_GetRoomByGame(0, 69);
	if(room != NULL) {
		printf("a game is already hosted\n");
		return false;
	}
	
	LOBBY_CreateRoom("PONG", 2, 69, 2);
	room = LOBBY_GetRoomByID(ROOMID_MYROOM);
	return true;
}

LocalConnection* LocalConnectionManager::JoinGame()
{
	int i = 0;
	do {
		LOBBY_Update();
		room = LOBBY_GetRoomByGame(0, 69);
		swiWaitForVBlank();
	} while(room == NULL && i++ < 10*60);
	
	if(room == NULL) {
		printf("can't find game\n");
		return NULL;
	}
	
	LOBBY_JoinRoom(room);
	LPLOBBY_USER user = LOBBY_GetRoomUserBySlot(room, 0) ;
	
	return CreateConnection(user);
}

void LocalConnectionManager::Tick()
{
	for(int i = 0; i < numConnections; i++) {
		LocalConnection* connection = connections[i].connection;
		connection->Tick();
		ConnectionState state = connection->GetState();
		if(state != CONNECTIONSTATE_CONNECTED) {
			if(state == CONNECTIONSTATE_TIMED_OUT) {
				reciever->ConnectionTimeout(connection);
			} else if(state == CONNECTIONSTATE_REJECTED) {
				reciever->ConnectionRejected(connection);
			}
			if(state != CONNECTIONSTATE_CLOSING)
			{
				DeleteConnection(i);
				i--;
			}
		}
	}
}

LocalConnection* LocalConnectionManager::CreateConnection(LPLOBBY_USER user)
{
	ASSERT(numConnections != maxConnections);

	LocalConnection* connection = new LocalConnection(user);
	
	connections[numConnections].user = user;
	connections[numConnections].connection = connection;
	numConnections++;
	
	reciever->ConnectionCreated(connection);
	return connection;
}

void LocalConnectionManager::DeleteConnection(int index)
{
	ASSERT(index >= 0);
	ASSERT(index < numConnections);

	delete connections[index].connection;
	connections[index].connection = NULL;

	numConnections--;
	for(int i = index; i < numConnections; i++) {
		connections[i] = connections[i+1];
	}
}

LocalConnection* LocalConnectionManager::FindConnection(LPLOBBY_USER user)
{
	for(int i = 0; i < numConnections; i++) {
		if(connections[i].user == user) {
			return connections[i].connection;
		}
	}

	return NULL;
}

void LocalConnectionManager::OnUserInfo(LPLOBBY_USER user, unsigned long reason)
{
	Connection* connection = FindConnection(user);
	
	switch(reason)
	{
	case USERINFO_REASON_REGOGNIZE:
		//printf("user %s recognized\n", LOBBY_GetUserName(user));
		break;
	case USERINFO_REASON_TIMEOUT:
		//printf("user %s timeout\n", LOBBY_GetUserName(user));
		if(connection != NULL) {
			connection->SetState(CONNECTIONSTATE_TIMED_OUT);
		}
		break;
	case USERINFO_REASON_RETURN:
		//printf("user %s returned\n", LOBBY_GetUserName(user));
		break;
	case USERINFO_REASON_ROOMCREATED:
		//printf("user %s created room\n", LOBBY_GetUserName(user));
		break;
	case USERINFO_REASON_ROOMCHANGE:
		//printf("user %s changed room\n", LOBBY_GetUserName(user));
		break;
	}
}

void LocalConnectionManager::OnPacket(unsigned char *data, int length, LPLOBBY_USER from)
{
	Connection* connection = FindConnection(from);
	if(connection == NULL) {
		connection = CreateConnection(from);
	}
	
	/* Copy the message to avoid unaligned memory */
	char packet[MAX_PACKET_SIZE];
	memcpy(packet, data, length);
	
	MessageHeader* header = (MessageHeader*)packet;
	
	reciever->MessageIn(
		connection,
		header->messageId,
		packet + sizeof(MessageHeader),
		length - sizeof(MessageHeader));
}
