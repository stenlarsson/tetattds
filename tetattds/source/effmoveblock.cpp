#include "tetattds.h"
#include "effmoveblock.h"

EffMoveBlock::EffMoveBlock(enum Direction dir, BaseBlock* block, int x, int y)
{
	Anim anim(block->GetTile());
	sprite =  new Sprite(x, y, BLOCKS_PRIORITY, SSIZE_16x16, anim, false, false);
	XOffset = x;
	YOffset = y;
	duration = 5;
	stepX = 0;
	stepY = 0;
	switch(dir)
	{
	case DIR_LEFT:
		stepX = -BLOCKSIZE/duration;
		break;
	case DIR_RIGHT:
		stepX = BLOCKSIZE/duration;
		break;
	case DIR_UP:
		stepY = -BLOCKSIZE/duration;
		break;
	case DIR_DOWN:
		stepY = BLOCKSIZE/duration;
		break;
	}
}

EffMoveBlock::~EffMoveBlock()
{
	delete sprite;
}

void EffMoveBlock::Draw()
{
	sprite->Draw();
}

void EffMoveBlock::Tick()
{
	duration--;
	sprite->Move(stepX,stepY);
}
