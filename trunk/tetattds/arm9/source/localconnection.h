#pragma once

#include <lobby.h>

#include "protocol.h"
#include "connection.h"

class LocalConnection : public Connection
{
public:
	LocalConnection(LPLOBBY_USER user);
	virtual ~LocalConnection();

	virtual void Tick();

private:
	virtual bool SendMessageImpl(
		unsigned char packetType,
		unsigned char messageId,
		const void* message,
		size_t length);

	LPLOBBY_USER user;
};
