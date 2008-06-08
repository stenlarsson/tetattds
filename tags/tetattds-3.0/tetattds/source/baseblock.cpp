#include "tetattds.h"
#include "baseblock.h"
#include "game.h"
#include "playfield.h"
#include "chain.h"

BaseBlock::BaseBlock(Anim const & anim, BlockType type, BlockState state, Chain* chain, bool needPopCheck)
	: anim(anim),
		type(type),
		state(state),
		popOffset(0),
		dieOffset(0),
		dropTimer(BLOCK_DROP_TIMER),
		stateDelay(-1),
		nextState(BST_IDLE),
		needPopCheck(needPopCheck),
		chain(NULL),
		popped(false)
{
	ASSERT(g_game != NULL);
	SetChain(chain);
}

BaseBlock::~BaseBlock()
{
}

int BaseBlock::GetTile() const
{
	return anim.GetFrame();
}

void BaseBlock::Tick()
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
	popped = false;
}

void BaseBlock::SetChain(Chain* newChain)
{
	if(chain != NULL)
		chain->activeBlocks--;
	chain = newChain;
	if(chain != NULL)
		chain->activeBlocks++;
}


