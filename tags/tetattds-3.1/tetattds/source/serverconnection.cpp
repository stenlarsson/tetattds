#include "tetattds.h"
#include "serverconnection.h"
#include "game.h"
#include "wrapping.h"
#include <limits.h>

ServerConnection::ServerConnection(const char* name)
:	connection(NULL),
	state(SERVERSTATE_DISCONNECTED),
	wins(0),
	name(strdup(name)),
	sendFieldStateDeltaTimer(INT_MAX),
	fieldState(0)
{
	ASSERT(g_fieldGraphics != NULL);
	ASSERT(name != NULL);
	memset(players, 0, sizeof(players));
	for(int i = 0; i < MAX_PLAYERS; i++) {
		PlayerInfo& player = players[i];
		memset(player.fieldState, TILE_BLANK, sizeof(player.fieldState));
	}

	ClearDeltaStore();
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

void ServerConnection::Tick()
{
	if(sendFieldStateDeltaTimer-- <= 0) {
		SendFieldStateDelta();

		sendFieldStateDeltaTimer =
			SEND_FIELDSTATE_DELTA_INTERVAL *
			GetConnectedPlayersCount();
	}
}

int ServerConnection::GetConnectedPlayersCount()
{
	int count = 0;
	for(int i = 0; i < MAX_PLAYERS; i++) {
		if(players[i].connected) {
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
	
	g_fieldGraphics->AddLog("Server is full\n");
	state = SERVERSTATE_DISCONNECTED;
	this->connection = NULL;
}

void ServerConnection::ConnectionTimeout(Connection* connection)
{
	ASSERT(connection != NULL);
	
	g_fieldGraphics->AddLog("Connection timed out\n");
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

void ServerConnection::mFieldStateDelta(Connection* from, FieldStateDeltaMessage* fieldStateDelta)
{
	ASSERT(fieldStateDelta->playerNum < MAX_PLAYERS);
	DEBUGVERBOSE("ServerConn: mFieldStateDelta %d, %d\n", fieldStateDelta->playerNum, players[fieldStateDelta->playerNum].fieldNum);
	if(state == SERVERSTATE_CONNECTED) {
		// not yet accepted, we need to know our playerNum first
		return;
	}

	wrapping8 fieldState(fieldStateDelta->acks[fieldStateDelta->playerNum]);

	PlayerInfo& player = players[fieldStateDelta->playerNum];
	
	if(player.seenFieldState > fieldState) {
		// out of order packet
		return;
	}

	// Record the seen state num, so that we can ack correctly
	player.seenFieldState = fieldState;
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

	unsigned int playerOnMyLeft = myPlayerNum;
	for(int i = 0; i < MAX_PLAYERS; i++) {
		--playerOnMyLeft %= MAX_PLAYERS;
		if(players[playerOnMyLeft].connected) {
			break;
		}
	}

	if(fieldStateDelta->playerNum == playerOnMyLeft) {
		// it our turn to send. we can do it right now and not have collisions
		sendFieldStateDeltaTimer = 0;
	}
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

	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		players[i].seenFieldState = accepted->seenFieldStates[i];
	}

	// try to spread out sending of the first field state
	sendFieldStateDeltaTimer =
		myPlayerNum *
		SEND_FIELDSTATE_DELTA_INTERVAL;
}

void ServerConnection::mDisconnect(Connection* from, DisconnectMessage* disconnect)
{
	DEBUGVERBOSE("ServerConn: mDisconnect\n");
	g_fieldGraphics->AddLog("Disconnected!\n");
	g_fieldGraphics->AddLog(disconnect->message);
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
	player.typing = playerInfo->typing != 0;

	if(!player.connected) {
		player.connected = true;
		player.ackedFieldState = fieldState;
		memset(player.fieldState, TILE_BLANK, sizeof(player.fieldState));
		ClearDeltaStore();
	}

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
	memset(&player, 0, sizeof(player));
	g_fieldGraphics->ClearPlayer(playerDisconnected->playerNum);
}

void ServerConnection::SendFieldStateDelta()
{
	if(state == SERVERSTATE_CONNECTED) {
		// not yet accepted, we need to know our playerNum first
		return;
	}
	PlayerInfo& me = players[myPlayerNum];

	// Compute where we must base our state
	uint8_t baseState(fieldState);
	for(unsigned int i = 0; i < MAX_PLAYERS; i++) {
		PlayerInfo& player = players[i];
		if (player.connected && (int)player.ackedFieldState - (player.ackedFieldState>fieldState)?256:0 < baseState)
			baseState = player.ackedFieldState;
	}
	
	// Move read position into place
	uint8_t deltaStoreBegin = deltaStoreBegins[baseState];
	uint8_t deltaStoreOriginalEnd = deltaStoreEnd;
	
	// Compute new delta items from current field state
	uint8_t delta[12*6];
	unsigned int length = 0;
	for(unsigned int i = 0; i < sizeof(me.fieldState); i++) {
		if(me.fieldState[i] != lastFieldState[i]) {
			delta[length++] = deltaStore[deltaStoreEnd++] = i;
			delta[length++] = deltaStore[deltaStoreEnd++] = me.fieldState[i];

			// Not needed thanks to deltaStore size and deltaStoreEnd datatype
			// if (deltaStoreEnd == sizeof(deltaStore))
			// 	deltaStoreEnd = 0;

			if (deltaStoreBegin == deltaStoreEnd || length == sizeof(delta)) {
				length = sizeof(delta);
				break;
			}
		}
	}
	memcpy(lastFieldState, me.fieldState, sizeof(lastFieldState));
	
	if (length == sizeof(delta)) {
		// Clear delta store...
		deltaStoreBegin = 0;
		ClearDeltaStore();
	}
	else if (deltaStoreBegin != deltaStoreEnd && deltaStore[deltaStoreBegin+1] == 255) {
		// We have orders to send full state...
		length = sizeof(delta);
	}
	else {
		// Add the new start point (unless empty delta)
		if (length != 0) {
			deltaStoreBegins[++fieldState] = deltaStoreEnd;
		}
		
		// Compose new delta with existing deltas for sending
		uint8_t pos = deltaStoreBegin;
		while (pos != deltaStoreOriginalEnd) {
			// Send only data that is still valid...
			if (me.fieldState[deltaStore[pos]] == deltaStore[pos+1]) {
				delta[length++] = deltaStore[pos];
				delta[length++] = deltaStore[pos+1];

				if(length == sizeof(delta)) {
					break;
				}
			}
			
			pos += 2;
			// Not needed thanks to deltaStore size and deltaStoreEnd datatype
			// if (pos == sizeof(deltaStore))
			// 	pos = 0;
		}
	}
	
	if (length == sizeof(delta)) {
		// Send full state instead of delta...
		memcpy(delta, me.fieldState, sizeof(delta));
	}
	
	// Note that we should send the message even when length == 0,
	// since the other players want to see our ack numbers...
	FieldStateDeltaMessage message;
	message.playerNum = myPlayerNum;
	me.ackedFieldState = fieldState;
	me.seenFieldState = fieldState;
	for(unsigned int i = 0; i < MAX_PLAYERS; i++) {
		message.acks[i] = players[i].seenFieldState;
	}
	message.length = length;
	memcpy(message.delta, delta, length);
	connectionManager->BroadcastMessage(message);
}

void ServerConnection::ClearDeltaStore()
{
	deltaStoreEnd = 0;
	
	// Insert a missing delta marker
	deltaStore[deltaStoreEnd++] = 0;
	deltaStore[deltaStoreEnd++] = 255;
	
	// Every state now requires a full update
	memset(deltaStoreBegins, 0, sizeof(deltaStoreBegins));
	deltaStoreBegins[++fieldState] = deltaStoreEnd;

}
