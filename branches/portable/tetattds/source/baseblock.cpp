#include "tetattds.h"
#include "baseblock.h"
#include "game.h"
#include "playfield.h"

BaseBlock::BaseBlock(Anim const & anim, BlockType type)
	: anim(anim),
		type(type),
		state(BST_IDLE),
		bNeedPopCheck(true),
		popOffset(0),
		dieOffset(0),
		dropTimer(-1),
		stateDelay(-1),
		nextState(BST_IDLE),
		chain(NULL),
		bPopped(false),
		bStress(false),
		bStop(false)
{
	ASSERT(g_game != NULL);
}

BaseBlock::~BaseBlock()
{
}

int BaseBlock::GetTile()
{
	int tile = anim.GetFrame();
#ifdef DEBUG
	// Makes blockgraphic tied to some other attribute, useful when debugging
	if(keysHeld() & KEY_L)
	{
		if(keysHeld() & KEY_UP)			// Shows current state of block
			tile = state*8;
		else if(keysHeld() & KEY_DOWN)		// Shows the statedelay
		{
			if(stateDelay == -1)
				tile = TILE_EGG;
			else
				tile = (stateDelay>13?13:stateDelay)*8;
		}
		else								// Shows which chain the block is involved in
			if(chain)
				tile = chain->chainNum*8;
			else
				tile = TILE_EGG;
	}
#endif//DEBUG
	return tile;
}

void BaseBlock::Drop()
{
	ChangeState(BST_FALLING);
}

void BaseBlock::Land()
{
	ChangeState(BST_IDLE);
}

void BaseBlock::Move()
{
	ChangeState(BST_MOVING);
}

void BaseBlock::Hover(int delay)
{
	stateDelay = delay;
	ChangeState(BST_HOVER);
}

void BaseBlock::Pop(int num, int total)
{
	ChangeState(BST_FLASH);
	const LevelData* data = g_game->GetLevelData();
	popOffset = data->popStartOffset+data->popTime*num;
	dieOffset = data->popStartOffset+data->popTime*total - popOffset - 1;
}

void BaseBlock::Stop(bool stop)
{
	if(type == BLC_GARBAGE || type == BLC_EVILGARBAGE)
		return;

	if(!bStop && stop && state == BST_IDLE)
	{
		anim = Anim(type+TILE_BLOCK_BOUNCE_3_OFFSET);
		bStress = false;
		bStop = true;
	}
	else if((bStop || bStress) && !stop && state == BST_IDLE)
	{
		anim = Anim(type+TILE_BLOCK_NORMAL_OFFSET);
		;
		bStress = false;
		bStop = false;
	}
}

void BaseBlock::Stress(bool stress)
{
	if(type == BLC_GARBAGE || type == BLC_EVILGARBAGE)
		return;

	if(!bStress && stress && state == BST_IDLE)
	{
		AnimFrame frames[] = {
			AnimFrame(type+TILE_BLOCK_BOUNCE_3_OFFSET, 5),
			AnimFrame(type+TILE_BLOCK_BOUNCE_2_OFFSET, 5),
			AnimFrame(type+TILE_BLOCK_BOUNCE_1_OFFSET, 5),
			AnimFrame(type+TILE_BLOCK_NORMAL_OFFSET, 5),
		};
		anim = Anim(ANIM_LOOPING, frames, COUNT_OF(frames));
		;
		bStop = false;
		bStress = true;
	}
	else if((bStress || bStop) && !stress && state == BST_IDLE)
	{
		anim = Anim(type+TILE_BLOCK_NORMAL_OFFSET);
		;
		bStop = false;
		bStress = false;
	}
}
