#pragma once

#include "protocol.h"
#include "player.h"

#define VERSION 31

#define DEAD_RETURN_TO_MENU_TIME 180

class Socket;

enum ServerGameState
{
	GAMESTATE_WAITING,
	GAMESTATE_RUNNING
};

class ServerGame : public MessageReciever {
public:
	ServerGame();
	~ServerGame();
	
	virtual void ConnectionCreated(Connection* connection);
	virtual void MessageIn(Connection* from, unsigned char id, const void* data, size_t size);
	virtual void ConnectionRejected(Connection* connection);
	virtual void ConnectionTimeout(Connection* connection);

	void Tick();
	void SendChat(const char* text);

	template<typename T>
	void SendMessageToAll(T& message)
	{
		for(int i = 0; i < MAX_PLAYERS; i++) {
			if(players[i] != 0 && players[i]->state != PLAYERSTATE_DISCONNECTED)
				players[i]->connection->SendMessage(message);
		}
	}

private:
	void mPing(Connection* from, PingMessage* ping);
	void mGarbage(Connection* from, GarbageMessage* garbage);
	void mFieldStateDelta(Connection* from, FieldStateDeltaMessage* fieldStateDelta);
	void mChat(Connection* from, ChatMessage* chat);

	void mConnect(Connection* from, ConnectMessage* connect);
	void mQuit(Connection* from, QuitMessage* quit);
	void mDied(Connection* from, DiedMessage* died);
	void mSetInfo(Connection* from, SetInfoMessage* setInfo);

	Player* GetPlayer(Connection* connection);
	Player* DetermineReciever(Player* from);

	int numPlayers;
	Player* players[MAX_PLAYERS];
	
	int nextPlace;

	ServerGameState state;

	int startTime;
};
