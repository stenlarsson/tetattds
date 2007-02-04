/**********************************
  Copyright (C) Rick Wong (Lick)
  http://licklick.wordpress.com/
***********************************/
#include <cartreset.h>


#ifdef ARM9

u32 cartGetType()
{
	/*-----------------------------------------
		 BELOW CODE IS TEMPORARY.
			  WORKS WITH DEVKITPRO R19 ONLY.
			THANKS TO JOSATH aka DAVR
	-----------------------------------------*/
	u32 deviceType = 0;
	u32 i;
	for(i=0; i<4; i++) 
	{
		if(_partitions[i])
		{
			if(_partitions[i]->disc->ioType == DEVICE_TYPE_FCSR)
				deviceType = DEVICE_TYPE_FCSR;

			else if(_partitions[i]->disc->ioType == DEVICE_TYPE_EFA2)
				deviceType = DEVICE_TYPE_EFA2;

			else if(_partitions[i]->disc->ioType == DEVICE_TYPE_MMCF)
				deviceType = DEVICE_TYPE_MMCF;

			else if(_partitions[i]->disc->ioType == DEVICE_TYPE_NJSD)
				deviceType = DEVICE_TYPE_NJSD;

			else if(_partitions[i]->disc->ioType == DEVICE_TYPE_NMMC)
				deviceType = DEVICE_TYPE_NMMC;

			else if(_partitions[i]->disc->ioType == DEVICE_TYPE_MPCF)
				deviceType = DEVICE_TYPE_MPCF;

			else if(_partitions[i]->disc->ioType == DEVICE_TYPE_M3CF)
				deviceType = DEVICE_TYPE_M3CF;

			else if(_partitions[i]->disc->ioType == DEVICE_TYPE_M3SD)
				deviceType = DEVICE_TYPE_M3SD;

			else if(_partitions[i]->disc->ioType == DEVICE_TYPE_SCCF)
				deviceType = DEVICE_TYPE_SCCF;

			else if(_partitions[i]->disc->ioType == DEVICE_TYPE_SCSD)
				deviceType = DEVICE_TYPE_SCSD;

			if(deviceType)
				break;
		}
	}
	/*-----------------------------------------
		 ABOVE CODE IS TEMPORARY.
			  WORKS WITH DEVKITPRO R19  ONLY.
			THANKS TO JOSATH aka DAVR
	-----------------------------------------*/
	return deviceType;
}



bool cartSupportReset(u32 _deviceType)
{
    u32 deviceType = _deviceType;

    if(deviceType == DEVICE_TYPE_AUTO)
    {
		deviceType = cartGetType();
	}
	
    if(deviceType == DEVICE_TYPE_EFA2)
        return true;
    else if(deviceType == DEVICE_TYPE_MPCF)
        return true;
    else if(deviceType == DEVICE_TYPE_EZSD)
        return true;
    else if(deviceType == DEVICE_TYPE_M3CF || deviceType == DEVICE_TYPE_M3SD)
        return true;
    else if(deviceType == DEVICE_TYPE_SCCF || deviceType == DEVICE_TYPE_SCSD)
        return true;

	return false;
}



bool cartSetMenuMode(u32 _deviceType)
{
    *(vu16*)(0x04000204) &= ~0x0880;    //sysSetBusOwners(true, true);
    u32 deviceType = _deviceType;

    if(deviceType == DEVICE_TYPE_AUTO)
    {
		deviceType = cartGetType();
    }

    if(deviceType == DEVICE_TYPE_EFA2)
    {
        *(u16 *)0x9FE0000 = 0xD200;
        *(u16 *)0x8000000 = 0x1500;
        *(u16 *)0x8020000 = 0xD200;
        *(u16 *)0x8040000 = 0x1500;
        *(u16 *)0x9880000 = 1 << 15;
        *(u16 *)0x9FC0000 = 0x1500;
        return true;
    }
    else if(deviceType == DEVICE_TYPE_MPCF)
    {
        return true;
    }
    else if(deviceType == DEVICE_TYPE_EZSD)
    {
        return true;
    }
    else if(deviceType == DEVICE_TYPE_M3CF || deviceType == DEVICE_TYPE_M3SD)
    {
        u32 mode = 0x00400004;
	    vu16 tmp;
        tmp = *(vu16*)(0x08E00002);
        tmp = *(vu16*)(0x0800000E);
        tmp = *(vu16*)(0x08801FFC);
        tmp = *(vu16*)(0x0800104A);
        tmp = *(vu16*)(0x08800612);
        tmp = *(vu16*)(0x08000000);
        tmp = *(vu16*)(0x08801B66);
        tmp = *(vu16*)(0x08000000 + (mode << 1)); 
        tmp = *(vu16*)(0x0800080E);
        tmp = *(vu16*)(0x08000000);

        tmp = *(vu16*)(0x080001E4);
        tmp = *(vu16*)(0x080001E4);
        tmp = *(vu16*)(0x08000188);
        tmp = *(vu16*)(0x08000188);
        return true;
    }
    else if(deviceType == DEVICE_TYPE_SCCF || deviceType == DEVICE_TYPE_SCSD)
    {
        *(vu16*)0x09FFFFFE = 0xA55A;
        *(vu16*)0x09FFFFFE = 0xA55A;
        *(vu16*)0x09FFFFFE = 0;
        *(vu16*)0x09FFFFFE = 0;
        return true;
    }

    return false;
}



void passmeloopEnter()
{
    *(vu16*)(0x04000208) = 0;       //REG_IME = IME_DISABLE;
    *(vu16*)(0x04000204) |= 0x0880; //sysSetBusOwners(false, false);
    *((vu32*)0x027FFFFC) = 0;
    *((vu32*)0x027FFE04) = (u32)0xE59FF018;
    *((vu32*)0x027FFE24) = (u32)0x027FFE04;
    asm("swi 0x00");                //swiSoftReset();
    asm("bx lr");
}

#endif


#ifdef ARM7

bool passmeloopQuery()
{
    if(*((vu32*)0x027FFE24) == (u32)0x027FFE04)
        return true;
    return false;
}



void cartExecute()
{
    *(vu16*)(0x04000208) = 0;       //REG_IME = IME_DISABLE;
    *((vu32*)0x027FFE34) = (u32)0x080000C0;
    asm("swi 0x00");                //swiSoftReset();
    asm("bx lr");
}

#endif


