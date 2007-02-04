/**********************************
  Copyright (C) Rick Wong (Lick)
  http://licklick.wordpress.com/
***********************************/
#ifndef CARTRESET_H
#define CARTRESET_H

#include <fat.h>


#ifdef __cplusplus
extern "C" {
#endif

#ifdef ARM9
// Auto detect:
#define DEVICE_TYPE_AUTO        0x00000000 // will not work in "nolibfat" version of libcartreset

// Not supported:
#define DEVICE_TYPE_FCSR        0x52534346
#define DEVICE_TYPE_MMCF        0x46434D4D
#define DEVICE_TYPE_NJSD        0x44534A4E
#define DEVICE_TYPE_NMMC        0x434D4D4E

// Supported:
#define DEVICE_TYPE_EFA2        0x32414645
#define DEVICE_TYPE_MPCF        0x4643504D
#define DEVICE_TYPE_M3CF        0x4643334D
#define DEVICE_TYPE_M3SD        0x4453334D
#define DEVICE_TYPE_SCCF        0x46434353
#define DEVICE_TYPE_SCSD        0x44534353

// Supported, but libfat doesn't detect the device:
#define DEVICE_TYPE_EZSD        0x44535A45


/*-----------------------------------------
     BELOW CODE IS TEMPORARY.
          WORKS WITH DEVKITPRO R19  ONLY.
        THANKS TO JOSATH aka DAVR
-----------------------------------------*/
typedef bool (* FN_MEDIUM_STARTUP)(void) ;
typedef bool (* FN_MEDIUM_ISINSERTED)(void) ;
typedef bool (* FN_MEDIUM_READSECTORS)(u32 sector, u32 numSectors, void* buffer) ;
typedef bool (* FN_MEDIUM_WRITESECTORS)(u32 sector, u32 numSectors, const void* buffer) ;
typedef bool (* FN_MEDIUM_CLEARSTATUS)(void) ;
typedef bool (* FN_MEDIUM_SHUTDOWN)(void) ;
struct IO_INTERFACE_STRUCT {
    unsigned long			ioType ;
    unsigned long			features ;
    FN_MEDIUM_STARTUP		fn_startup ;
    FN_MEDIUM_ISINSERTED	fn_isInserted ;
    FN_MEDIUM_READSECTORS	fn_readSectors ;
    FN_MEDIUM_WRITESECTORS	fn_writeSectors ;
    FN_MEDIUM_CLEARSTATUS	fn_clearStatus ;
    FN_MEDIUM_SHUTDOWN		fn_shutdown ;
} ;
typedef struct IO_INTERFACE_STRUCT IO_INTERFACE ;
typedef struct {
    u32 sector;
    u32 count;
    bool dirty;
} CACHE_ENTRY;
typedef struct {
    const IO_INTERFACE* disc;
    u32 numberOfPages;
    CACHE_ENTRY* cacheEntries;
    u8* pages;
} CACHE;
typedef enum {FS_UNKNOWN, FS_FAT12, FS_FAT16, FS_FAT32} FS_TYPE;
typedef struct {
    u32 fatStart;
    u32 sectorsPerFat;
    u32 lastCluster;
    u32 firstFree;
} FAT;
typedef struct {
    const IO_INTERFACE* disc;
    CACHE* cache;
    bool readOnly;	
    FS_TYPE filesysType;
    u32 totalSize;
    u32 rootDirStart;
    u32 rootDirCluster;
    u32 numberOfSectors;
    u32 dataStart;
    u32 bytesPerSector;
    u32 sectorsPerCluster;
    u32 bytesPerCluster;
    FAT fat;
    u32 cwdCluster;	
    u32 openFileCount;
} PARTITION;

extern PARTITION *_partitions[];
/*-----------------------------------------
     ABOVE CODE IS TEMPORARY.
          WORKS WITH DEVKITPRO R19  ONLY.
        THANKS TO JOSATH aka DAVR
-----------------------------------------*/

bool cartSupportReset(u32 _deviceType);
bool cartSetMenuMode(u32 _deviceType);
void passmeloopEnter();

#endif


#ifdef ARM7

bool passmeloopQuery();
void cartExecute();

#endif


#ifdef __cplusplus
}
#endif

#endif 

