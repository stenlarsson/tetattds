#include "tetattds.h"
#include "garbageblock.h"
#include "garbage.h"
#include "game.h"
#include "playfield.h"
#include "util.h"

GarbageBlock::GarbageBlock(int num, GarbageType type)
	: blocks(),
	  numFalling(0),
	  state(BST_IDLE),
	  popDelay(0),
	  type(type)
{
	int lineCount = (type != GARBAGE_CHAIN) ? 1 : num;
	int blockCount = (type == GARBAGE_COMBO) ? num : lineCount * PF_WIDTH;
	blocks.reserve(blockCount);
	for(int i = 0; i<blockCount; i++)
	{
		blocks.push_back(new Garbage(type, this));
		blocks.back()->SetBlockType(g_game->GetRandomBlockType(false));
	}
}

GarbageBlock::~GarbageBlock()
{
	delete_and_clear(blocks);
}

void GarbageBlock::InitPop(Chain* chain)
{
	for(unsigned int i = 0; i < blocks.size(); i++)
	{
		blocks[i]->SetPop();
		blocks[i]->SetChain(chain); // Needed so adjacent garbageblocks triggered by this one gets the same chain.
	}
}

inline static GarbageGraphicsType GetGraphic(int line, int lineCount)
{      
  if (lineCount == 1)
		return GARBAGE_GRAPHICS_SINGLE;
	else if (line == 0)
		return GARBAGE_GRAPHICS_TOP;
	else if (line == lineCount - 1)
		return GARBAGE_GRAPHICS_BOTTOM;
	else
		return GARBAGE_GRAPHICS_MIDDLE;
}

static inline int GetChunkGraphic(GarbageGraphicsType type, int block, int blockCount)
{
	if(type == GARBAGE_GRAPHICS_NONE)
		return GARBAGE_GRAPHIC_DISABLED;
	else if(block == 0)
		return (int)type + 0;
	else if(block == blockCount - 1)
		return (int)type + 2;
	else
		return (int)type + 1;
}

/**
 * Return the number of lines given the block count.
 * 3,4,5 => 1, n*6 => n
 */
static inline int GetLines(int count)
{
	return (count + PF_WIDTH-1) / PF_WIDTH;
}

void GarbageBlock::Pop(int delay, int total, Chain* newchain)
{
	int i;
	for(i = 0; i < (int)blocks.size()-PF_WIDTH; i++)
		blocks[i]->Pop(
			delay + blocks.size() - i - 1,
			total,
			GetChunkGraphic(
				GetGraphic(i/PF_WIDTH, GetLines(blocks.size())-1),
				i % PF_WIDTH,
				blocks.size() < PF_WIDTH ? blocks.size() : PF_WIDTH));

	for( ; i < (int)blocks.size(); i++)
		blocks[i]->Pop(
			delay + blocks.size() - i - 1, total, GARBAGE_GRAPHIC_DISABLED);

	state = BST_POP;
	const LevelData* data = g_game->GetLevelData();
	popDelay = data->popStartOffset+data->popTime*total + data->flashTime;// + 1;
}

void GarbageBlock::SetGraphic()
{
	if(type == GARBAGE_EVIL)
		for(int i = 0; i < PF_WIDTH; i++)
			blocks[i]->SetGraphic(GetChunkGraphic(GARBAGE_GRAPHICS_EVIL, i, PF_WIDTH));
	else
		for(unsigned int i = 0; i < blocks.size(); i++)
			blocks[i]->SetGraphic(
				GetChunkGraphic(
					GetGraphic(i/PF_WIDTH, GetLines(blocks.size())),
					i % PF_WIDTH,
					blocks.size() < PF_WIDTH ? blocks.size() : PF_WIDTH));
}

BaseBlock* GarbageBlock::GetBlock(int num)
{
	ASSERT(num >= 0 && num < (int)blocks.size());

	return blocks[num];
}

void GarbageBlock::Drop()
{
	numFalling++;
	if((numFalling == (int)blocks.size() || numFalling == PF_WIDTH) && state != BST_POP)
	{
		for(unsigned int i = 0; i < blocks.size(); i++)
			blocks[i]->ChangeState(BST_FALLING);
		state = BST_FALLING;
		numFalling = 0;
	}
}

void GarbageBlock::Land()
{
	if(state == BST_FALLING)
	{
		for(unsigned int i = 0; i < blocks.size(); i++)
			blocks[i]->ChangeState(BST_IDLE);
		state = BST_IDLE;
	}
}

void GarbageBlock::Hover(int delay)
{
	for(unsigned int i = 0; i < blocks.size(); i++)
		blocks[i]->ChangeState(BST_IDLE);
}

void GarbageBlock::Tick()
{
	if(state == BST_IDLE && numFalling != 0)
	{
		for(unsigned int i = 0; i < blocks.size(); i++)
			blocks[i]->ChangeState(BST_IDLE);
	}
	if(state == BST_POP && --popDelay <= 0)
	{
		state = BST_FALLING;
		for(int i = 0; i < (int)blocks.size()-PF_WIDTH; i++)
			blocks[i]->SetChain(NULL);
	}
	numFalling = 0;
}

void GarbageBlock::RemoveBlock()
{
	blocks.back() = NULL;
	blocks.pop_back();
}
