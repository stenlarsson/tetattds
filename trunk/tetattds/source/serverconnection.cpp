#include "tetattds.h"
#include "serverconnection.h"
#include "game.h"

ServerConnection::ServerConnection(const char* name)
:	connection(NULL),
	state(SERVERSTATE_DISCONNECTED),
	wins(0),
	name(strdup(name))
{
	ASSERT(g_fieldGraphics != NULL);
	ASSERT(name != NULL);
	memset(players, 0, sizeof(players));
	for(int i = 0; i < MAX_PLAYERS; i++) {
		PlayerInfo& player = players[i];
		for(int j = 0; j < 12*6; j++) {
			player.fieldState[j] = TILE_BLANK;
		}
	}
}

ServerConnection::~ServerConnection()
{
	// Clean up graphics when disconnected
	for(int i = 0; i < MAX_PLAYERS; i++) {
		if(players[i].connected) {
			players[i].connected = false;
			g_fieldGraphics->ClearPlayer(i);
		}
	}

	free(name);
}

int ServerConnection::GetAlivePlayersCount()
{
	int count = 0;
	for(int i = 0; i < MAX_PLAYERS; i++) {
		if(players[i].connected && !players[i].dead) {
			count++;
		}
	}
	return count;
}

void ServerConnection::ConnectionCreated(Connection* connection)
{
	ASSERT(connection != NULL);
	
	this->connection = connection;
	state = SERVERSTATE_CONNECTED;
	ConnectMessage connect;
	connect.version = VERSION;
	strncpy(connect.name, name, sizeof(connect.name));
	SendMessage(connect);
}

void ServerConnection::MessageIn(Connection* from, unsigned char id, const void* data, size_t size)
{
	ASSERT(data != NULL);
	ASSERT(size > 0);

	BEGIN_MESSAGE_HANDLER
	HANDLE_MESSAGE(Ping)
	HANDLE_MESSAGE(Garbage)
	HANDLE_MESSAGE(FieldState)
	HANDLE_MESSAGE(FieldStateDelta)
	HANDLE_MESSAGE(Chat)
	HANDLE_MESSAGE(Accepted)
	HANDLE_MESSAGE(Disconnect)
	HANDLE_MESSAGE(GameStart)
	HANDLE_MESSAGE(GameEnd)
	HANDLE_MESSAGE(PlayerInfo)
	HANDLE_MESSAGE(PlayerDied)
	HANDLE_MESSAGE(PlayerDisconnected)
	END_MESSAGE_HANDLER
}

void ServerConnection::ConnectionRejected(Connection* connection)
{
	ASSERT(connection != NULL);
	
	PrintStatus("Server is full\n");
	state = SERVERSTATE_DISCONNECTED;
	this->connection = NULL;
}

void ServerConnection::ConnectionTimeout(Connection* connection)
{
	ASSERT(connection != NULL);
	
	PrintStatus("Connection timed out\n");
	state = SERVERSTATE_DISCONNECTED;
	this->connection = NULL;
}

void ServerConnection::Shutdown()
{
	DEBUGVERBOSE("ServerConn: shutDown\n");
	QuitMessage message;
	SendMessage( message );
	connection->Close();
}

/*
 * Common messages
 */
void ServerConnection::mPing(Connection* from, PingMessage* ping)
{
}

void ServerConnection::mGarbage(Connection* from, GarbageMessage* garbage)
{
	ASSERT(g_game != NULL);
	
	DEBUGVERBOSE("ServerConn: mGarbage %d, %d, %d\n",
		garbage->num,
		garbage->player,
		garbage->type);
	g_game->ReceiveGarbage(
		garbage->num,
		garbage->player,
		(GarbageType)(int)garbage->type);
}

void ServerConnection::mFieldState(Connection* from, FieldStateMessage* fieldState)
{
	ASSERT(fieldState->playerNum < MAX_PLAYERS);
	DEBUGVERBOSE("ServerConn: mFieldState %d, %d\n", fieldState->playerNum, players[fieldState->playerNum].fieldNum);
	
	PlayerInfo& player = players[fieldState->playerNum];
	memcpy(player.fieldState, fieldState->field, sizeof(player.fieldState));

	g_fieldGraphics->PrintPlayerInfo(fieldState->playerNum, &player);
}

