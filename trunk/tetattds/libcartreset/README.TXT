/**********************************
  Copyright (C) Rick Wong (Lick)
  http://licklick.wordpress.com/
   rickinhethuis @ gmail . com
***********************************/


/------------------------------------
Q: How do I INSTALL libcartreset?
------------------------------------/
1)  Put /include/cartreset.h in /DevKitPro/libnds/include/.
2)  Put /lib/libcartreset.a in /DevKitPro/libnds/lib/.
3)  Link to -lcartreset7/9.a before -lfat and -lnds7/9.
    Example: "-lcartreset7.a -lfat -lnds7"
             "-lcartreset9.a -lfat -lnds9"         

/------------------------------------
Q: How do I use the code?
------------------------------------/
1) Include "cartreset.h" where needed.

2) Example on ARM9 side.

    if(keysUp() & KEY_START)
    {
        if(cartSetMenuMode(DEVICE_TYPE_AUTO))       // Prepare the Slot2 device for reset
            passmeloopEnter();      // Enter PassMe loop.
    }
   
3) Example on the ARM7 side.

    while(1)
    {
        if(passmeloopQuery())       // If ARM9 has enter the PassMe loop..
            cartExecute();          // .. reset!
        swiWaitForVBlank();
    }