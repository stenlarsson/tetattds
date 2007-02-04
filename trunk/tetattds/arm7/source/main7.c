#include <nds.h>

#include <nds/bios.h>
#include <nds/arm7/touch.h>
#include <nds/arm7/clock.h>

#include <dswifi7.h>

#include <mikmod7.h>

#include <cartreset.h>

#include "../../arm9/source/constants.h"

void SetupWifi();

void FIFOHandler()
{
	while(!(REG_IPC_FIFO_CR & IPC_FIFO_RECV_EMPTY))
	{
		u32 command = REG_IPC_FIFO_RX;
		switch(command)
		{
		case FIFO_START_WIFI:
			SetupWifi();
			break;
			
		case FIFO_SYNC_WIFI:
			Wifi_Sync();
			break;
			
		default:
			MikMod7_ProcessCommand(command);
			break;
		}
	}
}

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
 
 
void irqVblank()
{
	static int heartbeat = 0;
 
	uint16 but=0, x=0, y=0, xpx=0, ypx=0, z1=0, z2=0, batt=0, aux=0;
	int t1=0, t2=0;
	uint32 temp=0;
	uint8 ct[sizeof(IPC->curtime)];
 
	// Update the heartbeat
	heartbeat++;
 
	// Read the touch screen
 
	but = REG_KEYXY;
 
	if (!(but & (1<<6))) {
 
		touchPosition tempPos = touchReadXY();

		x = tempPos.x;
		y = tempPos.y;
		xpx = tempPos.px;
		ypx = tempPos.py;
	}
 
	z1 = touchRead(TSC_MEASURE_Z1);
	z2 = touchRead(TSC_MEASURE_Z2);
 
 
	batt = touchRead(TSC_MEASURE_BATTERY);
	aux  = touchRead(TSC_MEASURE_AUX);
 
	// Read the time
	rtcGetTime((uint8 *)ct);
	BCDToInteger((uint8 *)&(ct[1]), 7);
 
	// Read the temperature
	temp = touchReadTemperature(&t1, &t2);
 
	IPC->mailBusy = 1;
	// Update the IPC struct
	IPC->heartbeat	= heartbeat;
	IPC->buttons		= but;
	IPC->touchX			= x;
	IPC->touchY			= y;
	IPC->touchXpx		= xpx;
	IPC->touchYpx		= ypx;
	IPC->touchZ1		= z1;
	IPC->touchZ2		= z2;
	IPC->battery		= batt;
	IPC->aux			= aux;
	IPC->mailBusy = 0;
 
	{
		int i;
		for(i=0; i<sizeof(ct); i++)
			IPC->curtime[i] = ct[i];
	}

	IPC->temperature = temp;
	IPC->tdiode1 = t1;
	IPC->tdiode2 = t2;
 

	//sound code  :)
	TransferSound *snd = IPC->soundData;
	IPC->soundData = 0;
 
	if(snd != 0)
	{
		int i;
		for(i=0; i<snd->count; i++)
		{
			s32 chan = getFreeSoundChannel();
 			if (chan >= 0)
			{
				startSound(snd->data[i].rate, snd->data[i].data, snd->data[i].len, chan, snd->data[i].vol, snd->data[i].pan, snd->data[i].format);
			}
		}
	}
 
	Wifi_Update();
}

void SendFifo(u32 data)
{
    while (REG_IPC_FIFO_CR & IPC_FIFO_SEND_FULL);
    if (REG_IPC_FIFO_CR & IPC_FIFO_ERROR)
    {
        REG_IPC_FIFO_CR |= IPC_FIFO_SEND_CLEAR;
    } 
    
    REG_IPC_FIFO_TX = data;
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
}

int main(int argc, char ** argv)
{
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ | IPC_FIFO_SEND_CLEAR;
	IPC->mailData=0;
	IPC->mailSize=0;

	// reset the clock if needed
	rtcReset();
 
	// enable sound
	powerON(POWER_SOUND);
	SOUND_CR = SOUND_ENABLE | SOUND_VOL(0x7f);
	IPC->soundData = 0;
	IPC->mailBusy = 0;

	// setup irq
	irqInit();

	irqSet(IRQ_VBLANK, irqVblank);
	irqEnable(IRQ_VBLANK);

	// we would like to react to fifo irq but that is unreliable
	// since interrupts are sometimes disabled in wifi lib
	//irqSet(IRQ_FIFO_NOT_EMPTY, FIFOHandler);
	//irqEnable(IRQ_FIFO_NOT_EMPTY);
	
	// keep the ARM7 out of main RAM
	while(true)
	{
		swiWaitForVBlank();
		
		if(passmeloopQuery())	// libcartreset
			cartExecute();
		
		FIFOHandler();
	}
}
