#include "tetattds.h"

#include <stdio.h>
#include <nds/arm9/console.h>

#include <fat.h>

#include "menubackground_bin.h"

#include "splash2_bin.h"
#include "splash2_map_bin.h"
#include "splash2_pal_bin.h"

#include "fieldgraphics.h"
#include "util.h"
#include "settings.h"
#include "sramsettings.h"
#include "fifo.h"
#include "statusdialog.h"
#include "state.h"

#define BLEND_SRC_ALL 0x003F

static unsigned int currentTime = 0;

static bool supportsFat = false;

char name[10];
Settings* settings = NULL;

// used in network lib
unsigned int GetTime()
{
	return currentTime/60;
}

void SeedRandom()
{
	srand(currentTime);
}

void ShowSplashScreen()
{
	swiWaitForVBlank();
	lcdMainOnBottom();
	
	// main screen
	videoSetMode(
		MODE_0_2D |
		DISPLAY_BG0_ACTIVE);
	// 128K
	vramSetBankA(VRAM_A_MAIN_BG);

	// palette
	Decompress(BG_PALETTE, splash2_pal_bin);

	// splashscreen
	BG0_CR =
		BG_32x32 |
		BG_TILE_BASE(BACKGROUND_TILE_BASE) |
		BG_MAP_BASE(BACKGROUND_MAP_BASE) |
		BG_PRIORITY(BACKGROUND_PRIORITY) |
		BG_256_COLOR;
	Decompress((void*)BG_TILE_RAM(BACKGROUND_TILE_BASE), splash2_bin);
	Decompress((void*)BG_MAP_RAM(BACKGROUND_MAP_BASE), splash2_map_bin);
	
	// fade in
	for(int i = 31; i >= 0; i--)
	{
		BLEND_Y = i>>1;
		SUB_BLEND_Y = i>>1;
		swiWaitForVBlank();
	}
}
	
void HideSplashScreen()
{
	// fade out
	BLEND_CR = BLEND_FADE_BLACK | BLEND_SRC_ALL;
	SUB_BLEND_CR = BLEND_FADE_BLACK | BLEND_SRC_ALL;
	
	for(int i = 0; i < 32; i++)
	{
		BLEND_Y = i>>1;
		SUB_BLEND_Y = i>>1;
		swiWaitForVBlank();
	}
}

void InitSettings()
{		
	// init fatLib
	supportsFat = fatInitDefault();
	
	if(supportsFat)
		settings = new FatSettings();
	else
		settings = new SramSettings();
	
	settings->Load();
}

void GetName()
{
	memset(name, 0, sizeof(name));
	char* src = (char*)PersonalData->name;
	char* dest = name;
	for(int i = 0; i < PersonalData->nameLen; i++)
	{
		*dest = *src;
		dest++;
		src+=2;
	}
	*dest = '\0';
}

void myExceptionHandler()
{
	for(int i = 0; i < 16; i++)
		printf("r%02i: 0x%08lx\n", i, exceptionRegisters[i]);

	for(;;);
}

void InitConsole()
{
	BLEND_CR = BLEND_FADE_WHITE | BLEND_SRC_ALL;
	SUB_BLEND_CR = BLEND_FADE_WHITE | BLEND_SRC_ALL;
	BLEND_Y = 15;
	SUB_BLEND_Y = 15;
	swiWaitForVBlank();

	// sub screen
	videoSetModeSub(
		MODE_0_2D |
		DISPLAY_BG3_ACTIVE);
	// 128K
	vramSetBankC(VRAM_C_SUB_BG);

	BG_PALETTE_SUB[0] = RGB15(0,0,0);
	BG_PALETTE_SUB[255] = RGB15(31,31,31);

	// console
	SUB_BG3_CR =
		BG_32x32 |
		BG_TILE_BASE(CONSOLE_TILE_BASE) |
		BG_MAP_BASE(CONSOLE_MAP_BASE) |
		BG_PRIORITY(CONSOLE_PRIORITY) |
		BG_16_COLOR;
	consoleInitDefault(
		(u16*)SCREEN_BASE_BLOCK_SUB(CONSOLE_MAP_BASE),
		(u16*)CHAR_BASE_BLOCK_SUB(CONSOLE_TILE_BASE),
		16);
}

void VBlankHandler()
{
	currentTime++;
}

// TODO: Move this someplace else
void InitGui();

int main(void)
{
	setExceptionHandler(myExceptionHandler);
	irqInit();
	irqSet(IRQ_VBLANK, VBlankHandler);
	irqEnable(IRQ_VBLANK); // needed by swiWaitForVBlank()
	
	InitFifo();
	
	GetName();
	
	InitConsole();
	Sprite::InitSprites();
	g_fieldGraphics = new FieldGraphics();
	InitGui();
	InitStates();
	
	while(true)
	{
		StateTick();
		swiWaitForVBlank();
	}

	return 0;
}

void* GetMenuBackground()
{
	int imageSize;
	return Decompress(menubackground_bin, &imageSize);
}

void* Decompress(const void* source, int* size)
{
	u32 header = *(u32*)source;
	ASSERT((header & 0xFF) == 0x10);

	int uncompressedSize = header >> 8;

	void* temp = malloc(uncompressedSize);

	swiDecompressLZSSWram((void*)source, temp);

	*size = uncompressedSize;
	return temp;
}

void Decompress(void* destination, const void* source)
{
	u32 header = *(u32*)source;
	ASSERT((header & 0xFF) == 0x10);

	int uncompressedSize = header >> 8;
	
	void* temp = malloc(uncompressedSize);

	swiDecompressLZSSWram((void*)source, temp);

	memcpy(destination, temp, uncompressedSize);
	free(temp);
}

