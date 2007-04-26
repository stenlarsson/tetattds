#pragma once

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
#define PLAYFIELD_OFFSET_Y 192

class Sprite;
class PlayField;
struct SDL_Surface;

struct PlayerInfo
{
	char name[64];
	int fieldNum;
	bool dead;
	int place;
	int level;
	int wins;
	bool ready;
	bool connected;
	bool typing;
};

class FieldGraphics
{
public:
	static void InitMainScreen();
	static void InitSubScreen(bool wifi);
	FieldGraphics();
	~FieldGraphics();

	void SetScrollOffset(int scrollOffset);

	void DrawSmallField(int fieldNum, char* field, bool shaded);
	void PrintPlayerInfo(PlayerInfo* player);
	void ClearPlayer(PlayerInfo* player);
	void ClearSmallField(int fieldNum);

	void Draw(PlayField *pf);
	void PrintCountdown(int count);
	void AddChat(char* text);
	void ClearChat();
	
	EffectHandler *GetEffectHandler() { return &effects; }

	SDL_Surface *framebuffer;
private:
	void PrintScore(int score);
	void PrintTime(int ticks);
	void PrintStopTime(int ticks);

	void PrintSmall(uint32_t offset, const char* text);
	void PrintLarge(uint32_t offset, const char* text);
	void ClearText(uint32_t offset, int length);
	void ClearTextLine(uint32_t offset);
	void PrintChat();
	
	EffectHandler effects;
	Sprite* marker;
	Sprite* touchMarker;
	
	char chatBuffer[MAX_CHAT_LINES][32+1];
	int lastChatLine;
};

extern FieldGraphics* g_fieldGraphics;
