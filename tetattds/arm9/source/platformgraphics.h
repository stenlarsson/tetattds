#pragma once

#include "ds.h"
#include "fieldgraphics.h"

// map baseblocks, the same are used for both main and sub
// 32 blocks of 0x800 bytes each starting at 0x6000000
#define BACKGROUND_MAP_BASE  0  // start = 0x6000000, size = 32*32*2 = 0x00800
#define BLOCKS_MAP_BASE      1  // start = 0x6000800, size = 32*32*2 = 0x00800
#define CONSOLE_MAP_BASE     2  // start = 0x6001000, size = 32*32*2 = 0x00800
#define TEXT_MAP_BASE        3  // start = 0x6001800, size = 32*32*2 = 0x00800

// tile baseblocks, the same are used for both main and sub
// 8 blocks of 0x4000 bytes each starting at 0x6000000
#define BACKGROUND_TILE_BASE 1  // start = 0x6004000, size ~ 39 KB = 0xA000
#define BLOCKS_TILE_BASE     4  // start = 0x6010000, size = 32 KB = 0x8000
#define CONSOLE_TILE_BASE    6  // start = 0x6018000, size =  8 KB = 0x2000
#define TEXT_TILE_BASE       7  // start = 0x601C000, size = ?

// map offsets etc
#define BLOCKMAP_STRIDE 32
#define SMALL_FIELD_OFFSET (11*BLOCKMAP_STRIDE + 1)

#define TEXTMAP_STRIDE 32
#define SCORE_TEXT_OFFSET (3*TEXTMAP_STRIDE)
#define TIME_TEXT_OFFSET (7*TEXTMAP_STRIDE)
#define STOP_TIME_TEXT_OFFSET (11*TEXTMAP_STRIDE)
#define COUNTDOWN_TEXT_OFFSET (5*TEXTMAP_STRIDE + 16)
#define NAME_TEXT_OFFSET (8*TEXTMAP_STRIDE)
#define WINS_TEXT_OFFSET (9*TEXTMAP_STRIDE)
#define LEVEL_TEXT_OFFSET (10*TEXTMAP_STRIDE)
#define READY_TEXT_OFFSET (14*TEXTMAP_STRIDE + 1)
#define PLACE_TEXT_OFFSET (15*TEXTMAP_STRIDE + 2)

#define MAX_CHAT_LENGTH 128
#define MAX_CHAT_LINES 6

#define PLAYFIELD_OFFSET_X 88
#define PLAYFIELD_OFFSET_Y 0

class PlatformGraphics : public FieldGraphics
{
public:
	static void InitMainScreen();
	static void InitSubScreen(bool wifi);
	PlatformGraphics();
	~PlatformGraphics();
	static void CreateShadedPalette(uint16_t* dest, uint16_t* src);

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
	virtual void AddChat(char* text);
	void ClearChat();

private:
	void PrintSmall(uint32_t offset, const char* text);
	void PrintLarge(uint32_t offset, const char* text, bool subScreen);
	void PrintChat();
	
	void ClearText(uint16_t* cell, int length);
	void ClearTextLine(uint32_t cell);

	uint16_t* mainBlockMap;
	uint16_t* subBlockMap;
	uint16_t* mainTextMap;
	uint16_t* subTextMap;
};

