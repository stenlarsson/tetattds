#pragma once

#include "protocol.h"
#include "marker.h"
#include "effecthandler.h"

class BaseBlock;
class PlayField;

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

private:
	void PrintScore(int score);
	void PrintTime(int ticks);
	void PrintStopTime(int ticks);

	static void CreateShadedPalette(u16* dest, u16* src);
	void PrintSmall(u16* startCell, const char* text);
	void PrintLarge(u16* startCell, const char* text);
	void ClearText(u16* startCell, int length);
	void ClearTextLine(u16* startCell);
	void PrintChat();
	
	u16* mainBlockMap;
	u16* subBlockMap;
	u16* mainTextMap;
	u16* subTextMap;
	EffectHandler effects;
	Marker marker;
	Marker touchMarker;
	
	char chatBuffer[MAX_CHAT_LINES][32+1];
	int lastChatLine;
};

extern FieldGraphics* g_fieldGraphics;
