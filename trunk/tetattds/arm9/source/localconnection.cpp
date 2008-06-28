#include "tetattds.h"
#include "localconnection.h"
#include "packet.h"

LocalConnection::LocalConnection(LPLOBBY_USER user)
:	user(user)
{
	ASSERT(user != NULL);
}

LocalConnection::~LocalConnection()
{
}

void LocalConnection::Tick()
{
	if(state == CONNECTIONSTATE_CLOSING)
	{
		LOBBY_LeaveRoom();
		state = CONNECTIONSTATE_CLOSED;
	}
}

bool LocalConnection::SendMessageImpl(
	unsigned char packetType,
	unsigned char messageId,
	const void* message,
	size_t length)
{
	if(length > MAX_PACKET_SIZE) {
		return false;
	}

	Packet packet(packetType, messageId, 0, message, length);

	LOBBY_SendToUser(user, 0x0001, (unsigned char*)&packet, sizeof(MessageHeader) + length);
	return true;
}
