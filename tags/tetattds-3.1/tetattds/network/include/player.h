#pragma once

#include <stdlib.h>
#include <string.h>
#include "connection.h"

class Game;

enum PlayerState
{
	PLAYERSTATE_DISCONNECTED,
	PLAYERSTATE_CONNECTING,
	PLAYERSTATE_CONNECTED,
	PLAYERSTATE_READY,
	PLAYERSTATE_ALIVE,
	PLAYERSTATE_DEAD
};

class Player
{
public:
	Player(int playerNum, Connection* connection)
	:	connection(connection),
		state(PLAYERSTATE_CONNECTING),
		playerNum(playerNum),
		name(NULL),
		place(0),
		level(4),
		wins(0),
		typing(false),
		deadTime(0),
		seenFieldState(0)
	{
	}

	~Player()
	{
		free(name);
	}

	void FillPlayerInfoMessage(PlayerInfoMessage& message)
	{
		message.playerNum = playerNum;
		strncpy(message.name, name, sizeof(message.name));
		message.level = level;
		message.wins = wins;
		message.ready = (state == PLAYERSTATE_READY);
		message.typing = typing;
	}

	Connection *connection;
	PlayerState state;
	int playerNum;
	char* name;
	int place;
	int level;
	int wins;
	bool typing;
	int deadTime;
	uint8_t seenFieldState;
};
