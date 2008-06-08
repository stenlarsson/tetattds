#pragma once

#include <stdint.h>
#include <string.h>
#include <vector>
#include "udpsocket.h"
#include "protocol.h"
#include "ipaddress.h"
#include "connection.h"
#include "udpconnection.h"

extern unsigned int GetTime();

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

class UdpConnection : public Connection
{
public:
	UdpConnection(UdpSocket* socket, IpAddress address, MessageReciever* reciever);
	virtual ~UdpConnection();

	void PacketIn(void* data, size_t size);

	virtual void Tick();

private:
	virtual bool SendMessageImpl(
		unsigned char packetType,
		unsigned char messageId,
		const void* message,
		size_t length);
	
	MessageReciever* reciever;
	UdpSocket* socket;
	IpAddress address;
	unsigned int incomingSequence;
	unsigned int outgoingSequence;
	std::vector<CachedPacket> futurePackets;
	std::vector<CachedPacket> nonAckedPackets;
	unsigned int lastSendTime;
	unsigned int lastRecieveTime;
};
