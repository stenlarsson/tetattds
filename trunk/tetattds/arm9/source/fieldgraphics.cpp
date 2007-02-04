#include "tetattds.h"
#include "fieldgraphics.h"
#include "baseblock.h"
#include "util.h"
#include "sprite.h"

#include "sprites_bin.h"
#include "sprites_pal_bin.h"
#include "background_bin.h"
#include "background_map_bin.h"
#include "background_pal_bin.h"

#include "singlebackground_bin.h"
#include "singlebackground_map_bin.h"
#include "singlebackground_pal_bin.h"
#include "wifibackground_bin.h"
#include "wifibackground_map_bin.h"
#include "wifibackground_pal_bin.h"
#include "smalltiles_bin.h"
#include "smalltiles_pal_bin.h"

#include "font_bin.h"
#include "font_pal_bin.h"

FieldGraphics* g_fieldGraphics = NULL;

void FieldGraphics::InitMainScreen()
{
	videoSetMode(
		MODE_0_2D |
		DISPLAY_BG0_ACTIVE |
		DISPLAY_BG1_ACTIVE |
		DISPLAY_BG2_ACTIVE |
		DISPLAY_SPR_ACTIVE |
		DISPLAY_SPR_1D |
		DISPLAY_BG_EXT_PALETTE);
	// 128K
	vramSetBankA(VRAM_A_MAIN_BG);
	// 128K
	vramSetBankB(VRAM_B_MAIN_SPRITE);

	// black background
	BG_PALETTE[0] = RGB15(0,0,0);
	
	// 64K, can hold palettes for all four BG layers
	// allocate for cpu access
	vramSetBankE(VRAM_E_LCD);
	
	// background
	Decompress((void*)(0x6880000), background_pal_bin);
	
	// blocks
	Decompress((void*)(0x6880000 + 256*16*2), sprites_pal_bin);
	
	CreateShadedPalette(
		(u16*)(0x6880000 + 256*17*2),
		(u16*)(0x6880000 + 256*16*2));
		
	// text
	Decompress((void*)(0x6880000 + 256*16*2*2), font_pal_bin);
		
	// then allocate it for extended palette use
	vramSetBankE(VRAM_E_BG_EXT_PALETTE);

	// background image
	BG0_CR =
		BG_32x32 |
		BG_TILE_BASE(BACKGROUND_TILE_BASE) |
		BG_MAP_BASE(BACKGROUND_MAP_BASE) |
		BG_PRIORITY(BACKGROUND_PRIORITY) |
		BG_256_COLOR;
	Decompress((void*)BG_TILE_RAM(BACKGROUND_TILE_BASE), background_bin);
	Decompress((void*)BG_MAP_RAM(BACKGROUND_MAP_BASE), background_map_bin);

	// blocks
	BG1_CR =
		BG_32x32 |
		BG_TILE_BASE(BLOCKS_TILE_BASE) |
		BG_MAP_BASE(BLOCKS_MAP_BASE) |
		BG_PRIORITY(BLOCKS_PRIORITY) |
		BG_256_COLOR;
	// currently uses the same tiles as the sprites
	Decompress((void*)BG_TILE_RAM(BLOCKS_TILE_BASE), sprites_bin);
	u16* ptr = (u16*)BG_MAP_RAM(BLOCKS_MAP_BASE);
	for(int i = 0; i < 32*32; i++)
	{
		*ptr++ = TILE_BLANK*4;
	}

	// text
	BG2_CR =
		BG_32x32 |
		BG_TILE_BASE(TEXT_TILE_BASE) |
		BG_MAP_BASE(TEXT_MAP_BASE) |
		BG_PRIORITY(TEXT_PRIORITY) |
		BG_256_COLOR;
	Decompress((void*)BG_TILE_RAM(TEXT_TILE_BASE), font_bin);
	ptr = (u16*)BG_MAP_RAM(TEXT_MAP_BASE);
	for(int i = 0; i < 32*32; i++)
	{
		*ptr++ = 0;
	}
	
	// sprites
	Decompress(SPRITE_GFX, sprites_bin);
	Decompress(SPRITE_PALETTE, sprites_pal_bin);
	Sprite::InitSprites();
}

