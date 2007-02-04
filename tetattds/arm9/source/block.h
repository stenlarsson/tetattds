#pragma once

#include "baseblock.h"

class Block : public BaseBlock
{
  public:
	Block(enum BlockType Type);
	~Block();

	void Tick();

  private:
	void ChangeState(enum BlockState newState);
};
