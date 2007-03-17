#include "tetattds.h"
#include "block.h"
#include "game.h"
#include "playfield.h"

Block::Block(enum BlockType type)
	: BaseBlock(
			AnimFrame(type+TILE_BLOCK_NORMAL_OFFSET),
			type)
{
}

Block::~Block()
{
	if(chain != NULL)
		chain->activeBlocks--;
}

void Block::Tick()
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
}

void Block::ChangeState(enum BlockState newState)
{
	switch(newState)
	{
	case BST_IDLE:
		{
			AnimFrame frames[] = {
				AnimFrame(type+TILE_BLOCK_BOUNCE_3_OFFSET,5),
				AnimFrame(type+TILE_BLOCK_BOUNCE_2_OFFSET,5),
				AnimFrame(type+TILE_BLOCK_BOUNCE_1_OFFSET,5),
				AnimFrame(type+TILE_BLOCK_NORMAL_OFFSET,5)
			};
			
			if(bStress)
				anim = Anim(ANIM_LOOPING, frames, COUNT_OF(frames));
			else if(state == BST_FALLING)
				anim = Anim(ANIM_ONCE, frames, COUNT_OF(frames));

			state = BST_IDLE;
			stateDelay = -1;
			bNeedPopCheck = true;
		}
		break;
	case BST_FALLING:
		state = BST_FALLING;
		dropTimer = BLOCK_DROP_TIMER;
		stateDelay = -1;
		break;
	case BST_HOVER:
		state = BST_HOVER;
		nextState = BST_FALLING;
		bNeedPopCheck = true; // needed for lateslip-technique
		break;
	case BST_MOVING:
		anim = Anim(type+TILE_BLOCK_NORMAL_OFFSET);
		state = BST_MOVING;
		nextState = BST_POSTMOVE;
		stateDelay = 5;
		break;
	case BST_POSTMOVE:
		anim = Anim(type+TILE_BLOCK_NORMAL_OFFSET);
		state = BST_POSTMOVE;
		nextState = BST_IDLE;
		stateDelay = 1;
		break;
	case BST_FLASH:
		{
			AnimFrame frames [] = {
				AnimFrame(type+TILE_BLOCK_NORMAL_OFFSET),
				AnimFrame(type+TILE_BLOCK_FLASH_OFFSET, 4),
			};
			anim = Anim(ANIM_LOOPING, frames, COUNT_OF(frames));

			state = BST_FLASH;
			nextState = BST_POP;
			stateDelay = g_game->GetLevelData()->flashTime;
		}
		break;
	case BST_POP:
		anim = Anim(type+TILE_BLOCK_EYES_OFFSET);
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
		anim = Anim(TILE_BLANK);
		state = BST_POP3;
		nextState = BST_DEAD;
		stateDelay = dieOffset;
		break;
	case BST_DEAD:
		state = BST_DEAD;
		stateDelay = -1;
		break;
	}
}
