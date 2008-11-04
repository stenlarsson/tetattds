#pragma once

#ifdef GEKKO
#include <network.h>
#include <errno.h>

void SetupWifi()
{
}

enum WIFI_ASSOCSTATUS {
	ASSOCSTATUS_DISCONNECTED = 0,
	ASSOCSTATUS_ACQUIRINGDHCP = 4,
	ASSOCSTATUS_ASSOCIATED = 5,
	ASSOCSTATUS_CANNOTCONNECT = 6
};

WIFI_ASSOCSTATUS Wifi_AssocStatus()
{
	s32 result = net_init();

	if(result == -EAGAIN) {
		return ASSOCSTATUS_ACQUIRINGDHCP;
	} else if(result < 0) {
		return ASSOCSTATUS_CANNOTCONNECT;
	} else {
		return ASSOCSTATUS_ASSOCIATED;
	}
}

unsigned int Wifi_GetIP()
{
	char myIP[16];
	if (if_config(myIP, NULL, NULL, true) < 0)
	{
		printf("Error reading IP address\n");
        return -1;
	}
	return inet_addr(myIP);
}

#else

void SetupWifi() {}

enum WIFI_ASSOCSTATUS {
	ASSOCSTATUS_DISCONNECTED = 0,
	ASSOCSTATUS_ASSOCIATED = 5
};

WIFI_ASSOCSTATUS Wifi_AssocStatus() {
	return ASSOCSTATUS_ASSOCIATED;
}

unsigned int Wifi_GetIP() { return 0; }
#endif
