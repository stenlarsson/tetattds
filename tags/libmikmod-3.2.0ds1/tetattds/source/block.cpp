#include "tetattds.h"
#include "block.h"
#include "game.h"
#include "playfield.h"

Block::Block(
	BlockType type, BlockState state, Chain* chain, bool needPopCheck)
	: BaseBlock(
			AnimFrame(type+TILE_BLOCK_NORMAL_OFFSET),
			type,
			state,
			chain,
			needPopCheck),
		stress(SS_NORMAL)
{
}

Block::~Block()
{
	SetChain(NULL);
}

void Block::Drop()
{
	ChangeState(BST_FALLING);
}

void Block::Land()
{
	ChangeState(BST_IDLE);
}

void Block::Hover(int delay)
{
	stateDelay = delay;
	ChangeState(BST_HOVER);
}

void Block::Pop(int num, int total)
{
	ChangeState(BST_FLASH);
	const LevelData* data = g_game->GetLevelData();
	popOffset = data->popStartOffset+data->popTime*num;
	dieOffset = data->popStartOffset+data->popTime*total - popOffset - 1;
}

void Block::Move()
{
	ChangeState(BST_MOVING);
}

void Block::SetStress(StressState newStress)
{
	if(state != BST_IDLE || stress == newStress)
		return;

	switch(newStress)
	{
	case SS_NORMAL:
		anim = Anim(type+TILE_BLOCK_NORMAL_OFFSET);
		break;
	case SS_STOP:
		anim = Anim(type+TILE_BLOCK_BOUNCE_3_OFFSET);
		break;
	case SS_STRESS:
		{
			AnimFrame frames[] = {
				AnimFrame(type+TILE_BLOCK_BOUNCE_3_OFFSET, 5),
				AnimFrame(type+TILE_BLOCK_BOUNCE_2_OFFSET, 5),
				AnimFrame(type+TILE_BLOCK_BOUNCE_1_OFFSET, 5),
				AnimFrame(type+TILE_BLOCK_NORMAL_OFFSET, 5),
			};
			anim = Anim(ANIM_LOOPING, frames, COUNT_OF(frames));			
		}
		break;
	}
	stress = newStress;
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
			
			if(stress == SS_STRESS)
				anim = Anim(ANIM_LOOPING, frames, COUNT_OF(frames));
			else if(state == BST_FALLING)
				anim = Anim(ANIM_ONCE, frames, COUNT_OF(frames));

			state = BST_IDLE;
			stateDelay = -1;
			ForcePopCheck();
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
		ForcePopCheck(); // needed for lateslip-technique
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
