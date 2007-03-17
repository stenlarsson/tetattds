#include "tetattds.h"
#include "fieldgraphics.h"
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
	*font;
static SDL_Surface *subbackground = NULL;

void FieldGraphics::InitMainScreen()
{
}

void FieldGraphics::InitSubScreen(bool wifi)
{
	subbackground = wifi ? wifibackground : singlebackground;
}

FieldGraphics::FieldGraphics()
:	framebuffer(NULL),
	effects(),
	marker(true),
	touchMarker(false),
	lastChatLine(-1)
{
	framebuffer = SDL_CreateRGBSurface(
    SDL_SWSURFACE, //Uint32 flags
    256,           //int width
    192,           //int height
    15,            //int bitsPerPixel
    0x0000001f,    //Uint32 Rmask
    0x000003e0,    //Uint32 Gmask
    0x00007c00,    //Uint32 Bmask
    0x00000000);   //Uint32 Amask
	
	if(framebuffer == NULL) {
		fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
		exit(1);
	}
  
	memset(chatBuffer,0,sizeof(chatBuffer));
	
	g_fieldGraphics = this;
}

FieldGraphics::~FieldGraphics()
{
}

void FieldGraphics::Draw(PlayField *pf)
{
	SDL_BlitSurface(background, NULL, framebuffer, NULL);

	int scrollOffset = (int)pf->GetScrollOffset();
	
	PrintScore(*pf->GetScore());
	PrintTime(pf->GetTimeTicks());
	PrintStopTime(pf->GetScrollPause());
	PFState state = pf->GetState();

	// draw field
	SDL_Rect tilerect, destrect;
	tilerect.w = tilerect.h = destrect.w = destrect.h = BLOCKSIZE;
	for(int i = PF_FIRST_BLOCK_FIRST_VISIBLE_ROW; i < PF_NUM_BLOCKS; i++)
	{
		BaseBlock* block = pf->GetField(i);
		uint16_t tile;
		if(block != NULL && block->GetState() != BST_MOVING)
			tile = block->GetTile();
		else
			tile = TILE_BLANK;

		tilerect.x = (tile % 8) * BLOCKSIZE;
		tilerect.y = (tile / 8) * BLOCKSIZE;
		destrect.x = pf->GetFieldX(i);
		destrect.y = pf->GetFieldY(i) + scrollOffset;
	
		SDL_BlitSurface(sprites, &tilerect, framebuffer, &destrect);
	}
	
	// Gray out bottom row tiles (or all tiles if we are dead)
	if (SDL_MUSTLOCK(framebuffer))
		SDL_LockSurface(framebuffer);
	
	for (int y = (state == PFS_DEAD) ? 0 : 192 + scrollOffset; y < 192; y++)
	{
		uint16_t* p = ((uint16_t*)(((uint8_t*)framebuffer->pixels) + y * framebuffer->pitch)) + 88;
		for (int x = 0; x < PF_WIDTH * BLOCKSIZE; x++)
			p[x] = (p[x] >> 1) & 0x3DEF;
	}

	if (SDL_MUSTLOCK(framebuffer))
		SDL_UnlockSurface(framebuffer);

	effects.Draw();

	//and finally draw marker
	if(state == PFS_PLAY || state == PFS_START)
	{
		switch(pf->GetControlMode())
		{
		case MM_NONE:
			marker.Hide();
			touchMarker.Hide();
			break;
		case MM_KEY:
			{
				int markerPos = pf->GetMarkerPos();
				marker.Draw(pf->GetFieldX(markerPos), pf->GetFieldY(markerPos) + scrollOffset);
				touchMarker.Hide();
			}
			break;
		case MM_TOUCH:
		  {
				int touchPos = pf->GetTouchPos();
				marker.Hide();
				touchMarker.Draw(pf->GetFieldX(touchPos), pf->GetFieldY(touchPos) + scrollOffset);
			}
			break;
		}
	}
	else
	{
		marker.Hide();
		touchMarker.Hide();
	}
	
	SDL_BlitSurface(framebuffer, NULL, SDL_GetVideoSurface(), NULL);
}

