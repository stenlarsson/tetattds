#include "tetattds.h"
#include "garbageblock.h"
#include "garbagechunk.h"
#include "garbage.h"
#include "game.h"
#include "playfield.h"

GarbageBlock::GarbageBlock(int num, GarbageType type)
	: lines(type != GARBAGE_CHAIN ? 1 : num),
	  numBlocks(type == GARBAGE_COMBO ? num : lines * PF_WIDTH),
	  chunks(),
	  numFalling(0),
	  state(BST_IDLE),
	  popDelay(0),
	  type(type)
{
	chunks.reserve(lines);
	switch(type)
	{
		case GARBAGE_COMBO:
			chunks.push_back(new GarbageChunk(num, this, type));
			break;
		case GARBAGE_CHAIN:
			for(int i = 0;i<num;i++)
				chunks.push_back(new GarbageChunk(PF_WIDTH, this, type));
			break;
		case GARBAGE_EVIL:
			chunks.push_back(new GarbageChunk(PF_WIDTH, this, type));
			break;
	}
}

GarbageBlock::~GarbageBlock()
{

}

void GarbageBlock::InitPop(Chain* chain)
{
	for(int i = 0;i < numBlocks;i++)
	{
		GetBlock(i)->SetPop();
		GetBlock(i)->SetChain(chain); // Needed so adjacent garbageblocks triggered by this one gets the same chain.
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

void GarbageBlock::Pop(int delay, int total, Chain* newchain)
{
	for(int i = 0; i < lines-1; i++)
		chunks[i]->Pop(
			GetGraphic(i, lines-1), delay+(lines-i-1)*PF_WIDTH, total);

	chunks[lines-1]->Pop(GARBAGE_GRAPHICS_NONE, delay, total);

	state = BST_POP;
	const LevelData* data = g_game->GetLevelData();
	popDelay = data->popStartOffset+data->popTime*total + data->flashTime;// + 1;
}

void GarbageBlock::SetGraphic()
{
	if(type == GARBAGE_EVIL)
		chunks[0]->SetGraphic(GARBAGE_GRAPHICS_EVIL);
	else
		for(int i = 0; i < lines; i++)
			chunks[i]->SetGraphic(GetGraphic(i, lines));
}

BaseBlock* GarbageBlock::GetBlock(int num)
{
	ASSERT(num >= 0 && num < numBlocks);

	return chunks[num/6]->GetBlock(num%6);
}

void GarbageBlock::Drop()
{
	numFalling++;
	if((numFalling == numBlocks || numFalling == 6) && state != BST_POP)
	{
		for(int i = 0;i < numBlocks;i++)
			GetBlock(i)->ChangeState(BST_FALLING);
		state = BST_FALLING;
		numFalling = 0;
	}
}

void GarbageBlock::Land()
{
	if(state == BST_FALLING)
	{
		for(int i = 0;i < numBlocks;i++)
			GetBlock(i)->ChangeState(BST_IDLE);
		state = BST_IDLE;
	}
}

void GarbageBlock::Hover(int delay)
{
	for(int i = 0;i < numBlocks;i++)
		GetBlock(i)->ChangeState(BST_IDLE);
}

void GarbageBlock::Tick()
{
	if(state == BST_IDLE && numFalling != 0)
	{
		for(int i = 0;i < numBlocks;i++)
			GetBlock(i)->ChangeState(BST_IDLE);
	}
	if(state == BST_POP && --popDelay <= 0)
	{
		state = BST_FALLING;
		for(int i = 0; i < lines-1; i++)
			chunks[i]->SetChain(NULL);
	}
	numFalling = 0;
}

void GarbageBlock::RemoveBlock()
{
	numBlocks--;
	if(numBlocks%6 == 0)
	{
		delete chunks.back();
		chunks.back() = NULL;
		chunks.pop_back();
		lines--;
	}
}
