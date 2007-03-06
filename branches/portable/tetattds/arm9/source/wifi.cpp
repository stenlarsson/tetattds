#include "tetattds.h"
#include "ds.h"
#include "wifi.h"
#include <dswifi9.h>

#include "fifo.h"
#include "util.h"

// wifi timer function, to update internals of sgIP
void Timer_50ms(void) {
	Wifi_Timer(50);
}

// notification function to send fifo message to arm7
void arm9_synctoarm7()
{
	SendFifo(FIFO_SYNC_WIFI);
}

void SetupWifi()
{
	// send fifo message to initialize the arm7 wifi
	u32 Wifi_pass= Wifi_Init(WIFIINIT_OPTION_USELED);
	SendFifo(FIFO_START_WIFI);
	SendFifo(Wifi_pass);

	*((volatile u16 *)0x0400010E) = 0; // disable timer3

	irqSet(IRQ_TIMER3, Timer_50ms); // setup timer IRQ
	irqEnable(IRQ_TIMER3);

	irqSet(IRQ_IPC_SYNC, Wifi_Sync);
	irqEnable(IRQ_IPC_SYNC);
	REG_IPC_SYNC = IPC_SYNC_IRQ_ENABLE;

	Wifi_SetSyncHandler(arm9_synctoarm7); // tell wifi lib to use our handler to notify arm7

	// set timer3
	*((volatile u16 *)0x0400010C) = -6553; // 6553.1 * 256 cycles = ~50ms;
	*((volatile u16 *)0x0400010E) = 0x00C2; // enable, irq, 1/256 clock

	while(Wifi_CheckInit()==0)
	{ // wait for arm7 to be initted successfully
		swiWaitForVBlank();
	}

	// wifi init complete - wifi lib can now be used!
	Wifi_AutoConnect();
}
