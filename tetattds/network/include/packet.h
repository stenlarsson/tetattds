#pragma once

#include "protocol.h"

struct Packet {
	Packet(
		unsigned char packetType,
		unsigned char messageId,
		uint16_t sequence,
		const void* message,
		size_t length)
		:	header(packetType, messageId, sequence)
	{
		memcpy(this->message, message, length);
	}
	
	MessageHeader header;
	char message[MAX_PACKET_SIZE];
};
