#include "tetattds.h"

#include <stdio.h>
#include <nds/arm9/console.h>

#include <fat.h>
#include <dswifi9.h>

#include <driver.h>
#include <theme.h>

#include "menubackground_bin.h"

#include "splash2_bin.h"
#include "splash2_map_bin.h"
#include "splash2_pal_bin.h"

#include "menu1_bin.h"
#include "menu2_bin.h"

#include "wifi.h"
#include "fieldgraphics.h"
#include "util.h"
#include "settings.h"
#include "sound.h"
#include "fifo.h"
#include "statusdialog.h"
#include "state.h"

#define BLEND_SRC_ALL 0x003F

unsigned int currentTime = 0;

static bool hasSetupWifi = false;
static bool supportsFat = false;

char name[10];
Settings* settings = NULL;
FwGui::Driver* gui = NULL;

// used in network lib
unsigned int GetTime()
{
	return currentTime/60;
}

extern State *nextState, *wifiState, *mainMenuState;
State* setupWifiState;
State* splashScreenState;

class SetupWifiState : public State {
public:
	SetupWifiState() 
		: dialog(NULL) {
	}
	virtual void Enter() {
		FieldGraphics::InitSubScreen(true);
		g_fieldGraphics->ClearChat();

		dialog = new StatusDialog("PLEASE WAIT");
		gui->SetActiveDialog(dialog);
		if(hasSetupWifi) {
			nextState = wifiState;
			return;
		}
		SetupWifi();
		status = ASSOCSTATUS_DISCONNECTED;
	}
	virtual void Tick() {
		if (dialog->abort) {
			nextState = mainMenuState;
			return;
		}
		
		WIFI_ASSOCSTATUS newStatus = (WIFI_ASSOCSTATUS)Wifi_AssocStatus();
		if(status != newStatus) {
			status = newStatus;
			static const char* statusMessages[] = {
				"Disconnected",
				"Searching...",
				"Authenticating...",
				"Associating...",
				"Acquiring DHCP...",
				"Associated",
				"Cannot connect"
			};
			char statusString[1024];
			sprintf(
				statusString,
				"Connecting to Access Point\n%s",
				(status <= 6) ? statusMessages[status] : "???");
			dialog->SetStatus(statusString);
				
			if(status == ASSOCSTATUS_ASSOCIATED) {
				hasSetupWifi = true;
				nextState = wifiState;
			}
		}
	}

	virtual void Exit() {
		gui->SetActiveDialog(NULL);
		delete dialog;
		dialog = NULL;
	}
private:
	StatusDialog* dialog;
	WIFI_ASSOCSTATUS status;
};

class SplashScreenState : public State {
public:
	virtual void Enter() {
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
		
		// load music
		Sound::InitMusic();
		Sound::LoadMusic();
	}
	
	void Tick() {
		nextState = mainMenuState;
	}
	
	void Exit() {
		// fade out
		BLEND_CR = BLEND_FADE_BLACK | BLEND_SRC_ALL;
		SUB_BLEND_CR = BLEND_FADE_BLACK | BLEND_SRC_ALL;
		
		for(int i = 0; i < 32; i++)
		{
			BLEND_Y = i>>1;
			SUB_BLEND_Y = i>>1;
			swiWaitForVBlank();
		}
		
		// init fatLib
		supportsFat = fatInitDefault();
		
		if(supportsFat)
			settings = new FatSettings();
		else
			settings = new SramSettings();
		
		settings->Load();
	}
};

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

	gui = new FwGui::Driver();
	int imageSize;
	u16* image = (u16*)Decompress(menubackground_bin, &imageSize);
	FwGui::backgroundImage = image;
	FwGui::selectedColor = FwGui::Color(255, 255, 255);
	FwGui::labelTextColor = FwGui::Color(0, 0, 0);
	FwGui::enabledButtonColor = FwGui::Color(0, 0, 0, 190);
	FwGui::disabledButtonColor = FwGui::Color(171, 171, 171, 190);
	FwGui::buttonTextColor = FwGui::Color(255, 255, 255);
	FwGui::enabledEditBoxColor = FwGui::Color(255, 255, 255, 190);
	TransferSoundData keySound;
	keySound.data = menu1_bin;
	keySound.len = menu1_bin_size;
	keySound.rate = 22050;
	keySound.vol = 64;
	keySound.pan = 64;
	keySound.format = 2;
	FwGui::keyClickSound = &keySound;
	
	TransferSoundData menuSound;
	menuSound.data = menu2_bin;
	menuSound.len = menu2_bin_size;
	menuSound.rate = 22050;
	menuSound.vol = 64;
	menuSound.pan = 64;
	menuSound.format = 2;
	
	setupWifiState = new SetupWifiState;
	splashScreenState = new SplashScreenState;
	InitStates();
	
	while(true)
	{
		if (StateTick())
			playSound(&menuSound);

		gui->Tick();
		Sound::UpdateMusic();
		swiWaitForVBlank();
	}

	return 0;
}
