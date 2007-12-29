#include "network.h"
#include "loopbackconnection.h"

LoopbackConnection::LoopbackConnection(MessageReciever* reciever)
:	reciever(reciever),
	otherEnd(NULL)
{
}

LoopbackConnection::~LoopbackConnection()
{
}

void LoopbackConnection::Tick()
{
	if(state == CONNECTIONSTATE_CLOSING)
	{
		state = CONNECTIONSTATE_CLOSED;
	}
}

void LoopbackConnection::Connect(LoopbackConnection* otherEnd)
{
	this->otherEnd = otherEnd;
	otherEnd->otherEnd = this;
	
	reciever->ConnectionCreated(this);
	otherEnd->reciever->ConnectionCreated(otherEnd);
}

bool LoopbackConnection::SendMessageImpl(
	unsigned char packetType,
	unsigned char messageId,
	const void* message,
	size_t length)
{
	if(otherEnd == NULL) {
		return false;
	}
	
	otherEnd->reciever->MessageIn(
		otherEnd,
		messageId,
		message,
		length);
	return true;
}
