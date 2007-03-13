#pragma once

#include "protocol.h"
#include "marker.h"
#include "effecthandler.h"

class BaseBlock;
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
	Marker marker;
	Marker touchMarker;
	
	char chatBuffer[MAX_CHAT_LINES][32+1];
	int lastChatLine;
};

extern FieldGraphics* g_fieldGraphics;
