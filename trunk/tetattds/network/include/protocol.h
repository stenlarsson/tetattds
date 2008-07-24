#pragma once

#include <stdint.h>
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
#define MESSAGE_FIELDSTATE_DELTA 004
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

#ifdef WIN32
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif

struct MessageHeader
{
	MessageHeader(uint8_t packetType, uint8_t messageId, uint16_t sequence)
	:	sequence(sequence),
		packetType(packetType),
		messageId(messageId)
	{
	}

	netval<uint16_t> sequence;
	uint8_t packetType;
	uint8_t messageId;
};

struct PingMessage
{
	static const uint8_t packetType = PACKET_TYPE_UNRELIABLE;
	static const uint8_t messageId = MESSAGE_PING;
	size_t size() const { return sizeof(PingMessage); }
};

struct GarbageMessage
{
	static const uint8_t packetType = PACKET_TYPE_ORDERED;
	static const uint8_t messageId = MESSAGE_GARBAGE;
	size_t size() const { return sizeof(GarbageMessage); }
	uint8_t num;
	uint8_t player;
	uint8_t type;
};

struct FieldStateMessage
{
	static const uint8_t packetType = PACKET_TYPE_UNRELIABLE;
	static const uint8_t messageId = MESSAGE_FIELDSTATE;
	size_t size() const { return sizeof(FieldStateMessage); }
	uint8_t playerNum;
	uint8_t field[12*6];
};

struct FieldStateDeltaMessage
{
	static const uint8_t packetType = PACKET_TYPE_UNRELIABLE;
	static const uint8_t messageId = MESSAGE_FIELDSTATE_DELTA;
	size_t size() const { return length + 2; }
	uint8_t playerNum;
	uint8_t length;
	uint8_t delta[12*6];
};

struct ChatMessage
{
	static const uint8_t packetType = PACKET_TYPE_ORDERED;
	static const uint8_t messageId = MESSAGE_CHAT;
	size_t size() const { return strnlen(text, 128+1) + 1; }
	char text[128+1];
};

struct ConnectMessage
{
	static const uint8_t packetType = PACKET_TYPE_ORDERED;
	static const uint8_t messageId = MESSAGE_CONNECT;
	size_t size() const { return strnlen(name, 64) + 1 + 1; }
	uint8_t version;
	char name[64];
};

struct QuitMessage
{
	static const uint8_t packetType = PACKET_TYPE_ORDERED;
	static const uint8_t messageId = MESSAGE_QUIT;
	size_t size() const { return sizeof(QuitMessage); }
};

struct DiedMessage
{
	static const uint8_t packetType = PACKET_TYPE_ORDERED;
	static const uint8_t messageId = MESSAGE_DIED;
	size_t size() const { return sizeof(DiedMessage); }
};

struct SetInfoMessage
{
	static const uint8_t packetType = PACKET_TYPE_ORDERED;
	static const uint8_t messageId = MESSAGE_SET_INFO;
	size_t size() const { return sizeof(SetInfoMessage); }
	uint8_t level;
	uint8_t ready;
	uint8_t typing;
};

struct AcceptedMessage
{
	static const uint8_t packetType = PACKET_TYPE_ORDERED;
	static const uint8_t messageId = MESSAGE_ACCEPTED;
	size_t size() const { return sizeof(AcceptedMessage); }
	uint8_t playerNum;
};

struct DisconnectMessage
{
	static const uint8_t packetType = PACKET_TYPE_ORDERED;
	static const uint8_t messageId = MESSAGE_DISCONNECT;
	size_t size() const { return strnlen(message, 128+1) + 1; }
	char message[128+1];
};

struct GameStartMessage
{
	static const uint8_t packetType = PACKET_TYPE_ORDERED;
	static const uint8_t messageId = MESSAGE_GAMESTART;
	size_t size() const { return sizeof(GameStartMessage); }
};

struct GameEndMessage
{
	static const uint8_t packetType = PACKET_TYPE_ORDERED;
	static const uint8_t messageId = MESSAGE_GAMEEND;
	size_t size() const { return sizeof(GameEndMessage); }
	uint8_t winner;
};

struct PlayerInfoMessage
{
	static const uint8_t packetType = PACKET_TYPE_ORDERED;
	static const uint8_t messageId = MESSAGE_PLAYER_INFO;
	size_t size() const { return strnlen(name, 64) + 1 + 5; }
	uint8_t playerNum;
	uint8_t level;
	uint8_t wins;
	uint8_t ready;
	uint8_t typing;
	char name[64];
};

struct PlayerDiedMessage
{
	static const uint8_t packetType = PACKET_TYPE_ORDERED;
	static const uint8_t messageId = MESSAGE_PLAYER_DIED;
	size_t size() const { return sizeof(PlayerDiedMessage); }
	uint8_t playerNum;
	uint8_t place;
};

struct PlayerDisconnectedMessage
{
	static const uint8_t packetType = PACKET_TYPE_ORDERED;
	static const uint8_t messageId = MESSAGE_PLAYER_DISCONNECTED;
	size_t size() const { return sizeof(PlayerDisconnectedMessage); }
	uint8_t playerNum;
};

#ifdef WIN32
#pragma pack(pop)
#else
#pragma pack(0)
#endif
