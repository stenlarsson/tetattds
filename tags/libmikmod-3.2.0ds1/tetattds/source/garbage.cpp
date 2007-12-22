#include "tetattds.h"
#include "garbage.h"
#include "garbageblock.h"
#include "block.h"
#include "game.h"
#include "playfield.h"

Garbage::Garbage(GarbageType type, BlockType blockType, GarbageBlock * gb)
	: BaseBlock(
			AnimFrame(GARBAGE_GRAPHICS_SINGLE),
	  	type == GARBAGE_EVIL ? BLC_EVILGARBAGE : BLC_GARBAGE,
			BST_IDLE,
			NULL,
			false),
		blockType(blockType),
		gb(gb),
		nextGraphic(GARBAGE_GRAPHIC_DISABLED),
		bWantToDrop(false)
{
}

Garbage::~Garbage()
{
	SetChain(NULL);
	gb->RemoveBlock();
	gb = NULL;
}

void Garbage::SetGraphic(int newGraphic)
{
	anim = Anim(newGraphic);
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

void Garbage::ChangeState(BlockState newState)
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
		anim = Anim(nextGraphic == GARBAGE_GRAPHIC_DISABLED ? blockType : nextGraphic);
		state = BST_POP3;
		nextState = BST_DEAD;
		stateDelay = dieOffset;
		break;
	case BST_DEAD:
		if(nextGraphic == GARBAGE_GRAPHIC_DISABLED)
		{
			state = BST_DEAD;
		}
		else
		{
			nextGraphic = GARBAGE_GRAPHIC_DISABLED;
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
	BaseBlock::Tick();
	bWantToDrop = false;
}

BaseBlock* Garbage::CreateBlock()
{
	return new Block(blockType, BST_FALLING, GetChain(), false);
}
