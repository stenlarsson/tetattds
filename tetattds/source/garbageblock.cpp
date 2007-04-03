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
		blocks.push_back(new Garbage(type, g_game->GetRandomBlockType(false), this));
}

GarbageBlock::~GarbageBlock()
{
	delete_and_clear(blocks);
}

void GarbageBlock::InitPop(Chain* chain)
{
	for_each(blocks, std::mem_fun(&Garbage::SetPop));

	// Needed so adjacent garbageblocks triggered by this one gets the same chain.
	for_each(blocks, mem_fun_with(&Garbage::SetChain, chain));
}

/** Get the correct graphics style specific line in a block of a certain size. */
inline static GarbageGraphicsType GetLineGraphic(int line, int lineCount)
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

/** Adjust the tile number for block ends. */
static inline int GetBlockGraphic(GarbageGraphicsType type, int block, int blockCount)
{
	if(block == 0)
		return (int)type + 0;
	else if(block == blockCount - 1)
		return (int)type + 2;
	else
		return (int)type + 1;
}

/**
 * Get the appropriate tile for block index given the number of lines and
 * the number of blocks in each row.
 */
static inline int GetGraphic(int index, int lineCount, int blockCount)
{
	return GetBlockGraphic(
		GetLineGraphic(index / PF_WIDTH, lineCount), index % PF_WIDTH, blockCount);
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
	// int required here since we're stopping on less then 0
	int firstRemoved = std::max<size_t>(blocks.size(), PF_WIDTH) - PF_WIDTH;

	for(int i = blocks.size() - 1; i >= firstRemoved; i--)
		blocks[i]->Pop(delay++, total, GARBAGE_GRAPHIC_DISABLED);

	for(int i = firstRemoved - 1; i >= 0; i--)
		blocks[i]->Pop(
			delay++, total,
			GetGraphic(i, GetLines(blocks.size()) - 1, PF_WIDTH));

	state = BST_POP;
	const LevelData* data = g_game->GetLevelData();
	popDelay = data->popStartOffset+data->popTime*total + data->flashTime;// + 1;
}

void GarbageBlock::SetGraphic()
{
	if(type == GARBAGE_EVIL)
		for(unsigned int i = 0; i < PF_WIDTH; i++)
			blocks[i]->SetGraphic(
				GetBlockGraphic(GARBAGE_GRAPHICS_EVIL, i, PF_WIDTH));
	else
		for(unsigned int i = 0; i < blocks.size(); i++)
			blocks[i]->SetGraphic(
				GetGraphic(
					i, GetLines(blocks.size()),
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
	if((numFalling == blocks.size() || numFalling == PF_WIDTH) && state != BST_POP)
	{
		for_each(blocks, mem_fun_with(&Garbage::ChangeState, BST_FALLING));
		state = BST_FALLING;
		numFalling = 0;
	}
}

void GarbageBlock::Land()
{
	if(state == BST_FALLING)
	{
		for_each(blocks, mem_fun_with(&Garbage::ChangeState, BST_IDLE));
		state = BST_IDLE;
	}
}

void GarbageBlock::Hover(int delay)
{
	for_each(blocks, mem_fun_with(&Garbage::ChangeState, BST_IDLE));
}

void GarbageBlock::Tick()
{
	if(state == BST_IDLE && numFalling != 0)
		for_each(blocks, mem_fun_with(&Garbage::ChangeState, BST_IDLE));

	if(state == BST_POP && --popDelay <= 0)
	{
		state = BST_FALLING;
		for_each(blocks, mem_fun_with(&Garbage::SetChain, (Chain*)NULL));
	}
	numFalling = 0;
}

void GarbageBlock::RemoveBlock()
{
	blocks.back() = NULL;
	blocks.pop_back();
}
