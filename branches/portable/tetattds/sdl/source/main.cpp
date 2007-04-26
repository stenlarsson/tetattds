#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <SDL.h>
#include <driver.h>
#include <textentrydialog.h>

#include "tetattds.h"

#include <stdio.h>

#include "fieldgraphics.h"
#include "util.h"
#include "settings.h"
#include "statusdialog.h"
#include "state.h"
#include "sound.h"
#include "sprite.h"
#include "udpsocket.h"

char name[10];
Settings* settings = NULL;
uint32_t heldKeys, downKeys, upKeys;


void SeedRandom()
{
	srand(SDL_GetTicks());
}

void ShowSplashScreen()
{
	// No splash screen for now...
}
	
void HideSplashScreen()
{
	// No splash screen for now...
}

void InitSettings()
{		
	settings = new FatSettings();
	settings->Load();
}

void GetName()
{
	const char * user = getenv("USER");
	strncpy(name, user ? user : "anonymous", sizeof(name));
}

// TODO: Move this someplace else
void InitGui();

SDL_Surface
  *menubackground = NULL,
	*background = NULL,
	*singlebackground = NULL,
	*splash2 = NULL,
	*wifibackground = NULL,
	*sprites = NULL,
	*font = NULL,
	*smalltiles = NULL;

#define LOAD_IMAGE(name, kind) \
	name = SDL_LoadBMP("images/" #kind "/" #name ".bmp"); \
	if (name == NULL) { \
		fprintf(stderr, "Failed to load %s-image %s\n", #kind, #name); \
		return EXIT_FAILURE; \
	}
		
	

int main(int,char **)
{
	if(SDL_Init(SDL_INIT_VIDEO) == -1) {
		fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	SDL_WM_SetCaption(VERSION_STRING, VERSION_STRING);

	SDL_Surface* surface = SDL_SetVideoMode(
		256, //int width
		384, //int height
		0,   //int bitsperpixel
		0);  //Uint32 flags
	if(surface == NULL) {
		fprintf(stderr, "Failed to set SDL video mode: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	LOAD_IMAGE(menubackground,   bg);
	LOAD_IMAGE(background,       bg_tile);
	LOAD_IMAGE(singlebackground, bg_tile);
	LOAD_IMAGE(splash2,          bg_tile);
	LOAD_IMAGE(wifibackground,   bg_tile);
	LOAD_IMAGE(sprites,          tile16);
	LOAD_IMAGE(font,             tile8);
	LOAD_IMAGE(smalltiles,       tile8);
	
	SDL_SetColorKey(sprites, SDL_SRCCOLORKEY | SDL_RLEACCEL, 0);
	SDL_SetColorKey(font, SDL_SRCCOLORKEY | SDL_RLEACCEL, 0);
	SDL_SetColorKey(smalltiles, SDL_SRCCOLORKEY | SDL_RLEACCEL, 0);
	
	GetName();
	
	if(!UdpSocket::InitSockets()) return 1;
	Sprite::InitSprites();
	g_fieldGraphics = new FieldGraphics();
	InitGui();
	InitStates();
	
	while(true)
	{
		uint32_t ticks = SDL_GetTicks();
		if(!StateTick()) {
			break;
		}
		g_fieldGraphics->DrawSubScreen();
		SDL_Flip(surface);

		//while(SDL_GetTicks() - ticks < 16)
		//	Sound::UpdateMusic();
		// Emulate swiWaitForVBlank...
		Sound::UpdateMusic();
		SDL_Delay(16 - (SDL_GetTicks() - ticks) % 16);
	}

	SDL_Quit();
	return EXIT_SUCCESS;
}

void* GetMenuBackground()
{
	return menubackground;
}