void FieldGraphics::DrawSmallField(int fieldNum, char* field, bool shaded)
{
	// TODO: Implement, no problem except placement
}

void FieldGraphics::ClearSmallField(int fieldNum)
{
	// TODO: Implement, no problem except placement
}

void FieldGraphics::PrintPlayerInfo(PlayerInfo* player)
{
	// TODO: Implement
}

void FieldGraphics::ClearPlayer(PlayerInfo* player)
{
	// TODO: Implement
}

void FieldGraphics::PrintScore(int score)
{
	char str[9+1];
	snprintf(str, sizeof(str), "%9i", score);
	PrintLarge(SCORE_TEXT_OFFSET, str);
}

void FieldGraphics::PrintTime(int ticks)
{
	int seconds = ticks/60;
	char str[9+1];
	snprintf(str, sizeof(str), "%6i:%02i", seconds / 60, seconds % 60);
	PrintLarge(TIME_TEXT_OFFSET, str);
}

void FieldGraphics::PrintCountdown(int count)
{
	char str[2];
	if(count == 0)
		snprintf(str, sizeof(str), " ");
	else
		snprintf(str, sizeof(str), "%i", count);
	PrintLarge(COUNTDOWN_TEXT_OFFSET, str);
}

void FieldGraphics::PrintStopTime(int ticks)
{
	char str[9+1];
	snprintf(str, sizeof(str), "%6i.%02i", ticks/60, ticks*100/60%100);
	PrintLarge(STOP_TIME_TEXT_OFFSET, str);
}

void FieldGraphics::PrintLarge(uint32_t cell, const char* text)
{
	ASSERT(text != NULL);

	SDL_Rect tilerect, destrect;
	tilerect.w = destrect.w = 8;
	tilerect.h = destrect.h = 16;	
	destrect.x = (cell % TEXTMAP_STRIDE) * 8;
	destrect.y = (cell / TEXTMAP_STRIDE) * 8;
	while(*text != '\0')
	{
		char c = *text;
		int tile;
		if(c >= '0' && c <= '9')
		{
			tile = c - '0' + TILE_LARGE_ZERO;
		}
		else if(c == ':')
		{
			tile = TILE_LARGE_COLON;
		}
		else if(c == '\'')
		{
			tile = TILE_LARGE_TICK;
		}
		else if(c == '"')
		{
			tile = TILE_LARGE_DOUBLE_TICK;
		}
		else if(c == '.')
		{
			tile = TILE_LARGE_FULL_STOP;
		}
		else
		{
			tile = TILE_LARGE_SPACE;
		}
		tilerect.x = (tile % 16) * 8;
		tilerect.y = (tile / 16) * 8;

		SDL_BlitSurface(font, &tilerect, framebuffer, &destrect);

		destrect.x += 8;
		text++;
	}
}

void FieldGraphics::PrintSmall(uint32_t cell, const char* text)
{
	ASSERT(text != NULL);
	
	SDL_Rect tilerect, destrect;
	tilerect.w = destrect.w = tilerect.h = destrect.h = 8;	
	destrect.x = (cell % TEXTMAP_STRIDE) * 8;
	destrect.y = (cell / TEXTMAP_STRIDE) * 8;
	while(*text != '\0')
	{
		char c = *text;
		int tile = (c >= ' ' && c <= '~') ? c - ' ' : 0;
		tilerect.x = (tile % 16) * 8;
		tilerect.y = (tile / 16) * 8;

		SDL_BlitSurface(font, &tilerect, framebuffer, &destrect);

		destrect.x += 8;
		text++;
	}
}

void FieldGraphics::ClearTextLine(uint32_t cell)
{
	// No action required, we always repaint everything
}

void FieldGraphics::ClearText(uint32_t cell, int length)
{
	// No action required, we always repaint everything
}

void FieldGraphics::AddChat(char* text)
{
	// TODO: Implement
}

void FieldGraphics::PrintChat()
{
	// TODO: Implement
}

void FieldGraphics::ClearChat()
{
	// TODO: Implement
}
