#pragma once

#include "effect.h"
#include "sprite.h"
#include "baseblock.h"

class EffMoveBlock : public Effect
{
  public:
	EffMoveBlock(enum Direction dir, BaseBlock* block, int x, int y);
	~EffMoveBlock();

	void Draw();
	void Tick();

	void SetOffset(int x, int y) { sprite->Move(x,y); }

  private:
	Sprite* sprite;
	BaseBlock* block;
	int stepX;
	int stepY;
};
