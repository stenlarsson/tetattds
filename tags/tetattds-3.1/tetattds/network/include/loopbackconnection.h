#pragma once

#include "protocol.h"
#include "connection.h"

class LoopbackConnection : public Connection
{
public:
	LoopbackConnection(MessageReciever* reciever);
	virtual ~LoopbackConnection();

	virtual void Tick();
	
	void Connect(LoopbackConnection* otherEnd);

private:
	virtual bool SendMessageImpl(
		unsigned char packetType,
		unsigned char messageId,
		const void* message,
		size_t length);

	MessageReciever* reciever;
	LoopbackConnection* otherEnd;
	Connection* broadcastConnection;
};
