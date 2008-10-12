#pragma once

#include "fieldgraphics.h"
#include "protocol.h"
#include "effecthandler.h"
#include <SDL.h>

// map offsets etc
#define BLOCKMAP_STRIDE 32
#define SMALL_FIELD_OFFSET (11*BLOCKMAP_STRIDE + 1)

#define TEXTMAP_STRIDE 32
#define SCORE_TEXT_OFFSET (27*TEXTMAP_STRIDE)
#define TIME_TEXT_OFFSET (31*TEXTMAP_STRIDE)
#define STOP_TIME_TEXT_OFFSET (35*TEXTMAP_STRIDE)
#define COUNTDOWN_TEXT_OFFSET (29*TEXTMAP_STRIDE + 16)
#define NAME_TEXT_OFFSET (8*TEXTMAP_STRIDE)
#define WINS_TEXT_OFFSET (9*TEXTMAP_STRIDE)
#define LEVEL_TEXT_OFFSET (10*TEXTMAP_STRIDE)
#define READY_TEXT_OFFSET (14*TEXTMAP_STRIDE + 1)
#define PLACE_TEXT_OFFSET (15*TEXTMAP_STRIDE + 2)
#define PLAYER_OFFSET_TEXT_OFFSET (23*TEXTMAP_STRIDE + 20)

#define PLAYFIELD_OFFSET_X 88
#define PLAYFIELD_OFFSET_Y 0

struct SDL_Surface;

class PlatformGraphics : public FieldGraphics
{
public:
	PlatformGraphics();
	~PlatformGraphics();

	virtual void InitMainScreen();
	virtual void InitSubScreen(bool wifi);

	virtual int GetFieldX(int fieldNum) const {
		return PLAYFIELD_OFFSET_X + FieldGraphics::GetFieldX(fieldNum);
	}
	virtual int GetFieldY(int fieldNum, bool scrolled) const {
		return PLAYFIELD_OFFSET_Y + FieldGraphics::GetFieldY(fieldNum, scrolled);
	}

	void PrintPlayerInfo(int playerNum, PlayerInfo* player);
	void ClearPlayer(int playerNum);
	void ClearSmallField(int fieldNum);

	void Draw(PlayField *pf);
	void DrawField(PlayField *pf, int x, int y, int tile, bool shaded);
	void DrawSubScreen();

	virtual void PrintChat() {};

	void TogglePlayerOffset() { playerOffset ^= 4; }
	
	SDL_Surface *framebuffer;
private:
	SDL_Surface *subbackground;
	SDL_Color sprites_pal[2][256];
	int numChatLines;

	void ReallyDrawSmallField(int fieldNum, PlayerInfo* player);

	void PrintSmall(uint32_t offset, const char* text);
	void PrintLarge(uint32_t offset, const char* text, bool subScreen);
	void ReallyPrintChat();
};

extern PlatformGraphics* g_PlatformGraphics;
