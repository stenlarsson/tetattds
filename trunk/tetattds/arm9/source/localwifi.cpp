#include "tetattds.h"
#include <nds.h>
#include <MessageQueue.h>
#include <802.11.h>
#include <lobby.h>
#include "localwifi.h"
#include "localconnectionmanager.h"
#include "fifo.h"
#include "ds.h"

static void PacketCallback(unsigned char *data, int length, LPLOBBY_USER from)
{
	if(g_localConnectionManager != NULL) {
		g_localConnectionManager->OnPacket(data, length, from);
	}
}

static void UserInfoCallback(LPLOBBY_USER user, unsigned long reason)
{
	if(g_localConnectionManager != NULL) {
		g_localConnectionManager->OnUserInfo(user, reason);
	}
}

void SetupLocalWifi()
{
	SendFifo(FIFO_START_LOCAL_WIFI);
	swiWaitForVBlank();
	IPC_Init();
	IPC_SetChannelCallback(0, &LWIFI_IPC_Callback);
	
	LOBBY_Init();
	LOBBY_SetStreamHandler(0x0001, &PacketCallback);
	LOBBY_SetStreamHandler(0x8001, &PacketCallback);
	LOBBY_SetUserInfoCallback(&UserInfoCallback);
	
	for(int i = 0; i < 60; i++) {
		LOBBY_Update();
		swiWaitForVBlank();
	}
}
