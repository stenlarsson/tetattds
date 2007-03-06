#include "tetattds.h"
#include "garbage.h"
#include "garbageblock.h"
#include "block.h"
#include "game.h"
#include "playfield.h"

Garbage::Garbage(GarbageType type)
{
	if(type == GARBAGE_EVIL)
		this->type = BLC_EVILGARBAGE;
	else
		this->type = BLC_GARBAGE;
	blockType = BLC_GARBAGE;
	bNeedPopCheck = false;
	state = BST_IDLE;
	nextState = BST_IDLE;
	stateDelay = -1;
	dropTimer = -1;
	chain = NULL;
	nextGraphic = -1;
	bStress = false;
	bStop = false;
	bPopped = false;
	bWantToDrop = false;
}

Garbage::~Garbage()
{
	if(chain != NULL)
		chain->activeBlocks--;
	gb->RemoveBlock();
}

void Garbage::SetGraphic(int newGraphic)
{
	anim.Init(1, ANIM_STATIC);
	anim.AddFrame(newGraphic,1);
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
		anim.Init(2, ANIM_LOOPING);
		anim.AddFrame(type + TILE_GARBAGE_FLASH_1_OFFSET, 1);
		anim.AddFrame(type + TILE_GARBAGE_FLASH_2_OFFSET, 4);
		state = BST_FLASH;
		nextState = BST_POP;
		stateDelay = g_game->GetLevelData()->flashTime;
		break;
	case BST_POP:
		anim.Init(1, ANIM_STATIC);
		anim.AddFrame(type + TILE_GARBAGE_FLASH_1_OFFSET, 1);
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
		if(nextGraphic == -1)
		{
			anim.Init(1, ANIM_STATIC);
			anim.AddFrame(blockType, 1);
		}
		else
		{
			anim.Init(1, ANIM_STATIC);
			anim.AddFrame(nextGraphic, 1);
		}
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
