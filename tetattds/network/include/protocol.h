#pragma once

#include "netval.h"

#define MAX_PACKET_SIZE 512
#define CONNECTION_KEEPALIVE_TIME 2
#define CONNECTION_TIMEOUT 10
#define RESEND_PACKET_TIMEOUT 1

#define PACKET_TYPE_ACK 0
#define PACKET_TYPE_REJECT 1
#define PACKET_TYPE_UNRELIABLE 2
#define PACKET_TYPE_ORDERED 3
#define PACKET_TYPE_KEEPALIVE 4
#define PACKET_TYPE_BROADCAST 5

#define MESSAGE_PING 000
#define MESSAGE_GARBAGE 001
#define MESSAGE_FIELDSTATE 002
#define MESSAGE_CHAT 003
#define MESSAGE_CONNECT 100
#define MESSAGE_QUIT 101
#define MESSAGE_DIED 102
#define MESSAGE_SET_INFO 104
#define MESSAGE_ACCEPTED 200
#define MESSAGE_DISCONNECT 201
#define MESSAGE_GAMESTART 202
#define MESSAGE_GAMEEND 203
#define MESSAGE_PLAYER_INFO 204
#define MESSAGE_PLAYER_DIED 205
#define MESSAGE_PLAYER_DISCONNECTED 206

struct MessageHeader
{
	MessageHeader(unsigned char packetType, unsigned char messageId, unsigned int sequence)
	:	packetType(packetType),
		messageId(messageId),
		sequence(sequence)
	{
	}

	unsigned char packetType;
	unsigned char messageId;
	netval<unsigned int> sequence;
};

struct PingMessage
{
	static const unsigned char packetType = PACKET_TYPE_UNRELIABLE;
	static const unsigned char messageId = MESSAGE_PING;
};

struct GarbageMessage
{
	static const unsigned char packetType = PACKET_TYPE_ORDERED;
	static const unsigned char messageId = MESSAGE_GARBAGE;
	netval<int> num;
	netval<int> player;
	netval<int> type;
};

struct FieldStateMessage
{
	static const unsigned char packetType = PACKET_TYPE_UNRELIABLE;
	static const unsigned char messageId = MESSAGE_FIELDSTATE;
	netval<int> playerNum;
	char field[12*6];
};

struct ChatMessage
{
	static const unsigned char packetType = PACKET_TYPE_ORDERED;
	static const unsigned char messageId = MESSAGE_CHAT;
	char text[128+1];
};

struct ConnectMessage
{
	static const unsigned char packetType = PACKET_TYPE_ORDERED;
	static const unsigned char messageId = MESSAGE_CONNECT;
	netval<int> version;
	char name[64];
};

struct QuitMessage
{
	static const unsigned char packetType = PACKET_TYPE_ORDERED;
	static const unsigned char messageId = MESSAGE_QUIT;
};

struct DiedMessage
{
	static const unsigned char packetType = PACKET_TYPE_ORDERED;
	static const unsigned char messageId = MESSAGE_DIED;
};

struct SetInfoMessage
{
	static const unsigned char packetType = PACKET_TYPE_ORDERED;
	static const unsigned char messageId = MESSAGE_SET_INFO;
	netval<int> level;
	netval<bool> ready;
	netval<bool> typing;
};

struct AcceptedMessage
{
	static const unsigned char packetType = PACKET_TYPE_ORDERED;
	static const unsigned char messageId = MESSAGE_ACCEPTED;
	netval<int> playerNum;
};

struct DisconnectMessage
{
	static const unsigned char packetType = PACKET_TYPE_ORDERED;
	static const unsigned char messageId = MESSAGE_DISCONNECT;
	char message[128+1];
};

struct GameStartMessage
{
	static const unsigned char packetType = PACKET_TYPE_ORDERED;
	static const unsigned char messageId = MESSAGE_GAMESTART;
};

struct GameEndMessage
{
	static const unsigned char packetType = PACKET_TYPE_ORDERED;
	static const unsigned char messageId = MESSAGE_GAMEEND;
	netval<int> winner;
};

struct PlayerInfoMessage
{
	static const unsigned char packetType = PACKET_TYPE_ORDERED;
	static const unsigned char messageId = MESSAGE_PLAYER_INFO;
	netval<int> playerNum;
	char name[64];
	netval<int> level;
	netval<int> wins;
	netval<bool> ready;
	netval<bool> typing;
};

struct PlayerDiedMessage
{
	static const unsigned char packetType = PACKET_TYPE_ORDERED;
	static const unsigned char messageId = MESSAGE_PLAYER_DIED;
	netval<int> playerNum;
	netval<int> place;
};

struct PlayerDisconnectedMessage
{
	static const unsigned char packetType = PACKET_TYPE_ORDERED;
	static const unsigned char messageId = MESSAGE_PLAYER_DISCONNECTED;
	netval<int> playerNum;
};
