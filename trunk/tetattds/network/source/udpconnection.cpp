#include "network.h"
#include "udpconnection.h"
#include "packet.h"
#include <string.h>
#ifdef _WIN32
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

#ifndef ARM9
#include <time.h>
unsigned int GetTime()
{
	return (unsigned int)time(NULL);
}
#endif

UdpConnection::UdpConnection(UdpSocket* socket, IpAddress address, MessageReciever* reciever)
:	reciever(reciever),
	socket(socket),
	address(address),
	incomingSequence(0),
	outgoingSequence(0),
	incomingUnreliableSequence(0xffff),
	outgoingUnreliableSequence(0),
	futurePackets(),
	nonAckedPackets(),
	lastSendTime(GetTime()),
	lastRecieveTime(GetTime())
{
	ASSERT(socket != NULL);
	ASSERT(reciever != NULL);
}

UdpConnection::~UdpConnection()
{
}

void UdpConnection::Tick()
{
	unsigned int now = GetTime();

	for(std::vector<CachedPacket>::iterator i = futurePackets.begin();
		i != futurePackets.end();) {

		CachedPacket& cachedPacket = *i;
		if(cachedPacket.sequence == incomingSequence) {
			MessageHeader* header = (MessageHeader*)cachedPacket.packet;
			reciever->MessageIn(
				this,
				header->messageId,
				(char*)cachedPacket.packet + sizeof(MessageHeader),
				cachedPacket.packetsize - sizeof(MessageHeader));
			incomingSequence++;
		}

		if(cachedPacket.sequence < incomingSequence) {
			::operator delete(cachedPacket.packet);
			i = futurePackets.erase(i);
		} else {
			i++;
		}
	}

	for(unsigned int i = 0; i < nonAckedPackets.size(); i++) {
		CachedPacket& cachedPacket = nonAckedPackets[i];
		if(now - cachedPacket.lastSendTime >= RESEND_PACKET_TIMEOUT) {
			socket->Send(cachedPacket.packet, cachedPacket.packetsize, address);
			cachedPacket.lastSendTime = now;
		}
	}

	if(now - lastSendTime >= CONNECTION_KEEPALIVE_TIME) {
		MessageHeader keepalive(PACKET_TYPE_KEEPALIVE, 0, 0);
		socket->Send(&keepalive, sizeof(keepalive), address);
		lastSendTime = now;
	}

	if(now - lastRecieveTime >= CONNECTION_TIMEOUT) {
		state = CONNECTIONSTATE_TIMED_OUT;
	}
	
	if(nonAckedPackets.size() == 0 && state == CONNECTIONSTATE_CLOSING)
	{
		state = CONNECTIONSTATE_CLOSED;
	}
}

void UdpConnection::PacketIn(void* data, size_t size)
{
	ASSERT(data != NULL);
	ASSERT(size > 0);
	
	MessageHeader* header = (MessageHeader*)data;
	switch(header->packetType) {
	case PACKET_TYPE_ACK:
		for(std::vector<CachedPacket>::iterator i = nonAckedPackets.begin();
			i != nonAckedPackets.end();
			i++) {

			CachedPacket& cachedPacket = *i;
			if(cachedPacket.sequence == header->sequence) {
				::operator delete(cachedPacket.packet);
				nonAckedPackets.erase(i);
				break;
			}
		}
		break;

	case PACKET_TYPE_REJECT:
		state = CONNECTIONSTATE_REJECTED;
		break;
	
	case PACKET_TYPE_ORDERED:
		{
			MessageHeader ack(PACKET_TYPE_ACK, 0, header->sequence);
			socket->Send(&ack, sizeof(ack), address);

			if(header->sequence < incomingSequence) {
				// duplicate
				break;
			} else if(header->sequence > incomingSequence) {
				// wrong order
				futurePackets.push_back(CachedPacket(
					(uint32_t)-1,
					header->sequence,
					data,
					size));
				break;
			}
			incomingSequence++;
		}

		// Hate fall through ...
		reciever->MessageIn(
			this,
			header->messageId,
			(char*)data + sizeof(MessageHeader),
			size - sizeof(MessageHeader));
		break;

	case PACKET_TYPE_UNRELIABLE:
		if(incomingUnreliableSequence != (uint16_t)(((uint16_t)header->sequence) - 1)) {
			printf("Previous %i, got %i\n", (uint16_t)incomingUnreliableSequence, (uint16_t)header->sequence);
		}
		incomingUnreliableSequence = wrapping(header->sequence);
	
		reciever->MessageIn(
			this,
			header->messageId,
			(char*)data + sizeof(MessageHeader),
			size - sizeof(MessageHeader));
		break;

	case PACKET_TYPE_KEEPALIVE:
		break;
	}

	lastRecieveTime = GetTime();
}

bool UdpConnection::SendMessageImpl(
	unsigned char packetType,
	unsigned char messageId,
	const void* message,
	size_t length)
{
	if(length > MAX_PACKET_SIZE) {
		return false;
	}

	Packet packet(
		packetType,
		messageId,
		(packetType == PACKET_TYPE_UNRELIABLE) ? outgoingUnreliableSequence : outgoingSequence,
		message,
		length);

	if(!socket->Send(&packet, sizeof(MessageHeader) + length, address)) {
		return false;
	}

	switch(packetType) {
	case PACKET_TYPE_UNRELIABLE:
		outgoingUnreliableSequence++;
		break;

	case PACKET_TYPE_ORDERED:
		nonAckedPackets.push_back(CachedPacket(
			GetTime(),
			packet.header.sequence,
			&packet,
			sizeof(MessageHeader) + length));
		outgoingSequence++;
		break;
	}

	lastSendTime = GetTime();
	return true;
}
