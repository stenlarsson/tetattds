#pragma once

#include "fieldgraphics.h"
#include "protocol.h"
#include "effecthandler.h"

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

#define MAX_CHAT_LENGTH 128
#define MAX_CHAT_LINES 6

#define PLAYFIELD_OFFSET_X 88
#define PLAYFIELD_OFFSET_Y 0

struct SDL_Surface;

class PlatformGraphics : public FieldGraphics
{
public:
	static void InitMainScreen();
	static void InitSubScreen(bool wifi);
	PlatformGraphics();
	~PlatformGraphics();

	virtual int GetFieldX(int fieldNum) const {
		return PLAYFIELD_OFFSET_X + FieldGraphics::GetFieldX(fieldNum);
	}
	virtual int GetFieldY(int fieldNum, bool scrolled) const {
		return PLAYFIELD_OFFSET_Y + FieldGraphics::GetFieldY(fieldNum, scrolled);
	}

	void DrawSmallField(int fieldNum, char* field, bool shaded);
	void PrintPlayerInfo(PlayerInfo* player);
	void ClearPlayer(PlayerInfo* player);
	void ClearSmallField(int fieldNum);

	void Draw(PlayField *pf);
	void DrawField(PlayField *pf, int x, int y, int tile, bool shaded);
	void DrawSubScreen();
	void ClearChat();
	
	SDL_Surface *framebuffer;
private:
	void ReallyDrawSmallField(int fieldNum);

	void PrintSmall(uint32_t offset, const char* text);
	void PrintLarge(uint32_t offset, const char* text, bool subScreen);
	void PrintChat();
	
	struct {
		char field[12*6];
		bool shaded;
		PlayerInfo* player;
	} smallFields[4];
};

extern PlatformGraphics* g_PlatformGraphics;
