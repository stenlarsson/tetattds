#define _CRT_SECURE_NO_DEPRECATE
#include "network.h"
#include "servergame.h"
#include "player.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "protocol.h"

#ifdef WIN32
#define snprintf _snprintf
#endif


void PrintStatus(const char* format, ...);

ServerGame::ServerGame()
:	numPlayers(0),
	state(GAMESTATE_WAITING)
{
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		players[i] = NULL;
	}

	PrintStatus(VERSION_STRING " server\n");

}

ServerGame::~ServerGame()
{
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		delete players[i];
		players[i] = NULL;
	}
}

void ServerGame::ConnectionCreated(Connection* connection)
{
	ASSERT(connection != NULL);
	
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(players[i] == NULL)
		{
			PrintStatus("player %i connected", i);
			players[i] = new Player(i, connection);
			numPlayers++;
			return;
		}
	}
}

void ServerGame::MessageIn(Connection* from, unsigned char id, const void* data, size_t size)
{
	ASSERT(from != NULL);
	ASSERT(data != NULL);
	ASSERT(size > 0);
	
	BEGIN_MESSAGE_HANDLER
	HANDLE_MESSAGE(Ping)
	HANDLE_MESSAGE(Garbage)
	HANDLE_MESSAGE(FieldState)
	HANDLE_MESSAGE(Chat)
	HANDLE_MESSAGE(Connect)
	HANDLE_MESSAGE(Quit)
	HANDLE_MESSAGE(Died)
	HANDLE_MESSAGE(SetInfo)
	END_MESSAGE_HANDLER
}

void ServerGame::ConnectionRejected(Connection* connection)
{
	ASSERT(false);
}

void ServerGame::ConnectionTimeout(Connection* connection)
{
	ASSERT(connection != NULL);

	Player* player = GetPlayer(connection);
	PrintStatus("player %s timed out", player->name);

	if(player->state == PLAYERSTATE_ALIVE)
	{
		// without this fix, nobody will be "2nd"
		nextPlace--;
	}

	player->state = PLAYERSTATE_DISCONNECTED;
}

void ServerGame::Tick()
{
	int playersReady = 0;
	int playersAlive = 0;
	int activePlayers = 0;
	Player* lastPlayerAlive = NULL;
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(players[i] == NULL)
			continue;

		if(players[i]->state == PLAYERSTATE_DISCONNECTED)
		{
			PlayerDisconnectedMessage message;
			message.playerNum = i;
			SendMessageToAll(message, i);
			
			ChatMessage chat;
			sprintf(chat.text, "* %s disconnected.", players[i]->name);
			SendMessageToAll(chat, i);
			
			PrintStatus("player %i disconnected", i);
			delete players[i];
			players[i] = NULL;
			
			numPlayers--;
		}
		else if(players[i]->state == PLAYERSTATE_READY)
		{
			playersReady++;
		}
		else if(players[i]->state == PLAYERSTATE_ALIVE)
		{
			playersAlive++;
			activePlayers++;
			lastPlayerAlive = players[i];
		}
		else if(players[i]->state == PLAYERSTATE_DEAD)
		{
			if(++players[i]->deadTime > DEAD_RETURN_TO_MENU_TIME)
			{
				GameEndMessage message;
				message.winner = -1;
				players[i]->connection->SendMessage(message);
				players[i]->state = PLAYERSTATE_CONNECTED;
			}
			else
			{
				activePlayers++;
			}
		}
	}

	switch(state)
	{
	case GAMESTATE_WAITING:
		if(playersReady == numPlayers && numPlayers > 0)
		{
			PrintStatus("game started");

			state = GAMESTATE_RUNNING;
			nextPlace = numPlayers;

			GameStartMessage message;
			SendMessageToAll(message);

			ChatMessage chat;
			sprintf(chat.text, "* Game started!");
			SendMessageToAll(chat);

			for(int i = 0; i < MAX_PLAYERS; i++)
			{
				if(players[i] != NULL)
					players[i]->state = PLAYERSTATE_ALIVE;
			}

			startTime = time(NULL);
		}
		break;
		
	case GAMESTATE_RUNNING:
		if( (activePlayers > 1 && playersAlive <= 1) ||
			(activePlayers == 1 && playersAlive == 0) ||
			(activePlayers == 0) )
		{
			GameEndMessage message;
			if(lastPlayerAlive != NULL)
			{
				lastPlayerAlive->wins++;
				PlayerInfoMessage pimessage;
				lastPlayerAlive->FillPlayerInfoMessage(pimessage);
				SendMessageToAll(pimessage);

				message.winner = lastPlayerAlive->playerNum;
				PrintStatus("game over, winner is %s", lastPlayerAlive->name);
				ChatMessage chat;
				sprintf(chat.text, "* %s wins!", lastPlayerAlive->name);
				SendMessageToAll(chat);
			}
			else
			{
				message.winner = -1;
				PrintStatus( "game over, no winner" );
				ChatMessage chat;
				if(activePlayers == 1)
					sprintf(chat.text, "* Game Over.");
				else
					sprintf(chat.text, "* It's a tie!");
				SendMessageToAll(chat);
			}

			int gameTime = time(NULL) - startTime - 3; // subtract countdown
			ChatMessage chat;
			sprintf(chat.text, "* The game lasted %i:%02i", gameTime/60, gameTime%60);
			SendMessageToAll(chat);

			SendMessageToAll( message );
			for( int i = 0; i < MAX_PLAYERS; i++ )
			{
				if( players[i] != NULL  && players[i]->state != PLAYERSTATE_READY)
					players[i]->state = PLAYERSTATE_CONNECTED;
			}

			state = GAMESTATE_WAITING;
		}
		break;
	}
}

