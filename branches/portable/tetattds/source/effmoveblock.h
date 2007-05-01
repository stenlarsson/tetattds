#pragma once

#include "effect.h"
#include "sprite.h"
#include "baseblock.h"

class EffMoveBlock : public Effect
{
public:
	EffMoveBlock(enum Direction dir, BaseBlock* block, int pos);
	~EffMoveBlock();

	void Draw();
	void Tick();

private:
	Sprite* sprite;
	BaseBlock* block;
	int stepX;
	int stepY;
};
