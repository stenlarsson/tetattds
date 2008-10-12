#pragma once

#include "connection.h"

class ConnectionManager
{
public:
	ConnectionManager()
	:	broadcastExtraConnection(NULL)
	{}
	virtual ~ConnectionManager() {}

	virtual void Tick() = 0;

	void SetBroadcastExtraConnection(Connection* connection)
	{
		broadcastExtraConnection = connection;
	}

	template<typename MessageT>
	void BroadcastMessage(const MessageT& message)
	{
		BroadcastMessageImpl(
			MessageT::packetType,
			MessageT::messageId,
			&message,
			message.size());
	}

	virtual void BroadcastMessageImpl(
		unsigned char packetType,
		unsigned char messageId,
		const void* message,
		size_t length) = 0;

protected:
	Connection* broadcastExtraConnection;
};
