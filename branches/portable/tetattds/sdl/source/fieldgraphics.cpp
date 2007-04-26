#include "tetattds.h"
#include "fieldgraphics.h"
#include "baseblock.h"
#include "playfield.h"
#include "util.h"
#include "sprite.h"
#include "marker.h"
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
	marker(GetMarker(true)),
	touchMarker(GetMarker(false)),
	lastChatLine(-1)
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
  
	for(int i = 0; i < 4; i++) {
		ClearSmallField(i);
		smallFields[i].player = NULL;
	}

	memset(chatBuffer,0,sizeof(chatBuffer));
}

FieldGraphics::~FieldGraphics()
{
	delete marker;
	delete touchMarker;
}

void FieldGraphics::Draw(PlayField *pf)
{
	SDL_Rect dstrect = {0, 192, 256, 192};
	SDL_BlitSurface(background, NULL, framebuffer, &dstrect);

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
		destrect.y = pf->GetFieldY(i) + scrollOffset + 192;
	
		SDL_BlitSurface(sprites, &tilerect, framebuffer, &destrect);
	}
	
	// Gray out bottom row tiles (or all tiles if we are dead)
	if (SDL_MUSTLOCK(framebuffer))
		SDL_LockSurface(framebuffer);
	
	for (int y = (state == PFS_DEAD) ? 192 : 384 + scrollOffset; y < 384; y++)
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
			marker->Disable();
			touchMarker->Disable();
			break;
		case MM_KEY:
			{
				int markerPos = pf->GetMarkerPos();
				marker->SetPos(pf->GetFieldX(markerPos), pf->GetFieldY(markerPos) + scrollOffset);
				marker->Draw();
				touchMarker->Disable();
			}
			break;
		case MM_TOUCH:
		  {
				marker->Disable();
				int touchPos = pf->GetTouchPos();
				touchMarker->SetPos(pf->GetFieldX(touchPos), pf->GetFieldY(touchPos) + scrollOffset);
				touchMarker->Draw();
			}
			break;
		}
	}
	else
	{
		marker->Disable();
		touchMarker->Disable();
	}
	
	SDL_Rect rect = {0, 192, 256, 192};
	SDL_BlitSurface(framebuffer, &rect, SDL_GetVideoSurface(), &rect);
}

void FieldGraphics::DrawSubScreen()
{
	SDL_BlitSurface(subbackground, NULL, framebuffer, NULL);
	PrintChat();
	for(int i = 0; i < 4; i++) {
		ReallyDrawSmallField(i);
		ReallyPrintPlayerInfo(i);
	}

	SDL_Rect rect = {0, 0, 256, 192};
	SDL_BlitSurface(framebuffer, &rect, SDL_GetVideoSurface(), &rect);
}

void FieldGraphics::DrawSmallField(int fieldNum, char* field, bool shaded)
{
	ASSERT(fieldNum >= 0);
	ASSERT(fieldNum < 4);
	ASSERT(field != NULL);
	memcpy(smallFields[fieldNum].field, field, 12*6);
	smallFields[fieldNum].shaded = shaded;
}

void FieldGraphics::ReallyDrawSmallField(int fieldNum)
{
	SDL_Rect tilerect, destrect;
	tilerect.w = destrect.w = tilerect.h = destrect.h = 8;
	uint32_t cell = SMALL_FIELD_OFFSET + fieldNum * 8;
	for(int i = 0; i < 12*6; i++)
	{
		uint32_t tile = smallFields[fieldNum].field[i];
		//if(smallFields[fieldNum].shaded)
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

void FieldGraphics::ClearSmallField(int fieldNum)
{
	memset(smallFields[fieldNum].field, TILE_BLANK, 12*6);
	smallFields[fieldNum].shaded = false;
}

void FieldGraphics::PrintPlayerInfo(PlayerInfo* player)
{
	ASSERT(player != NULL);
	smallFields[player->fieldNum].player = player;
}

void FieldGraphics::ReallyPrintPlayerInfo(int fieldNum)
{
	PlayerInfo* player = smallFields[fieldNum].player;
	if(player == NULL)
		return;

	// name
	char name[8+1];
	snprintf(name, sizeof(name), "%-8.8s", player->name);
	PrintSmall(NAME_TEXT_OFFSET + player->fieldNum * 8, name);
	
	// wins
	char wins[8+1];
	snprintf(wins, sizeof(wins), "%02i wins", player->wins);
	PrintSmall(WINS_TEXT_OFFSET + player->fieldNum * 8, wins);

	// level
	char level[8+1];
	snprintf(level, sizeof(level), "lv %02i", player->level+1);
	PrintSmall(LEVEL_TEXT_OFFSET + player->fieldNum * 8, level);

	// ready/typing
	char ready[8+1];
	snprintf(ready, sizeof(ready), (player->ready ? "READY" : (player->typing ? "typing" : "      ")));
	PrintSmall(READY_TEXT_OFFSET + player->fieldNum * 8, ready);

	// place
	const char* digit[] = {" ", "1", "2", "3", "4", "5"};
	const char* suffix[] = {"  ", "ST", "ND", "RD", "TH", "TH"};
	PrintLarge(
		PLACE_TEXT_OFFSET + player->fieldNum * 8,
		digit[player->place]);
	PrintSmall(
		PLACE_TEXT_OFFSET + player->fieldNum * 8 + TEXTMAP_STRIDE + 1,
		suffix[player->place]);
}

void FieldGraphics::ClearPlayer(PlayerInfo* player)
{
	ASSERT(player != NULL);

	smallFields[player->fieldNum].player = NULL;
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

void FieldGraphics::AddChat(char* text)
{
	ASSERT(text != NULL);
	
	int textLen = strlen(text);
	if(textLen > 32) // Too long, try to break it up
	{
		char* space = NULL;
		char* nextSpace = NULL;
		// Skip to the first space (if any)
		space = strchr(text, ' ');
		if(space != NULL && (textLen - strlen(space) <= 32) ) // If there is at least one space and the first word isn't horribly long
			nextSpace = strchr(space+1, ' ');	// Find the next space (if any)
		if(nextSpace != NULL)
			if(textLen - strlen(nextSpace) > 32)
				nextSpace = NULL;
		if(nextSpace == NULL)
		{
			// We end up here if theres 33+ characters of junk and no space. (probably shouldn't happen at all)
			// Or if the first two words are too long. (happens when someone types a long word in chat, the first word being the nickname)
			if(text[32] == ' ') // takes care of case with words exactly 32 chars long.
			{
				text[32] = '\0';
				AddChat(text);
				AddChat(text+33);
				return;
			}
	
			char buf[32+1];
			strncpy(buf, text, 32); // Get the first 32 characters
			buf[32] = '\0';
			AddChat(buf);
			AddChat(text+32);
			return;
		}

		while(true) // Search for where to break up the line.
		{
			if(nextSpace == NULL)
				break;
			if(textLen - strlen(nextSpace) > 32)
				break;

			space = nextSpace;
			nextSpace = strchr(space+1, ' ');
		}
		// Breakpoint found, do some magic!
		*space = '\0';
		AddChat(text);
		AddChat(space+1);
	}
	else // A nice short line, add it to the buffer
	{
		lastChatLine++;
		if(lastChatLine == MAX_CHAT_LINES)
			lastChatLine = 0;
		strcpy(chatBuffer[lastChatLine], text);
	}
}

void FieldGraphics::PrintChat()
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

void FieldGraphics::ClearChat()
{
	memset(chatBuffer, 0, sizeof(chatBuffer));
	lastChatLine = -1;
}
