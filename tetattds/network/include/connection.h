#pragma once

#include <vector>
#include "udpsocket.h"
#include "protocol.h"
#include "ipaddress.h"

extern unsigned int GetTime();

#define BEGIN_MESSAGE_HANDLER\
	switch(id) {\

#define HANDLE_MESSAGE(name)\
	case name##Message::messageId:\
		{\
			ASSERT(size >= sizeof(name##Message));\
			m##name(from, (name##Message*)data);\
		}\
		break;

#define END_MESSAGE_HANDLER\
	default:\
		printf( "Unhandled message in " __FILE__ "\n" );\
		break;\
	}

struct CachedPacket
{
	CachedPacket(unsigned int lastSendTime, unsigned int sequence, void* packet, size_t packetsize)
	:	lastSendTime(lastSendTime),
		sequence(sequence),
		packet(::operator new(packetsize)),
		packetsize(packetsize)
	{
		memcpy(this->packet, packet, packetsize);
	}

	~CachedPacket()
	{
		//::operator delete(packet);
	}
		
	unsigned int lastSendTime;
	unsigned int sequence;
	void* packet;
	size_t packetsize;
};

enum ConnectionState
{
	CONNECTIONSTATE_CLOSED,
	CONNECTIONSTATE_CLOSING,
	CONNECTIONSTATE_CONNECTED,
	CONNECTIONSTATE_REJECTED,
	CONNECTIONSTATE_TIMED_OUT
};

class Connection;

class MessageReciever
{
public:
	MessageReciever() {}
	virtual ~MessageReciever() {}
	virtual void ConnectionCreated(Connection* connection) = 0;
	virtual void MessageIn(Connection* from, unsigned char id, void* data, size_t size) = 0;
	virtual void ConnectionRejected(Connection* connection) = 0;
	virtual void ConnectionTimeout(Connection* connection) = 0;
};

class Connection
{
public:
	Connection(UdpSocket* socket, IpAddress address, MessageReciever* reciever);
	~Connection();

	void Tick();

	void PacketIn(void* data, size_t size);

	ConnectionState GetState() { return state; }

	void Close() { state = 	CONNECTIONSTATE_CLOSING; }

	template<typename MessageT>
	bool SendMessage(const MessageT& message)
	{
		struct Packet {
			Packet(
				unsigned char packetType,
				unsigned char messageId,
				unsigned int sequence,
				const MessageT& message)
			:	header(packetType, messageId, sequence),
				message(message)
			{
			}

			MessageHeader header;
			MessageT message;
		};

		Packet packet(MessageT::packetType, MessageT::messageId, outgoingSequence, message);

		if(!socket->Send(&packet, sizeof(packet), address)) {
			return false;
		}

		switch(MessageT::packetType) {
		case PACKET_TYPE_UNRELIABLE:
			break;

		case PACKET_TYPE_ORDERED:
			nonAckedPackets.push_back(CachedPacket(
				GetTime(),
				packet.header.sequence,
				&packet,
				sizeof(Packet)));
			outgoingSequence++;
			break;
		}

		lastSendTime = GetTime();
		return true;
	}

private:
	UdpSocket* socket;
	IpAddress address;
	MessageReciever* reciever;
	unsigned int incomingSequence;
	unsigned int outgoingSequence;
	std::vector<CachedPacket> futurePackets;
	std::vector<CachedPacket> nonAckedPackets;
	ConnectionState state;
	unsigned int lastSendTime;
	unsigned int lastRecieveTime;
};
