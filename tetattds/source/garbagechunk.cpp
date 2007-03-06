#include "tetattds.h"
#include "garbagechunk.h"
#include "game.h"

GarbageChunk::GarbageChunk(int num, GarbageBlock* newgb, GarbageType type)
{
	gb = newgb;
	for(int i = 0; i < num; i++)
	{
		blocks[i] = new Garbage(type);
		blocks[i]->SetBlockType(g_game->GetRandomBlockType(false));
		blocks[i]->SetGB(gb);
	}
	numBlocks = num;
}

GarbageChunk::~GarbageChunk()
{

}

void GarbageChunk::SetGraphic(GarbageGraphicsType type)
{
	int tile = TILE_GARBAGE_BLUE;

	switch(type)
	{
	case GARBAGE_GRAPHICS_SINGLE:	tile += TILE_GARBAGE_SINGLE_OFFSET;	break;
	case GARBAGE_GRAPHICS_TOP:		tile += TILE_GARBAGE_TOP_OFFSET;	break;
	case GARBAGE_GRAPHICS_MIDDLE:	tile += TILE_GARBAGE_MIDDLE_OFFSET;	break;
	case GARBAGE_GRAPHICS_BOTTOM:	tile += TILE_GARBAGE_BOTTOM_OFFSET;	break;
	case GARBAGE_GRAPHICS_EVIL:		tile = TILE_GARBAGE_GRAY; 			break;
	// Shouldn't ever get here, added to get rid of warning
	case GARBAGE_GRAPHICS_NONE:		tile = 0;							break;
	}

	for(int i = 0; i < numBlocks; i++)
	{
		int offset;
		if(i == 0)                  { offset = 0; }
		else if(i == numBlocks - 1) { offset = 2; }
		else                        { offset = 1; }

		blocks[i]->SetGraphic(tile + offset);
	}
}

void GarbageChunk::SetChain(Chain* chain)
{
	for(int i = 0;i < numBlocks;i++)
		blocks[i]->SetChain(chain);
}

void GarbageChunk::Pop(GarbageGraphicsType nextType, int delay, int total)
{
	int tile = TILE_GARBAGE_BLUE;

	switch(nextType)
	{
	case GARBAGE_GRAPHICS_SINGLE: tile += TILE_GARBAGE_SINGLE_OFFSET; break;
	case GARBAGE_GRAPHICS_TOP:    tile += TILE_GARBAGE_TOP_OFFSET;    break;
	case GARBAGE_GRAPHICS_MIDDLE: tile += TILE_GARBAGE_MIDDLE_OFFSET; break;
	case GARBAGE_GRAPHICS_BOTTOM: tile += TILE_GARBAGE_BOTTOM_OFFSET; break;
	case GARBAGE_GRAPHICS_NONE:
		// in case of turning into normal blocks
		// we do a special loop and return
		for(int i = 0; i < numBlocks; i++)
		{
			blocks[i]->Pop(delay + numBlocks - i - 1, total, -1);
		}
		return;
	case GARBAGE_GRAPHICS_EVIL: break;//shouldn't ever happen
	}

	for(int i = 0; i < numBlocks; i++)
	{
		int offset;
		if(i == 0)                  { offset = 0; }
		else if(i == numBlocks - 1) { offset = 2; }
		else                        { offset = 1; }

		blocks[i]->Pop(delay + numBlocks - i - 1, total, tile + offset);
	}
}
