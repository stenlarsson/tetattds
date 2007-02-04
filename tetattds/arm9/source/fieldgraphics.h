#pragma once

#include "protocol.h"

class BaseBlock;

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

	void DrawBlock(BaseBlock* block, int x, int y, bool shaded);
	void Clear(int x, int y);
	void PrintScore(int score);
	void PrintTime(int ticks);
	void PrintCountdown(int count);
	void AddChat(char* text);
	void ClearChat();

private:
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
	int scrollOffset;
	
	char chatBuffer[MAX_CHAT_LINES][32+1];
	int lastChatLine;
};

extern FieldGraphics* g_fieldGraphics;
