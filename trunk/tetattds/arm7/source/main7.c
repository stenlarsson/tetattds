#include <nds.h>

#include <nds/bios.h>
#include <nds/arm7/touch.h>
#include <nds/arm7/clock.h>

#include <dswifi7.h>

#include <mikmod7.h>

#include <smi_startup_viaipc.h>
#include <MessageQueue.h>
#include <wifi_hal.h>

#include "../../arm9/source/ds.h"

BOOL wifi = FALSE;
BOOL localWifi = FALSE;

void startSound(int sampleRate, const void* data, uint32 bytes, u8 channel, u8 vol,  u8 pan, u8 format)
{
	SCHANNEL_TIMER(channel)  = SOUND_FREQ(sampleRate);
	SCHANNEL_SOURCE(channel) = (u32)data;
	SCHANNEL_LENGTH(channel) = bytes >> 2;
	u32 form = 0;
	switch(format)
	{
	case 0:
		form = SOUND_FORMAT_16BIT;
		break;
	case 1:
		form = SOUND_FORMAT_8BIT;
		break;
	case 2:
		form = SOUND_FORMAT_ADPCM;
		break;
	}
	SCHANNEL_CR(channel) = SCHANNEL_ENABLE | SOUND_ONE_SHOT | SOUND_VOL(vol) | SOUND_PAN(pan) | form;
}
 
 
s32 getFreeSoundChannel()
{
	int i;
	for(i=1; i<16; i++)	// channel 0 is reserved for music
	{
		if ( (SCHANNEL_CR(i) & SCHANNEL_ENABLE) == 0 ) return i;
	}
	return -1;
}
 
touchPosition first,tempPos;

//---------------------------------------------------------------------------------
void VcountHandler() {
//---------------------------------------------------------------------------------
	static int lastbut = -1;
	
	uint16 but=0, x=0, y=0, xpx=0, ypx=0, z1=0, z2=0;

	but = REG_KEYXY;

	if (!( (but ^ lastbut) & (1<<6))) {
 
		tempPos = touchReadXY();

		if ( tempPos.x == 0 || tempPos.y == 0 ) {
			but |= (1 <<6);
			lastbut = but;
		} else {
			x = tempPos.x;
			y = tempPos.y;
			xpx = tempPos.px;
			ypx = tempPos.py;
			z1 = tempPos.z1;
			z2 = tempPos.z2;
		}
		
	} else {
		lastbut = but;
		but |= (1 <<6);
	}

	IPC->touchX			= x;
	IPC->touchY			= y;
	IPC->touchXpx		= xpx;
	IPC->touchYpx		= ypx;
	IPC->touchZ1		= z1;
	IPC->touchZ2		= z2;
	IPC->buttons		= but;

}

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------

	u32 i;

	//sound code  :)
	TransferSound *snd = IPC->soundData;
	IPC->soundData = 0;

	if (0 != snd) {

		for (i=0; i<snd->count; i++) {
			s32 chan = getFreeSoundChannel();

			if (chan >= 0) {
				startSound(snd->data[i].rate, snd->data[i].data, snd->data[i].len, chan, snd->data[i].vol, snd->data[i].pan, snd->data[i].format);
			}
		}
	}
}

void SendFifo(u32 data)
{
	if(localWifi) {
		IPC_SendMessage((char*)&data, 4);
	} else {
		while (REG_IPC_FIFO_CR & IPC_FIFO_SEND_FULL);
		if (REG_IPC_FIFO_CR & IPC_FIFO_ERROR)
		{
			REG_IPC_FIFO_CR |= IPC_FIFO_SEND_CLEAR;
		} 
		
		REG_IPC_FIFO_TX = data;
	}
}

// callback to allow wifi library to notify arm9
void arm7_synctoarm9()
{
	SendFifo(FIFO_SYNC_WIFI);
}

void SetupWifi()
{
	irqSet(IRQ_WIFI, (void(*)())Wifi_Interrupt); // set up wifi interrupt
	irqEnable(IRQ_WIFI);

	// sync with arm9 and init wifi
	u32 fifo_temp;   

	while(REG_IPC_FIFO_CR&IPC_FIFO_RECV_EMPTY) swiWaitForVBlank();
	fifo_temp=REG_IPC_FIFO_RX; // give next value to wifi_init
	Wifi_Init(fifo_temp);

	irqSet(IRQ_IPC_SYNC, Wifi_Sync);
	irqEnable(IRQ_IPC_SYNC);
	REG_IPC_SYNC = IPC_SYNC_IRQ_ENABLE;

	Wifi_SetSyncHandler(arm7_synctoarm9); // allow wifi lib to notify arm9
	// arm7 wifi init complete
	
	wifi = TRUE;
}

static void OnFifoCommand(u32 command);

static void FifoCallback(unsigned char *data, int length)
{
	OnFifoCommand(*(u32*)data);
}

static void SetupLocalWifi()
{
	IPC_Init();
	IPC_SetCustomCallback(FifoCallback);
	LWIFI_Init();
	
	localWifi = TRUE;
}

void FIFOHandler()
{

	while(!(REG_IPC_FIFO_CR & IPC_FIFO_RECV_EMPTY))
	{
		if(localWifi) {
//SCHANNEL_TIMER(10) = SOUND_FREQ( 277.183*8);
//SCHANNEL_CR(10) = SCHANNEL_ENABLE | SOUND_FORMAT_PSG | SOUND_PAN(4 * 15 + 2)  | (127 / 48) << 24 | 63;
		}
		OnFifoCommand(REG_IPC_FIFO_RX);
		if(localWifi) {
			return;
		}
	}
}

static void OnFifoCommand(u32 command)
{
	switch(command)
	{
	case FIFO_START_WIFI:
		SetupWifi();
		break;
		
	case FIFO_SYNC_WIFI:
		Wifi_Sync();
		break;
	
	case FIFO_START_LOCAL_WIFI:
		SetupLocalWifi();
		break;
		
	default:
		MikMod7_ProcessCommand(command);
		break;
	}
}

int main(int argc, char ** argv)
{
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ | IPC_FIFO_SEND_CLEAR;

	// read User Settings from firmware
	readUserSettings();

	//enable sound
	powerON(POWER_SOUND);
	writePowerManagement(PM_CONTROL_REG, ( readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE ) | PM_SOUND_AMP );
	SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7F);

	irqInit();

	// Start the RTC tracking IRQ
	initClockIRQ();

	SetYtrigger(80);
	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);

	irqEnable( IRQ_VBLANK | IRQ_VCOUNT);


	while(true)
	{		
		//swiWaitForVBlank();
		if(wifi) {
			Wifi_Update();
		}
		if(localWifi) {
			LWIFI_IPC_UpdateNOIRQ();
		} else {
			FIFOHandler();
		}
	}
}