void FieldGraphics::InitSubScreen(bool wifi)
{
	videoSetModeSub(
		MODE_0_2D |
		DISPLAY_BG0_ACTIVE |
		DISPLAY_BG1_ACTIVE |
		DISPLAY_BG2_ACTIVE |
		DISPLAY_BG3_ACTIVE |
		DISPLAY_BG_EXT_PALETTE);
	// 128k
	vramSetBankC(VRAM_C_SUB_BG);

	BG_PALETTE_SUB[0] = RGB15(0,0,0);
	BG_PALETTE_SUB[255] = RGB15(0,0,0);

	// 32K, can hold palettes for all four BG layers
	// allocate for cpu access
	vramSetBankH(VRAM_H_LCD);
	
	// background
	if(wifi)
		Decompress((void*)(0x6898000), wifibackground_pal_bin);
	else
		Decompress((void*)(0x6898000), singlebackground_pal_bin);
		
	// blocks
	Decompress((void*)(0x6898000 + 256*16*2), smalltiles_pal_bin);

	CreateShadedPalette(
		(u16*)(0x6898000 + 256*17*2),
		(u16*)(0x6898000 + 256*16*2));
		
	// text
	Decompress((void*)(0x6898000 + 256*16*2*2), font_pal_bin);

	// then allocate it for extended palette use
	vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE);

	// background image
	SUB_BG0_CR =
		BG_32x32 |
		BG_TILE_BASE(BACKGROUND_TILE_BASE) |
		BG_MAP_BASE(BACKGROUND_MAP_BASE) |
		BG_PRIORITY(BACKGROUND_PRIORITY) |
		BG_256_COLOR;
	if(wifi)
	{
		Decompress((void*)BG_TILE_RAM_SUB(BACKGROUND_TILE_BASE), wifibackground_bin);
		Decompress((void*)BG_MAP_RAM_SUB(BACKGROUND_MAP_BASE), wifibackground_map_bin);
	}
	else
	{
		Decompress((void*)BG_TILE_RAM_SUB(BACKGROUND_TILE_BASE), singlebackground_bin);
		Decompress((void*)BG_MAP_RAM_SUB(BACKGROUND_MAP_BASE), singlebackground_map_bin);
	}
	
	// blocks
	SUB_BG1_CR =
		BG_32x32 |
		BG_TILE_BASE(BLOCKS_TILE_BASE) |
		BG_MAP_BASE(BLOCKS_MAP_BASE) |
		BG_PRIORITY(BLOCKS_PRIORITY) |
		BG_256_COLOR;
	Decompress((void*)BG_TILE_RAM_SUB(BLOCKS_TILE_BASE), smalltiles_bin);
	u16* ptr = (u16*)BG_MAP_RAM_SUB(BLOCKS_MAP_BASE);
	for(int i = 0; i < 32*32; i++)
	{
		*ptr++ = TILE_BLANK;
	}
	
	// text
	SUB_BG2_CR =
		BG_32x32 |
		BG_TILE_BASE(TEXT_TILE_BASE) |
		BG_MAP_BASE(TEXT_MAP_BASE) |
		BG_PRIORITY(TEXT_PRIORITY) |
		BG_256_COLOR;
	Decompress((void*)BG_TILE_RAM_SUB(TEXT_TILE_BASE), font_bin);
	ptr = (u16*)BG_MAP_RAM_SUB(TEXT_MAP_BASE);
	for(int i = 0; i < 32*32; i++)
	{
		*ptr++ = 0;
	}
}

void FieldGraphics::CreateShadedPalette(u16* dest, u16* src)
{
	for(int i = 0; i < 256; i++)
		*dest++ = (*src++ >> 1) & 0x3DEF;
}

FieldGraphics::FieldGraphics()
:	mainBlockMap((u16*)BG_MAP_RAM(BLOCKS_MAP_BASE)),
	subBlockMap((u16*)BG_MAP_RAM_SUB(BLOCKS_MAP_BASE)),
	mainTextMap((u16*)BG_MAP_RAM(TEXT_MAP_BASE)),
	subTextMap((u16*)BG_MAP_RAM_SUB(TEXT_MAP_BASE)),
	lastChatLine(-1)
{
	memset(chatBuffer,0,sizeof(chatBuffer));
}

FieldGraphics::~FieldGraphics()
{
}

