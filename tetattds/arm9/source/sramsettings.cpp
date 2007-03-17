#include <nds.h>
#include "tetattds.h"
#include "sramsettings.h"

struct InternalSettings
{
	char names[NUM_MODES][NUM_HIGHSCORES][MAX_NAME_LENGTH];
	int scores[NUM_MODES][NUM_HIGHSCORES];
	char serverAddress[MAX_SERVER_ADDRESS_LENGTH];
};

SramSettings::SramSettings()
{
}

SramSettings::~SramSettings()
{
}

void SramSettings::Load()
{
	// map gba cartridge to arm9
	REG_POWERCNT &= ~0x80;

	u8* ptr = SRAM;
	
	if(MemCompare(ptr, "TETATTDS1", 9) != 0)
	{
		PrintStatus("No saved settings found.\n");
		return;
	}
	ptr += 9;
	
	MemCopy(settings, ptr, sizeof(InternalSettings));
}

void SramSettings::Save()
{
	// map gba cartridge to arm9
	REG_POWERCNT &= ~0x80;

	u8* ptr = SRAM;
	
	MemCopy(ptr, "TETATTDS1", 9);
	ptr += 9;
	
	MemCopy(ptr, settings, sizeof(InternalSettings));
	ptr += sizeof(InternalSettings);
}

/**
 * A memcpy that copies one byte at a time.
 */
void SramSettings::MemCopy(void* dest, const void* src, size_t size)
{
	u8* destPtr = (u8*)dest;
	u8* srcPtr = (u8*)src;
	for(size_t i = 0; i < size; i++)
	{
		*destPtr++ = *srcPtr++;
	}
}

/**
 * A memcmp that compares one byte at a time.
 */
int SramSettings::MemCompare(void* dest, const void* src, size_t size)
{
	u8* destPtr = (u8*)dest;
	u8* srcPtr = (u8*)src;
	for(size_t i = 0; i < size; i++)
	{
		if(*destPtr > *srcPtr)
		{
			return 1;
		}
		else if(*destPtr < *srcPtr)
		{
			return -1;
		}

		destPtr++;
		srcPtr++;
	}

	return 0;
}

/**
 * A memset that sets one byte to zero at a time.
 */
void SramSettings::ZeroMemory(void* dest, size_t size)
{
	u8* destPtr = (u8*)dest;
	for(size_t i = 0; i < size; i++)
	{
		*destPtr++ = 0;
	}
}
