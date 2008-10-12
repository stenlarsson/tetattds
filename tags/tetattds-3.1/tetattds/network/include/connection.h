#pragma once

#include "protocol.h"

#define BEGIN_MESSAGE_HANDLER\
	switch(id) {\

#define HANDLE_MESSAGE(name)\
	case name##Message::messageId:\
		{\
			ASSERT(size == ((name##Message*)data)->size());\
			m##name(from, (name##Message*)data);\
		}\
		break;

#define END_MESSAGE_HANDLER\
	default:\
		printf( "Unhandled message in " __FILE__ "\n" );\
		break;\
	}

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
	virtual void MessageIn(Connection* from, unsigned char id, const void* data, size_t size) = 0;
	virtual void ConnectionRejected(Connection* connection) = 0;
	virtual void ConnectionTimeout(Connection* connection) = 0;
};

class Connection
{
public:
	Connection()
	:	state(CONNECTIONSTATE_CONNECTED)
	{
	}
	
	virtual ~Connection() {}

	virtual void Tick() = 0;

	ConnectionState GetState() { return state; }
	void SetState(ConnectionState state) { this->state = state; }

	void Close() { state = 	CONNECTIONSTATE_CLOSING; }

	template<typename MessageT>
	bool SendMessage(const MessageT& message)
	{
		return SendMessageImpl(
			MessageT::packetType,
			MessageT::messageId,
			&message,
			message.size());
	}

	virtual bool SendMessageImpl(
		unsigned char packetType,
		unsigned char messageId,
		const void* message,
		size_t length) = 0;

protected:
	ConnectionState state;
};