void FieldGraphics::SetScrollOffset(int scrollOffset)
{
	this->scrollOffset = scrollOffset;
	BG1_Y0 = -scrollOffset;
}

void FieldGraphics::DrawBlock(BaseBlock* block, int x, int y, bool shaded)
{
	y -= scrollOffset;
	ASSERT(block != NULL);
	ASSERT(x >= 0);
	ASSERT(x < 256);
	ASSERT(y >= 0);
	ASSERT(y < 256);
	u16 tile = block->GetTile() * 4;
	if(shaded)
		tile |= (1 << 12);
	u16* cell = mainBlockMap + x/8 + y/8 * BLOCKMAP_STRIDE;
	cell[0] = tile;
	cell[1] = tile+1;
	cell[BLOCKMAP_STRIDE] = tile+2;
	cell[BLOCKMAP_STRIDE+1] = tile+3;
}

void FieldGraphics::Clear(int x, int y)
{
	y -= scrollOffset;
	ASSERT(x >= 0);
	ASSERT(x < 256);
	ASSERT(y >= 0);
	ASSERT(y < 256);
	int tile = TILE_BLANK * 4;
	u16* cell = mainBlockMap + x/8 + y/8 * BLOCKMAP_STRIDE;
	cell[0] = tile;
	cell[1] = tile+1;
	cell[BLOCKMAP_STRIDE] = tile+2;
	cell[BLOCKMAP_STRIDE+1] = tile+3;
}

void FieldGraphics::DrawSmallField(int fieldNum, char* field, bool shaded)
{
	ASSERT(fieldNum >= 0);
	ASSERT(fieldNum < 4);
	ASSERT(field != NULL);
	u16* cell = subBlockMap + SMALL_FIELD_OFFSET + fieldNum * 8;
	for(int i = 0; i < 12*6; i++)
	{
		u16 tile = field[i];
		if(shaded)
			tile |= (1 << 12);
		*cell = tile;
		cell++;
		if((i+1) % 6 == 0)
		{
			cell += BLOCKMAP_STRIDE - 6;
		}
	}
}

void FieldGraphics::ClearSmallField(int fieldNum)
{
	ASSERT(fieldNum >= 0);
	ASSERT(fieldNum < 4);
	u16* cell = subBlockMap + SMALL_FIELD_OFFSET + fieldNum * 8;
	for(int i = 0; i < 12*6; i++)
	{
		*cell = TILE_BLANK;
		cell++;
		if((i+1) % 6 == 0)
		{
			cell += BLOCKMAP_STRIDE - 6;
		}
	}
}

void FieldGraphics::PrintPlayerInfo(PlayerInfo* player)
{
	ASSERT(player != NULL);

	// name
	char name[8+1];
	snprintf(name, sizeof(name), "%-8.8s", player->name);
	PrintSmall(subTextMap + NAME_TEXT_OFFSET + player->fieldNum * 8, name);
	
	// wins
	char wins[8+1];
	snprintf(wins, sizeof(wins), "%02i wins", player->wins);
	PrintSmall(subTextMap + WINS_TEXT_OFFSET + player->fieldNum * 8, wins);

	// level
	char level[8+1];
	snprintf(level, sizeof(level), "lv %02i", player->level+1);
	PrintSmall(subTextMap + LEVEL_TEXT_OFFSET + player->fieldNum * 8, level);

	// ready/typing
	char ready[8+1];
	snprintf(ready, sizeof(ready), (player->ready ? "READY" : (player->typing ? "typing" : "      ")));
	PrintSmall(subTextMap + READY_TEXT_OFFSET + player->fieldNum * 8, ready);

	// place
	const char* digit[] = {" ", "1", "2", "3", "4", "5"};
	const char* suffix[] = {"  ", "ST", "ND", "RD", "TH", "TH"};
	PrintLarge(
		subTextMap + PLACE_TEXT_OFFSET + player->fieldNum * 8,
		digit[player->place]);
	PrintSmall(
		subTextMap + PLACE_TEXT_OFFSET + player->fieldNum * 8 + TEXTMAP_STRIDE + 1,
		suffix[player->place]);
}

