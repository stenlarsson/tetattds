#include "tetattds.h"
#include "block.h"
#include "game.h"
#include "playfield.h"

Block::Block(enum BlockType type)
{
	anim.Init(1, ANIM_STATIC);
	//Type equals framenumber in tileset
	anim.AddFrame(type+TILE_BLOCK_NORMAL_OFFSET, 0);
	this->type = type;
	state = BST_IDLE;
	nextState = BST_IDLE;
	stateDelay = -1;
	bNeedPopCheck = true;
	dropTimer = -1;
	chain = NULL;
	bStress = false;
	bStop = false;
	bPopped = false;
}

Block::~Block()
{
	if(chain != NULL)
	{
		chain->activeBlocks--;
	}
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
		if(bStress)
		{
			anim.Init(4, ANIM_LOOPING);
			anim.AddFrame(type+TILE_BLOCK_BOUNCE_3_OFFSET,5);
			anim.AddFrame(type+TILE_BLOCK_BOUNCE_2_OFFSET,5);
			anim.AddFrame(type+TILE_BLOCK_BOUNCE_1_OFFSET,5);
			anim.AddFrame(type+TILE_BLOCK_NORMAL_OFFSET,5);
		}
		else if(state == BST_FALLING)
		{
			anim.Init(4, ANIM_ONCE);
			anim.AddFrame(type+TILE_BLOCK_BOUNCE_3_OFFSET,5);
			anim.AddFrame(type+TILE_BLOCK_BOUNCE_2_OFFSET,5);
			anim.AddFrame(type+TILE_BLOCK_BOUNCE_1_OFFSET,5);
			anim.AddFrame(type+TILE_BLOCK_NORMAL_OFFSET,5);
		}
		state = BST_IDLE;
		stateDelay = -1;
		bNeedPopCheck = true;
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
		anim.Init(1, ANIM_STATIC);
		anim.AddFrame(type+TILE_BLOCK_NORMAL_OFFSET, 1);
		state = BST_MOVING;
		nextState = BST_POSTMOVE;
		stateDelay = 5;
		break;
	case BST_POSTMOVE:
		anim.Init(1, ANIM_STATIC);
		anim.AddFrame(type+TILE_BLOCK_NORMAL_OFFSET, 1);
		state = BST_POSTMOVE;
		nextState = BST_IDLE;
		stateDelay = 1;
		break;
	case BST_FLASH:
		anim.Init(2, ANIM_LOOPING);
		anim.AddFrame(type+TILE_BLOCK_NORMAL_OFFSET, 1);
		anim.AddFrame(type+TILE_BLOCK_FLASH_OFFSET, 4);
		state = BST_FLASH;
		nextState = BST_POP;
		stateDelay = g_game->GetLevelData()->flashTime;
		break;
	case BST_POP:
		anim.Init(1, ANIM_STATIC);
		anim.AddFrame(type+TILE_BLOCK_EYES_OFFSET, 1);
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
		anim.Init(1, ANIM_STATIC);
		anim.AddFrame(TILE_BLANK, 1);
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
