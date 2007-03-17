#include "tetattds.h"
#include "garbage.h"
#include "garbageblock.h"
#include "block.h"
#include "game.h"
#include "playfield.h"

Garbage::Garbage(GarbageType type, GarbageBlock * gb)
	: BaseBlock(
			AnimFrame(GARBAGE_GRAPHICS_SINGLE),
	  	type == GARBAGE_EVIL ? BLC_EVILGARBAGE : BLC_GARBAGE),
		blockType(BLC_GARBAGE),
		gb(gb),
		nextGraphic(-1),
		bWantToDrop(false)
{
	BaseBlock::bNeedPopCheck = false;
}

Garbage::~Garbage()
{
	if(chain != NULL)
		chain->activeBlocks--;
	gb->RemoveBlock();
	gb = NULL;
}

void Garbage::SetGraphic(int newGraphic)
{
	anim = Anim(newGraphic);
}

void Garbage::SetBlockType(enum BlockType newType)
{
	blockType = newType;
}

void Garbage::Drop()
{
	if(!bWantToDrop)
	{
		gb->Drop();
		bWantToDrop = true;
	}
	//ChangeState(BST_FALLING);
}

void Garbage::Land()
{
	gb->Land();
	//ChangeState(BST_IDLE);
}

void Garbage::Hover(int delay)
{
	gb->Hover(delay);
	//stateDelay = delay;
	//ChangeState(BST_HOVER);
}

void Garbage::Pop(int num, int total, int nextGraph)
{
	ChangeState(BST_FLASH);
	const LevelData* data = g_game->GetLevelData();
	popOffset = data->popStartOffset+data->popTime*num;
	dieOffset = data->popStartOffset+data->popTime*total - popOffset - 1;
	nextGraphic = nextGraph;
}

void Garbage::ChangeState(enum BlockState newState)
{
	switch(newState)
	{
	case BST_IDLE:
		state = BST_IDLE;
		stateDelay = -1;
		break;
	case BST_FALLING:
		state = BST_FALLING;
		dropTimer = BLOCK_DROP_TIMER;
		stateDelay = -1;
		break;
	case BST_HOVER:
		state = BST_HOVER;
		nextState = BST_FALLING;
		break;
	case BST_FLASH:
		{		
			AnimFrame frames[] = {
				AnimFrame(type + TILE_GARBAGE_FLASH_1_OFFSET, 1),
				AnimFrame(type + TILE_GARBAGE_FLASH_2_OFFSET, 4),				
			};
			anim = Anim(ANIM_LOOPING, frames, COUNT_OF(frames));
			state = BST_FLASH;
			nextState = BST_POP;
			stateDelay = g_game->GetLevelData()->flashTime;
		}
		break;
	case BST_POP:
		anim = Anim(type + TILE_GARBAGE_FLASH_1_OFFSET);
		state = BST_POP;
		nextState = BST_POP2;
		stateDelay = popOffset;
		break;
	case BST_POP2:
		state = BST_POP2;
		nextState = BST_POP3;
		stateDelay = 1;
		break;
	case BST_POP3:
		anim = Anim(nextGraphic == -1 ? blockType : nextGraphic);
		state = BST_POP3;
		nextState = BST_DEAD;
		stateDelay = dieOffset;
		break;
	case BST_DEAD:
		if(nextGraphic == -1)
		{
			state = BST_DEAD;
		}
		else
		{
			nextGraphic = -1;
			state = BST_FALLING;
			dropTimer = BLOCK_DROP_TIMER;
		}
		stateDelay = -1;
		break;
	case BST_MOVING: // Garbage doesn't "move", added to get rid of warning
	case BST_POSTMOVE:
		break;
	}
}

void Garbage::Tick()
{
	if(dropTimer > 0)
		dropTimer--;

	if(stateDelay > 0)
		stateDelay--;

	if(stateDelay == 0)
	{
		stateDelay = -1;
		ChangeState(nextState);
	}
	anim.Tick();
	bPopped = false;
	bWantToDrop = false;
}

BaseBlock* Garbage::CreateBlock()
{
	BaseBlock* newBlock = new Block(blockType);
	newBlock->SetChain(chain);
	newBlock->PopCheck(); // Withouth this, BIG NOO~~!
	newBlock->Drop();
	return newBlock;
}
