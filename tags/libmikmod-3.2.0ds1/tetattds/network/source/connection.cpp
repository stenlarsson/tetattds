#include "network.h"
#include "connection.h"
#ifdef _WIN32
// SDL.h can be used instead of stdint.h
#include <SDL.h>
#endif

#ifndef ARM9
#include <time.h>
unsigned int GetTime()
{
	return (unsigned int)time(NULL);
}
#endif

Connection::Connection(UdpSocket* socket, IpAddress address, MessageReciever* reciever)
:	socket(socket),
	address(address),
	reciever(reciever),
	incomingSequence(0),
	outgoingSequence(0),
	futurePackets(),
	nonAckedPackets(),
	state(CONNECTIONSTATE_CONNECTED),
	lastSendTime(GetTime()),
	lastRecieveTime(GetTime())
{
	ASSERT(socket != NULL);
	ASSERT(reciever != NULL);
}

Connection::~Connection()
{
}

void Connection::Tick()
{
	unsigned int now = GetTime();

	for(std::vector<CachedPacket>::iterator i = futurePackets.begin();
		i != futurePackets.end();
		i++) {

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
			futurePackets.erase(i);
			i--;
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

void Connection::PacketIn(void* data, size_t size)
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
				// duplicate or out of order packet
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
		// fall through

	case PACKET_TYPE_UNRELIABLE:
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
