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


