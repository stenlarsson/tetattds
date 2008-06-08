#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include "udpsocket.h"
#include "udpconnectionmanager.h"
#include "servergame.h"

void PrintStatus(const char* format, ...)
{
	va_list args;
	va_start( args, format );
	
	vprintf( format, args );
	printf("\n");

	va_end( args );
}

int main( int argc, char **argv )
{
	UdpSocket::InitSockets();

	UdpSocket* socket = new UdpSocket();
	if(!socket->Bind(13687)) {
		return 1;
	}

	ServerGame* game = new ServerGame();
	UdpConnectionManager* connectionManager = new UdpConnectionManager(MAX_PLAYERS, socket, game);

	// todo: need a way to quit
	while( true )
	{
		connectionManager->Tick();
		game->Tick();
		socket->Wait(0, 16666);
	}

	delete connectionManager;
	delete game;

	return 0;
}
