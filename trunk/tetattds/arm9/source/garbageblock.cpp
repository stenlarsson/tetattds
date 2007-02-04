#include "tetattds.h"
#include "garbageblock.h"
#include "garbagechunk.h"
#include "garbage.h"
#include "game.h"
#include "playfield.h"

GarbageBlock::GarbageBlock(int num, GarbageType type)
{
	for(int i = 0;i<MAX_GARBAGE_SIZE;i++)
		chunks[i] = NULL;
	
	numFalling = 0;
	state = BST_IDLE;
	popDelay = 0;
	numBlocks = 0;
	lines = 0;
	this->type = type;
	
	switch(type)
	{
		case GARBAGE_COMBO:
			chunks[0] = new GarbageChunk(num, this, GARBAGE_COMBO);
			lines = 1;
			numBlocks = num;
			break;
		case GARBAGE_CHAIN:
			if(num > MAX_GARBAGE_SIZE)
				num=MAX_GARBAGE_SIZE;

			for(int i = 0;i<num;i++)
			{
				chunks[i] = new GarbageChunk(PF_WIDTH, this, type);
				lines++;
			}
			numBlocks = PF_WIDTH*num;
			break;
		case GARBAGE_EVIL:
			chunks[0] = new GarbageChunk(PF_WIDTH, this, type);
			lines = 1;
			numBlocks = PF_WIDTH;
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

void GarbageBlock::Pop(int delay, int total, Chain* newchain)
{
	if(lines == 2)
	{
		chunks[0]->Pop(GARBAGE_GRAPHICS_SINGLE, delay+PF_WIDTH, total);
//		chunks[0]->SetChain(NULL);
	}
	else
	{
		for(int i = 0; i < lines-1; i++)
		{
			GarbageGraphicsType graphic;
			if(i == 0)
			{
				graphic = GARBAGE_GRAPHICS_TOP;
			}
			else if(i == lines - 2)
			{
				graphic = GARBAGE_GRAPHICS_BOTTOM;
			}
			else
			{
				graphic = GARBAGE_GRAPHICS_MIDDLE;
			}
			chunks[i]->Pop(graphic, delay+(lines-i-1)*PF_WIDTH, total);
//			chunks[i]->SetChain(NULL);
		}
	}
	chunks[lines-1]->Pop(GARBAGE_GRAPHICS_NONE, delay, total);
//	chunks[lines-1]->SetChain(newchain);

	state = BST_POP;
	const LevelData* data = g_game->GetLevelData();
	popDelay = data->popStartOffset+data->popTime*total + data->flashTime;// + 1;
}

void GarbageBlock::SetGraphic()
{
	if(lines == 1)
	{
		if(type == GARBAGE_EVIL)
			chunks[0]->SetGraphic(GARBAGE_GRAPHICS_EVIL);
		else
			chunks[0]->SetGraphic(GARBAGE_GRAPHICS_SINGLE);
	}
	else
	{
		for(int i = 0; i < lines; i++)
		{
			GarbageGraphicsType graphic;
			if(i == 0)
			{
				graphic = GARBAGE_GRAPHICS_TOP;
			}
			else if(i == lines - 1)
			{
				graphic = GARBAGE_GRAPHICS_BOTTOM;
			}
			else
			{
				graphic = GARBAGE_GRAPHICS_MIDDLE;
			}
			chunks[i]->SetGraphic(graphic);
		}
	}
}

BaseBlock* GarbageBlock::GetBlock(int num)
{
	if(num >= numBlocks)
		return (Garbage*)-1;

	if(numBlocks > 6)
		return chunks[num/6]->GetBlock(num%6);

	return chunks[0]->GetBlock(num);
}

int GarbageBlock::GetNum()
{
	return numBlocks;
}

void GarbageBlock::Drop()
{
	numFalling++;
	if((numFalling == numBlocks || numFalling == 6) && state != BST_POP)
	{
		for(int i = 0;i < numBlocks;i++)
		{
			GetBlock(i)->ChangeState(BST_FALLING);
		}
		state = BST_FALLING;
		numFalling = 0;
	}
}

void GarbageBlock::Land()
{
	if(state == BST_FALLING)
	{
		for(int i = 0;i < numBlocks;i++)
		{
			GetBlock(i)->ChangeState(BST_IDLE);
		}
		state = BST_IDLE;
	}
}

void GarbageBlock::Hover(int delay)
{
	for(int i = 0;i < numBlocks;i++)
	{
		GetBlock(i)->ChangeState(BST_IDLE);
	}
}

void GarbageBlock::Tick()
{
	if(state == BST_IDLE && numFalling != 0)
	{
		for(int i = 0;i < numBlocks;i++)
		{
			GetBlock(i)->ChangeState(BST_IDLE);
		}
	}
	if(state == BST_POP && --popDelay <= 0)
	{
		//for(int i = 0;i < numBlocks;i++)
		//{
		///	GetBlock(i)->ChangeState(BST_FALLING);
		//}
		state = BST_FALLING;
		for(int i = 0; i < lines-1; i++)
		{
			chunks[i]->SetChain(NULL);
		}
	}
	numFalling = 0;
}

void GarbageBlock::RemoveBlock()
{
	numBlocks--;
	if(numBlocks%6 == 0)
	{
		delete chunks[lines];
		chunks[lines] = NULL;
		lines--;
	}
}