void FieldGraphics::ClearPlayer(PlayerInfo* player)
{
	ASSERT(player != NULL);
	
	// name
	ClearText(subTextMap + NAME_TEXT_OFFSET + player->fieldNum * 8, 8);
	// wins
	ClearText(subTextMap + WINS_TEXT_OFFSET + player->fieldNum * 8, 8);
	// level
	ClearText(subTextMap + LEVEL_TEXT_OFFSET + player->fieldNum * 8, 8);
	// ready
	ClearText(subTextMap + READY_TEXT_OFFSET + player->fieldNum * 8, 8);
	// place
	PrintLarge(subTextMap + PLACE_TEXT_OFFSET + player->fieldNum * 8," ");
	ClearText(subTextMap + PLACE_TEXT_OFFSET + player->fieldNum * 8 + TEXTMAP_STRIDE + 1, 2);
	// smallfield
	ClearSmallField(player->fieldNum);
	memset(player, 0, sizeof(PlayerInfo));
}

void FieldGraphics::PrintScore(int score)
{
	char str[9+1];
	snprintf(str, sizeof(str), "%9i", score);
	PrintLarge(mainTextMap + SCORE_TEXT_OFFSET, str);
}

void FieldGraphics::PrintTime(int ticks)
{
	int seconds = ticks/60;
	char str[9+1];
	snprintf(str, sizeof(str), "%6i:%02i", seconds / 60, seconds % 60);
	PrintLarge(mainTextMap + TIME_TEXT_OFFSET, str);
}

void FieldGraphics::PrintCountdown(int count)
{
	char str[2];
	if(count == 0)
	{
		snprintf(str, sizeof(str), " ");
	}
	else
	{
		snprintf(str, sizeof(str), "%i", count);
	}
	PrintLarge(mainTextMap + COUNTDOWN_TEXT_OFFSET, str);
}

void FieldGraphics::PrintLarge(u16* cell, const char* text)
{
	ASSERT(cell != NULL);
	ASSERT(text != NULL);
	
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
		else
		{
			tile = TILE_LARGE_SPACE;
		}
		cell[0] = tile;
		cell[TEXTMAP_STRIDE] = tile + LARGE_DIGIT_TILE_STRIDE;
		text++;
		cell++;
	}
}

void FieldGraphics::PrintSmall(u16* cell, const char* text)
{
	ASSERT(cell != NULL);
	ASSERT(text != NULL);
	
	while(*text != '\0')
	{
		char c = *text;
		int tile;
		if(c >= ' ' && c <= '~')
		{
			tile = c - ' ';
		}
		else
		{
			tile = 0;
		}
		*cell = tile;
		text++;
		cell++;
	}
}

void FieldGraphics::ClearTextLine(u16* cell)
{
	ASSERT(cell != NULL);
	
	ClearText(cell, TEXTMAP_STRIDE);
}

void FieldGraphics::ClearText(u16* cell, int length)
{
	ASSERT(cell != NULL);
	
	for(int i = 0;i<length;i++)
	{
		*cell = 0;
		cell++;
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
		PrintChat();
	}
}
/*void FieldGraphics::AddChat(char* text)
{
	int textLen = strlen(text);
	if(textLen > 32) // Too long, try to break it up
	{
		char* space = text;
		char* nextSpace;

		nextSpace = strchr(space, ' ');
		space = nextSpace+1;
		while(true)
		{
			if(nextSpace == NULL)
				break;

			if( (textLen - strlen(nextSpace)) < 32 )
				space = nextSpace+1;
			else
			{
				*(space-1) = '\0';
				AddChat(text); // Add first part
				AddChat(space); // And the second
				return;
			}
			nextSpace = strchr(space, ' ');
		}
		// We end up here if theres 33+ characters of junk and no space
		char buf[32+1];
		strncpy(buf, text, 32); // Get the first 32 characters
		buf[32] = '\0';
		AddChat(buf);
		AddChat(text+32);
	}
	else // A nice short line, add it to the buffer
	{
		lastChatLine++;
		if(lastChatLine == MAX_CHAT_LINES)
			lastChatLine = 0;
		strcpy(chatBuffer[lastChatLine], text);
		PrintChat();
	}
}*/

void FieldGraphics::PrintChat()
{
	u16* cursor = subTextMap;
	int curChatLine = lastChatLine + 1;
	for(int i=0;i<MAX_CHAT_LINES;i++)
	{
		ClearTextLine(cursor);
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
	PrintChat();
}
