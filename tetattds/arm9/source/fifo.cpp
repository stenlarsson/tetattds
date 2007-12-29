#include <nds.h>
#include "tetattds.h"
#include <dswifi9.h>
#include <MessageQueue.h>
#include <802.11.h>
#include <lobby.h>

#include "ds.h"
#include "fifo.h"

static bool localWifi = false;

void FIFOHandler()
{
	if(localWifi) {
		IPC_RcvCompleteCheck();
	} else {
		while(!(REG_IPC_FIFO_CR & IPC_FIFO_RECV_EMPTY))
		{
			u32 command = REG_IPC_FIFO_RX;
			switch(command)
			{
			case FIFO_SYNC_WIFI:
				Wifi_Sync();
				break;
				
			default:
				break;
			}
		}
	}
}

void InitFifo()
{	
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ | IPC_FIFO_SEND_CLEAR;
	irqSet(IRQ_FIFO_NOT_EMPTY, FIFOHandler);
	irqEnable(IRQ_FIFO_NOT_EMPTY);
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
		
		if(data == FIFO_START_LOCAL_WIFI) {
			localWifi = true;
			REG_IPC_FIFO_CR = 0;
			irqDisable(IRQ_FIFO_NOT_EMPTY);
		}
	}
}
