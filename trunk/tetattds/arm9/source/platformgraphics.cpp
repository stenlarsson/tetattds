#include <nds.h>
#include "tetattds.h"
#include "PlatformGraphics.h"
#include "baseblock.h"
#include "playfield.h"
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

void PlatformGraphics::InitMainScreen()
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
	vramSetBankB(VRAM_B_MAIN_SPRITE_0x06400000);

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
}

void PlatformGraphics::InitSubScreen(bool wifi)
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
	BG_PALETTE_SUB[255] = RGB15(31,0,0);

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

void PlatformGraphics::CreateShadedPalette(u16* dest, u16* src)
{
	for(int i = 0; i < 256; i++)
		*dest++ = (*src++ >> 1) & 0x3DEF;
}

PlatformGraphics::PlatformGraphics()
:	mainBlockMap((u16*)BG_MAP_RAM(BLOCKS_MAP_BASE)),
	subBlockMap((u16*)BG_MAP_RAM_SUB(BLOCKS_MAP_BASE)),
	mainTextMap((u16*)BG_MAP_RAM(TEXT_MAP_BASE)),
	subTextMap((u16*)BG_MAP_RAM_SUB(TEXT_MAP_BASE))
{
	memset(chatBuffer,0,sizeof(chatBuffer));
}

PlatformGraphics::~PlatformGraphics()
{
	delete marker;
	delete touchMarker;
}

void PlatformGraphics::DrawField(PlayField *pf, int x, int y, int tile, bool shaded)
{
	tile <<= 2;
	if (shaded)
		tile |= (1 << 12);

	u16* cell = mainBlockMap + x/8 + y/8 * BLOCKMAP_STRIDE;
	cell[0] = tile;
	cell[1] = tile+1;
	cell[BLOCKMAP_STRIDE] = tile+2;
	cell[BLOCKMAP_STRIDE+1] = tile+3;
}

void PlatformGraphics::Draw(PlayField *pf)
{
	BG1_Y0 = -(int)pf->GetScrollOffset();
	
	PrintScore(pf->GetScore());
	PrintTime(pf->GetTimeTicks());
	PrintStopTime(pf->GetScrollPause());

	DrawFields(pf);
	DrawEffects();
	DrawMarkers(pf);
	PrintChat();
}

void PlatformGraphics::DrawSmallField(int fieldNum, char* field, bool shaded)
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

void PlatformGraphics::ClearSmallField(int fieldNum)
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

void PlatformGraphics::PrintPlayerInfo(PlayerInfo* player)
{
	ReallyPrintPlayerInfo(player);
}

void PlatformGraphics::ClearPlayer(PlayerInfo* player)
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
	PrintLarge(PLACE_TEXT_OFFSET + player->fieldNum * 8," ", true);
	ClearText(subTextMap + PLACE_TEXT_OFFSET + player->fieldNum * 8 + TEXTMAP_STRIDE + 1, 2);
	// smallfield
	ClearSmallField(player->fieldNum);
	memset(player, 0, sizeof(PlayerInfo));
}

void PlatformGraphics::PrintLarge(uint32_t cell, const char* text, bool subScreen)
{
	ASSERT(text != NULL);
	
	u16* map = subScreen ? subTextMap : mainTextMap;
	
	while(*text != '\0')
	{
		int tile = LargeCharTile(*text);
		map[cell] = tile;
		map[cell+TEXTMAP_STRIDE] = tile + LARGE_DIGIT_TILE_STRIDE;
		text++;
		cell++;
	}
}

void PlatformGraphics::PrintSmall(uint32_t cell, const char* text)
{
	ASSERT(text != NULL);
	
	while(*text != '\0')
	{
		int tile = SmallCharTile(*text);
		subTextMap[cell] = tile;
		text++;
		cell++;
	}
}

void PlatformGraphics::ClearTextLine(uint32_t cell)
{
	ClearText(subTextMap+cell, TEXTMAP_STRIDE);
}

void PlatformGraphics::ClearText(u16* cell, int length)
{
	ASSERT(cell != NULL);
	
	for(int i = 0;i<length;i++)
	{
		*cell = 0;
		cell++;
	}
}

void PlatformGraphics::AddChat(char* text)
{
	FieldGraphics::AddChat(text);
	PrintChat();
}

void PlatformGraphics::PrintChat()
{
	uint32_t cursor = 0;
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

void PlatformGraphics::ClearChat()
{
	memset(chatBuffer, 0, sizeof(chatBuffer));
	lastChatLine = -1;
	PrintChat();
}