void ServerGame::SendChat(const char* text)
{
	ChatMessage chat;
	snprintf(chat.text, sizeof(chat.text), "Server: %s", text);
	chat.text[sizeof(chat.text)-1] = '\0';
	SendMessageToAll(chat);
}

void ServerGame::mPing(Connection* from, PingMessage* ping)
{
}

void ServerGame::mGarbage(Connection* from, GarbageMessage* garbage)
{
	Player* player = GetPlayer(from);
	Player* reciever = DetermineReciever(player);
	if( reciever != NULL )
		reciever->connection->SendMessage(*garbage);
}

void ServerGame::mFieldState(Connection* from, FieldStateMessage* fieldState)
{
}

void ServerGame::mChat(Connection* from, ChatMessage* chat)
{
	Player* player = GetPlayer(from);
	char buffer[256];
	sprintf(buffer, "%s: %s", player->name, chat->text);
	strncpy(chat->text, buffer, sizeof(chat->text));
	chat->text[sizeof(chat->text)-1] = '\0';
	PrintStatus("%s", chat->text);
	SendMessageToAll(*chat);
}

void ServerGame::mConnect(Connection* from, ConnectMessage* connect)
{
	DisconnectMessage dm;
	if(connect->version < VERSION)
	{
		sprintf(dm.message, "Client version too old!\nPlease upgrade.\n" VERSION_STRING " is the latest.\n");
		PrintStatus("Client with old version(%d) tried to join.", (int)connect->version);
	}
	else if(connect->version > VERSION)
	{
		sprintf(dm.message, "Server version too old!\n");
		PrintStatus("Client with newer version(%d) tried to join. Please upgrade the server.", (int)connect->version);
	}
	if(connect->version != VERSION)
	{
		from->SendMessage(dm);
		Player* player = GetPlayer(from);
		player->state = PLAYERSTATE_DISCONNECTED;
		from->Close();
		return;
	}


	Player* player = GetPlayer(from);
	
	player->name = strdup( connect->name );

	PrintStatus("player %s joined", player->name);

	AcceptedMessage am;
	am.playerNum = player->playerNum;
	player->connection->SendMessage(am);

	PlayerInfoMessage pimessage;
	player->FillPlayerInfoMessage(pimessage);
	SendMessageToAll(pimessage);

	ChatMessage chat;
	sprintf(chat.text, "* %s joined", player->name);
	SendMessageToAll(chat, player->playerNum);

	// gather info about the other players
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		Player* otherPlayer = players[i];
		if(otherPlayer != NULL && otherPlayer != player && otherPlayer->state >= PLAYERSTATE_CONNECTED)
		{
			PlayerInfoMessage theirInfo;
			otherPlayer->FillPlayerInfoMessage(theirInfo);
			player->connection->SendMessage(theirInfo);
		}
	}

	player->state = PLAYERSTATE_CONNECTED;
}

void ServerGame::mQuit(Connection* from, QuitMessage* quit)
{
	Player* player = GetPlayer(from);
	player->state = PLAYERSTATE_DISCONNECTED;
	player->connection->Close();
}

void ServerGame::mDied(Connection* from, DiedMessage* died)
{
	Player* player = GetPlayer(from);

	int place = nextPlace--;
	player->place = place;
	player->state = PLAYERSTATE_DEAD;
	player->deadTime = 0;

	PlayerDiedMessage message;
	message.playerNum = player->playerNum;
	message.place = place;

	SendMessageToAll(message, player->playerNum);
}

void ServerGame::mSetInfo(Connection* from, SetInfoMessage* setInfo)
{
	Player* player = GetPlayer(from);

	if(player->state != PLAYERSTATE_READY && setInfo->ready)
	{
		ChatMessage chat;
		sprintf(chat.text, "* %s is ready!", player->name);
		SendMessageToAll(chat, player->playerNum);
	}
	if(player->state == PLAYERSTATE_READY && !setInfo->ready)
	{
		ChatMessage chat;
		sprintf(chat.text, "* %s is not ready", player->name);
		SendMessageToAll(chat, player->playerNum);
	}
	if(player->level != setInfo->level)
	{
		ChatMessage chat;
		sprintf(chat.text, "* %s selected level %i", player->name, setInfo->level+1);
		SendMessageToAll(chat, player->playerNum);
	}

	player->level = setInfo->level;
	player->state = setInfo->ready ? PLAYERSTATE_READY : PLAYERSTATE_CONNECTED;
	player->typing = setInfo->typing;

	PlayerInfoMessage pimessage;
	player->FillPlayerInfoMessage(pimessage);
	SendMessageToAll(pimessage);
}

Player* ServerGame::GetPlayer(Connection* connection)
{
	ASSERT(connection != NULL);
	
	for(int i = 0; i < MAX_PLAYERS; i++)
	{
		if(players[i] != NULL && players[i]->connection == connection)
			return players[i];
	}

	ASSERT(false);
	return NULL;
}

Player* ServerGame::DetermineReciever(Player* from)
{
	ASSERT(from != NULL);
	
	int victims[MAX_PLAYERS];
	memset(victims, -1, sizeof(victims));

	for(int i = 0;i<MAX_PLAYERS;i++)
	{
		bool bOk;
		do
		{
			bOk = true;
			int r = rand() % MAX_PLAYERS;
			if(victims[r] == -1)
				victims[r] = i;
			else
				bOk = false;
				
		} while(!bOk);
	}

	for(int i = 0;i<MAX_PLAYERS;i++)
	{
		Player* vict = players[victims[i]];
		if(vict == NULL)
			continue;
		if(vict == from)
			continue;
		if(vict->state != PLAYERSTATE_ALIVE)
			continue;

		return vict;
	}

	return from;
}
