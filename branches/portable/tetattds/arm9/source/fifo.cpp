#include "tetattds.h"
#include "fifo.h"
#include <dswifi9.h>

static void FIFOHandler()
{
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

void InitFifo()
{	
	REG_IPC_FIFO_CR = IPC_FIFO_ENABLE | IPC_FIFO_RECV_IRQ | IPC_FIFO_SEND_CLEAR;
	irqSet(IRQ_FIFO_NOT_EMPTY, FIFOHandler);
	irqEnable(IRQ_FIFO_NOT_EMPTY);
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

