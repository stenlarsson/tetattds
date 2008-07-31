#pragma once

#include "constants.h"
#include "protocol.h"
#include "effecthandler.h"

// // map offsets etc
// #define BLOCKMAP_STRIDE 32
// #define SMALL_FIELD_OFFSET (11*BLOCKMAP_STRIDE + 1)
// 
// #define TEXTMAP_STRIDE 32
// #define NAME_TEXT_OFFSET (8*TEXTMAP_STRIDE)
// #define WINS_TEXT_OFFSET (9*TEXTMAP_STRIDE)
// #define LEVEL_TEXT_OFFSET (10*TEXTMAP_STRIDE)
// #define READY_TEXT_OFFSET (14*TEXTMAP_STRIDE + 1)
// #define PLACE_TEXT_OFFSET (15*TEXTMAP_STRIDE + 2)
// 
#define MAX_CHAT_LENGTH 128
#define MAX_CHAT_LINES 6

// #define PLAYFIELD_OFFSET_X 88
// #define PLAYFIELD_OFFSET_Y 0
// 
class Sprite;
class PlayField;

struct PlayerInfo
{
	char name[64];
	bool dead;
	int place;
	int level;
	int wins;
	bool ready;
	bool connected;
	bool typing;
	char fieldState[12*6];
};

class FieldGraphics
{
public:
	FieldGraphics();
	virtual ~FieldGraphics();

	virtual int GetFieldX(int fieldNum) const {
		return (fieldNum % 6) * BLOCKSIZE;
	}
	virtual int GetFieldY(int fieldNum, bool scrolled = false) const {
		int scroll = scrolled ? scrollOffset : 0;
		return (fieldNum / 6 - PF_FIRST_VISIBLE_ROW) * BLOCKSIZE + scroll;
	}

	virtual void PrintPlayerInfo(int playerNum, PlayerInfo* player) = 0;
	virtual void ClearPlayer(int playerNum) = 0;

	virtual void Draw(PlayField *pf) = 0;
	void DrawFields(PlayField *pf);
	virtual void DrawField(PlayField *pf, int x, int y, int tile, bool shaded) = 0;
	void DrawMarkers(PlayField *pf);
	inline void DrawEffects() { effects.Draw(); };
		
	void PrintCountdown(int count);
	virtual void AddChat(char* text);
	virtual void ClearChat() = 0;
	void PrintPlayerOffset();

	EffectHandler *GetEffectHandler() { return &effects; }

	virtual void TogglePlayerOffset() = 0;

protected:
	void PrintScore(int score);
	void PrintTime(int ticks);
	void PrintStopTime(int ticks);

	virtual void PrintSmall(uint32_t offset, const char* text) = 0;
	virtual void PrintLarge(uint32_t offset, const char* text, bool subScreen = false) = 0;
	
	int LargeCharTile(char c);
	int SmallCharTile(char c);

 	void ReallyPrintPlayerInfo(int fieldNum, PlayerInfo* player);

	EffectHandler effects;
	Sprite* marker;
	Sprite* touchMarker;
	
	char chatBuffer[MAX_CHAT_LINES][32+1];
	int lastChatLine;
	int scrollOffset;
	PlayerInfo* players[MAX_PLAYERS];
	int playerOffset;
};

extern FieldGraphics* g_fieldGraphics;
