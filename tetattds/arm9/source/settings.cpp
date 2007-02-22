#include "tetattds.h"
#include "settings.h"

struct InternalSettings
{
	char names[NUM_MODES][NUM_HIGHSCORES][MAX_NAME_LENGTH];
	int scores[NUM_MODES][NUM_HIGHSCORES];
	char serverAddress[MAX_SERVER_ADDRESS_LENGTH];
};

/* Settings *****************************************************************/

Settings::Settings()
:	settings(new InternalSettings)
{
	memset(settings, 0, sizeof(InternalSettings));
	strcpy(settings->serverAddress, "wallbraker.ath.cx");
}

Settings::~Settings()
{
	delete settings;
}

void Settings::GetHighscore(int mode, int level, const char* & name, int & score)
{
	name = settings->names[mode][level];
	score = settings->scores[mode][level];
}

void Settings::UpdateHighscore(int mode, int level, char* name, int score)
{
	if(score > settings->scores[mode][level])
	{
		strncpy(settings->names[mode][level], name, MAX_NAME_LENGTH);
		settings->scores[mode][level] = score;
	}
}

const char* Settings::GetServerAddress()
{
	return settings->serverAddress;
}

void Settings::SetServerAddress(const char* address)
{
	strncpy(settings->serverAddress, address, MAX_SERVER_ADDRESS_LENGTH);
}


/* SramSettings *************************************************************/

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

/* FatSettings **************************************************************/

FatSettings::FatSettings()
{
}

FatSettings::~FatSettings()
{
}

void FatSettings::Load()
{
	FILE* f = fopen("tetattds.dat", "rb");
	if(f == NULL) {
		printf("Failed to open tetattds.dat for reading.\n");
		return;
	}
	
	fread(settings, sizeof(InternalSettings), 1, f);
	fclose(f);
}

void FatSettings::Save()
{
	FILE* f = fopen("tetattds.dat", "wb");
	if(f == NULL) {
		printf("Failed to open tetattds.dat for writing.\n");
		return;
	}
	
	fwrite(settings, sizeof(InternalSettings), 1, f);
	fclose(f);
}
