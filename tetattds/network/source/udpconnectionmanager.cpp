#include "network.h"
#include "udpconnectionmanager.h"

struct ConnectionInfo
{
	IpAddress address;
	UdpConnection* connection;
};

UdpConnectionManager::UdpConnectionManager(int maxConnections, UdpSocket* socket, MessageReciever* reciever)
:	maxConnections(maxConnections),
	numConnections(0),
	connections(new ConnectionInfo[maxConnections]),
	socket(socket),
	reciever(reciever)
{
	ASSERT(socket != NULL);
	ASSERT(reciever != NULL);
}

UdpConnectionManager::~UdpConnectionManager()
{
	delete socket;
	delete connections;
}

UdpConnection* UdpConnectionManager::CreateConnection(const char* address, int port)
{
	ASSERT(address != NULL);
	IpAddress ipAddress;

	if(!socket->GetAddress(address, port, &ipAddress)) {
		return NULL;
	}

	return CreateConnection(ipAddress);
}

void UdpConnectionManager::Tick()
{
	IpAddress fromAddress;
	char buffer[1024];
	
	while(true) {
		int size = socket->Recieve(buffer, sizeof(buffer), &fromAddress);
		if(size < 0) { break; }

		UdpConnection* connection = FindConnection(fromAddress);
		if(connection == NULL) {
			MessageHeader* header = (MessageHeader*)buffer;
			if(header->sequence != 0) {
				// stray packet?
				continue;
			} else if(numConnections == maxConnections) {
				printf("Connection from ");
				fromAddress.Print();
				printf(" rejected.\n");
				MessageHeader reject(PACKET_TYPE_REJECT, 0, 0);
				socket->Send(&reject, sizeof(reject), fromAddress);
				continue;
			}

			connection = CreateConnection(fromAddress);
		}

		connection->PacketIn(buffer, size);
	}

	for(int i = 0; i < numConnections; i++) {
		UdpConnection* connection = connections[i].connection;
		connection->Tick();
		ConnectionState state = connection->GetState();
		if(state != CONNECTIONSTATE_CONNECTED) {
			if(state == CONNECTIONSTATE_TIMED_OUT) {
				reciever->ConnectionTimeout(connection);
			} else if(state == CONNECTIONSTATE_REJECTED) {
				reciever->ConnectionRejected(connection);
			}
			if(state != CONNECTIONSTATE_CLOSING)
			{
				DeleteConnection(i);
				i--;
			}
		}
	}
}

UdpConnection* UdpConnectionManager::CreateConnection(IpAddress address)
{
	ASSERT(numConnections != maxConnections);

	UdpConnection* connection = new UdpConnection(socket, address, reciever);
	
	connections[numConnections].address = address;
	connections[numConnections].connection = connection;
	numConnections++;
	
	reciever->ConnectionCreated(connection);
	return connection;
}

void UdpConnectionManager::DeleteConnection(int index)
{
	ASSERT(index >= 0);
	ASSERT(index < numConnections);

	delete connections[index].connection;
	connections[index].connection = NULL;

	numConnections--;
	for(int i = index; i < numConnections; i++) {
		connections[i] = connections[i+1];
	}
}

UdpConnection* UdpConnectionManager::FindConnection(IpAddress address)
{
	for(int i = 0; i < numConnections; i++) {
		if(connections[i].address == address) {
			return connections[i].connection;
		}
	}

	return NULL;
}
