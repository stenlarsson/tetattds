#pragma once

void SetupWifi() {};

enum WIFI_ASSOCSTATUS {
	ASSOCSTATUS_DISCONNECTED = 0,
	ASSOCSTATUS_ASSOCIATED = 5
};

WIFI_ASSOCSTATUS Wifi_AssocStatus() {
	return ASSOCSTATUS_ASSOCIATED;
}

unsigned int Wifi_GetIP() { return 0; }