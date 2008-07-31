#include "tetattds.h"
#include "platformgraphics.h"
#include "baseblock.h"
#include "playfield.h"
#include "util.h"
#include "sprite.h"
#include <SDL.h>

FieldGraphics* g_fieldGraphics = NULL;
extern SDL_Surface
	*background,
	*singlebackground,
	*wifibackground,
	*sprites,
	*font,
	*smalltiles;
static SDL_Surface *subbackground = NULL;
SDL_Color sprites_pal[2][256];

void PlatformGraphics::InitMainScreen()
{
	memcpy(sprites_pal[0], sprites->format->palette->colors, sizeof(sprites_pal[0]));
	for( unsigned int i = 0; i < 256; i += 1 )
	{
		sprites_pal[1][i].r = sprites_pal[0][i].r >> 1;
		sprites_pal[1][i].b = sprites_pal[0][i].b >> 1;
		sprites_pal[1][i].g = sprites_pal[0][i].g >> 1;
	}
}

void PlatformGraphics::InitSubScreen(bool wifi)
{
	subbackground = wifi ? wifibackground : singlebackground;
}

PlatformGraphics::PlatformGraphics()
:	framebuffer(NULL)
{
	framebuffer = SDL_CreateRGBSurface(
    SDL_SWSURFACE, //Uint32 flags
    256,           //int width
    384,           //int height
    15,            //int bitsPerPixel
    0x0000001f,    //Uint32 Rmask
    0x000003e0,    //Uint32 Gmask
    0x00007c00,    //Uint32 Bmask
    0x00000000);   //Uint32 Amask
	
	if(framebuffer == NULL) {
		fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
		exit(1);
	}
}

PlatformGraphics::~PlatformGraphics()
{
}

void PlatformGraphics::DrawField(PlayField *pf, int x, int y, int tile, bool shaded)
{
	SDL_Rect tilerect, destrect;
	tilerect.w = tilerect.h = destrect.w = destrect.h = BLOCKSIZE;
	tilerect.x = (tile % 8) * BLOCKSIZE;
	tilerect.y = (tile / 8) * BLOCKSIZE;
	destrect.x = x;
	destrect.y = y + (int)pf->GetScrollOffset() + 192;
	
	if (shaded)
		SDL_SetColors(sprites, sprites_pal[1], 0, 256);

	SDL_BlitSurface(sprites, &tilerect, framebuffer, &destrect);

	if (shaded)
		SDL_SetColors(sprites, sprites_pal[0], 0, 256);
}

void PlatformGraphics::Draw(PlayField *pf)
{
	SDL_Rect dstrect = {0, 192, 256, 192};
	SDL_BlitSurface(background, NULL, framebuffer, &dstrect);

	PrintScore(pf->GetScore());
	PrintTime(pf->GetTimeTicks());
	PrintStopTime(pf->GetScrollPause());

	DrawFields(pf);
	DrawEffects();
	DrawMarkers(pf);
	
	SDL_Rect rect = {0, 192, 256, 192};
	SDL_BlitSurface(framebuffer, &rect, SDL_GetVideoSurface(), &rect);
}

void PlatformGraphics::DrawSubScreen()
{
	SDL_BlitSurface(subbackground, NULL, framebuffer, NULL);
	if(subbackground == wifibackground) {
		PrintPlayerOffset();
	}
	PrintChat();
	for(int i = 0; i < 4; i++) {
		PlayerInfo* player = players[i + playerOffset];
		if(player != NULL) {
			ReallyDrawSmallField(i, player);
			ReallyPrintPlayerInfo(i, player);
		}
	}

	SDL_Rect rect = {0, 0, 256, 192};
	SDL_BlitSurface(framebuffer, &rect, SDL_GetVideoSurface(), &rect);
}

void PlatformGraphics::ReallyDrawSmallField(int fieldNum, PlayerInfo* player)
{
	SDL_Rect tilerect, destrect;
	tilerect.w = destrect.w = tilerect.h = destrect.h = 8;
	uint32_t cell = SMALL_FIELD_OFFSET + fieldNum * 8;
	for(int i = 0; i < 12*6; i++)
	{
		uint32_t tile = player->fieldState[i];
		//if(player->dead)
		//	tile |= (1 << 12);
		destrect.x = (cell % BLOCKMAP_STRIDE) * 8;
		destrect.y = (cell / BLOCKMAP_STRIDE) * 8;
		tilerect.x = (tile % 8) * 8;
		tilerect.y = (tile / 8) * 8;
		SDL_BlitSurface(smalltiles, &tilerect, framebuffer, &destrect);
		cell++;
		if((i+1) % PF_WIDTH == 0)
		{
			cell += BLOCKMAP_STRIDE - PF_WIDTH;
		}
	}
}

void PlatformGraphics::PrintPlayerInfo(int playerNum, PlayerInfo* player)
{
	ASSERT(playerNum >= 0);
	ASSERT(playerNum < MAX_PLAYERS);
	ASSERT(player != NULL);
	players[playerNum] = player;
}

void PlatformGraphics::ClearPlayer(int playerNum)
{
	ASSERT(playerNum >= 0);
	ASSERT(playerNum < MAX_PLAYERS);
	players[playerNum] = NULL;
}

void PlatformGraphics::PrintLarge(uint32_t cell, const char* text, bool subScreen)
{
	ASSERT(text != NULL);

	SDL_Rect tilerect, destrect;
	tilerect.w = destrect.w = 8;
	tilerect.h = destrect.h = 16;	
	destrect.x = (cell % TEXTMAP_STRIDE) * 8;
	destrect.y = (cell / TEXTMAP_STRIDE) * 8;
	while(*text != '\0')
	{
		int tile = LargeCharTile(*text);
		tilerect.x = (tile % 16) * 8;
		tilerect.y = (tile / 16) * 8;

		SDL_BlitSurface(font, &tilerect, framebuffer, &destrect);

		destrect.x += 8;
		text++;
	}
}

void PlatformGraphics::PrintSmall(uint32_t cell, const char* text)
{
	ASSERT(text != NULL);
	
	SDL_Rect tilerect, destrect;
	tilerect.w = destrect.w = tilerect.h = destrect.h = 8;	
	destrect.x = (cell % TEXTMAP_STRIDE) * 8;
	destrect.y = (cell / TEXTMAP_STRIDE) * 8;
	while(*text != '\0')
	{
		int tile = SmallCharTile(*text);
		tilerect.x = (tile % 16) * 8;
		tilerect.y = (tile / 16) * 8;

		SDL_BlitSurface(font, &tilerect, framebuffer, &destrect);

		destrect.x += 8;
		text++;
	}
}

void PlatformGraphics::PrintChat()
{
	int cursor = 0;
	int curChatLine = lastChatLine + 1;
	for(int i=0;i<MAX_CHAT_LINES;i++)
	{
		if(curChatLine == MAX_CHAT_LINES)
			curChatLine = 0;
		PrintSmall(cursor, chatBuffer[curChatLine]);
		cursor+=TEXTMAP_STRIDE;
		curChatLine++;
	}
}

void PlatformGraphics::ClearChat()
{
	memset(chatBuffer, 0, sizeof(chatBuffer));
	lastChatLine = -1;
}