void ServerConnection::mFieldStateDelta(Connection* from, FieldStateDeltaMessage* fieldStateDelta)
{
	ASSERT(fieldStateDelta->playerNum < MAX_PLAYERS);
	DEBUGVERBOSE("ServerConn: mFieldStateDelta %d, %d\n", fieldStateDelta->playerNum, players[fieldStateDelta->playerNum].fieldNum);
	PlayerInfo& player = players[fieldStateDelta->playerNum];
	
	// Record the seen state num, so that we can ack correctly
	player.seenFieldState = fieldStateDelta->acks[fieldStateDelta->playerNum];
	player.ackedFieldState = fieldStateDelta->acks[myPlayerNum];

	if (fieldStateDelta->length < sizeof(player.fieldState)) {
		for(int i = 0; i < fieldStateDelta->length; i += 2) {
			player.fieldState[fieldStateDelta->delta[i]] = fieldStateDelta->delta[i+1];
		}
	}
	else {
		// We got the full package...
		memcpy(player.fieldState, fieldStateDelta->delta, sizeof(player.fieldState));
	}

	g_fieldGraphics->PrintPlayerInfo(fieldStateDelta->playerNum, &player);
}

void ServerConnection::mChat(Connection* from, ChatMessage* chat)
{
	DEBUGVERBOSE("ServerConn: mChat\n");
	g_fieldGraphics->AddChat(chat->text);
}

/*
 * Server messages
 */
void ServerConnection::mAccepted(Connection* from, AcceptedMessage* accepted)
{
	DEBUGVERBOSE("ServerConn: mAccepted\n");
	ASSERT(accepted->playerNum < MAX_PLAYERS);
	
	myPlayerNum = accepted->playerNum;
	state = SERVERSTATE_ACCEPTED;
}

void ServerConnection::mDisconnect(Connection* from, DisconnectMessage* disconnect)
{
	DEBUGVERBOSE("ServerConn: mDisconnect\n");
	PrintStatus("Disconnected!\n");
	PrintStatus(disconnect->message);
	state = SERVERSTATE_DISCONNECTED;
}

void ServerConnection::mGameStart(Connection* from, GameStartMessage* gameStart)
{
	DEBUGVERBOSE("ServerConn: mGameStart\n");
	state = SERVERSTATE_GAME_STARTED;
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(!players[i].connected)
			continue;
		
		players[i].dead = false;
		players[i].place = 0;
		players[i].ready = false;
		players[i].typing = false;
		g_fieldGraphics->PrintPlayerInfo(i, &players[i]);
	}
}

void ServerConnection::mGameEnd(Connection* from, GameEndMessage* gameEnd)
{
	DEBUGVERBOSE("ServerConn: mGameEnd\n");
	ASSERT(gameEnd->winner < MAX_PLAYERS || gameEnd->winner == 255);
	
	if(gameEnd->winner != 255)
	{
		if(gameEnd->winner == myPlayerNum)
		{
			wins++;
		}
		PlayerInfo& player = players[gameEnd->winner];
		player.place = 1;
		g_fieldGraphics->PrintPlayerInfo(gameEnd->winner, &player);
	}
	
	state = SERVERSTATE_GAME_ENDED;
}

void ServerConnection::mPlayerInfo(Connection* from, PlayerInfoMessage* playerInfo)
{
	DEBUGVERBOSE("ServerConn: mPlayerInfo\n");
	ASSERT(playerInfo->playerNum < MAX_PLAYERS);
	
	PlayerInfo& player = players[playerInfo->playerNum];
	strncpy(player.name, playerInfo->name, sizeof(player.name));
	player.level = playerInfo->level;
	player.wins = playerInfo->wins;
	player.ready = playerInfo->ready != 0;
	player.connected = true;
	player.typing = playerInfo->typing != 0;
	g_fieldGraphics->PrintPlayerInfo(playerInfo->playerNum, &player);
}

void ServerConnection::mPlayerDied(Connection* from, PlayerDiedMessage* playerDied)
{
	DEBUGVERBOSE("ServerConn: mPlayerDied\n");
	ASSERT(playerDied->playerNum < MAX_PLAYERS);
	
	PlayerInfo& player = players[playerDied->playerNum];
	player.dead = true;
	player.place = playerDied->place;
	
	g_fieldGraphics->PrintPlayerInfo(playerDied->playerNum, &player);
}

void ServerConnection::mPlayerDisconnected(Connection* from, PlayerDisconnectedMessage* playerDisconnected)
{
	DEBUGVERBOSE("ServerConn: mPlayerDisconnected\n");
	ASSERT(playerDisconnected->playerNum < MAX_PLAYERS);
	
	PlayerInfo& player = players[playerDisconnected->playerNum];
	player.connected = false;
	g_fieldGraphics->ClearPlayer(playerDisconnected->playerNum);
}
