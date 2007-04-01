#include "tetattds.h"
#include "garbagechunk.h"
#include "game.h"

GarbageChunk::GarbageChunk(int num, GarbageBlock* newgb, GarbageType type)
	: numBlocks(num)
{
	for(int i = 0; i < num; i++)
	{
		blocks[i] = new Garbage(type, newgb);
		blocks[i]->SetBlockType(g_game->GetRandomBlockType(false));
	}
}

GarbageChunk::~GarbageChunk()
{
}

int GetGraphic(GarbageGraphicsType type, int block, int blockCount)
{
	if(block == 0)
		return (int)type + 0;
	else if(block == blockCount - 1)
		return (int)type + 2;
	else
		return (int)type + 1;
}

void GarbageChunk::SetGraphic(GarbageGraphicsType type)
{
	for(int i = 0; i < numBlocks; i++)
		blocks[i]->SetGraphic(GetGraphic(type, i, numBlocks));
}

void GarbageChunk::SetChain(Chain* chain)
{
	for(int i = 0;i < numBlocks;i++)
		blocks[i]->SetChain(chain);
}

void GarbageChunk::Pop(GarbageGraphicsType nextType, int delay, int total)
{
	ASSERT(nextType != GARBAGE_GRAPHICS_EVIL);
	
	if (nextType == GARBAGE_GRAPHICS_NONE)
	{
		// in case of turning into normal blocks
		// we do a special loop and return
		for(int i = 0; i < numBlocks; i++)
			blocks[i]->Pop(delay + numBlocks - i - 1, total, -1);
	}
	else
	{
		for(int i = 0; i < numBlocks; i++)
			blocks[i]->Pop(
				delay + numBlocks - i - 1, total, GetGraphic(nextType, i, numBlocks));
	}
}
