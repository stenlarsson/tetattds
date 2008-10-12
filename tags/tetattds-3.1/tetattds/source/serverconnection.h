#pragma once

#include "connection.h"
#include "connectionmanager.h"

#include "fieldgraphics.h"

enum ServerState
{
	SERVERSTATE_DISCONNECTED,
	SERVERSTATE_CONNECTED,
	SERVERSTATE_ACCEPTED,
	SERVERSTATE_GAME_STARTED,
	SERVERSTATE_GAME_ENDED
};

class ServerConnection : public MessageReciever
{
public:
	ServerConnection(const char* name);
	virtual ~ServerConnection();
	
	void SetConnectionManager(ConnectionManager* connectionManager)
	{ this->connectionManager = connectionManager; }

	void Tick();

	ServerState GetState() { return state; }
	inline bool IsState(ServerState const & s) { return state == s; }
	PlayerInfo* GetPlayerInfo(int playerNum)
	{
		ASSERT(playerNum >= 0);
		ASSERT(playerNum < MAX_PLAYERS);
		return &players[playerNum];
	}

	void Shutdown();
	
	int GetWins() { return wins; }
	int GetMyPlayerNum() { return myPlayerNum; }

	int GetConnectedPlayersCount();

	template<typename T>
	void SendMessage(const T& message)
	{
		ASSERT(connection != NULL);
		connection->SendMessage(message);
	}

	virtual void ConnectionCreated(Connection* connection);
	virtual void MessageIn(Connection* from, unsigned char id, const void* data, size_t size);
	virtual void ConnectionRejected(Connection* connection);
	virtual void ConnectionTimeout(Connection* connection);

	void mPing(Connection* from, PingMessage* ping);
	void mGarbage(Connection* from, GarbageMessage* garbage);
	void mFieldStateDelta(Connection* from, FieldStateDeltaMessage* fieldStateDelta);
	void mChat(Connection* from, ChatMessage* chat );

	void mAccepted(Connection* from, AcceptedMessage* ping);
	void mDisconnect(Connection* from, DisconnectMessage* disconnect);
	void mGameStart(Connection* from, GameStartMessage* gameStart);
	void mGameEnd(Connection* from, GameEndMessage* gameEnd);
	void mPlayerInfo(Connection* from, PlayerInfoMessage* playerInfo);
	void mPlayerDied(Connection* from, PlayerDiedMessage* playerDied);
	void mPlayerDisconnected(Connection* from, PlayerDisconnectedMessage* playerDisconnected);

private:
	void SendFieldStateDelta();
	void ClearDeltaStore();

	ConnectionManager* connectionManager;
	Connection* connection;
	ServerState state;
	int myPlayerNum;
	PlayerInfo players[MAX_PLAYERS];
	int wins;
	int pingTimer;
	char* name;
	int sendFieldStateDeltaTimer;
	char lastFieldState[12*6];
	uint8_t fieldState;
	uint8_t deltaStore[256];
	uint8_t deltaStoreBegins[256];
	uint8_t deltaStoreEnd;
};
