#pragma once

#include "baseblock.h"

class Block : public BaseBlock
{
public:
	Block(BlockType type, BlockState state = BST_IDLE, Chain* chain = NULL, bool needPopCheck = true);
	virtual ~Block();

	virtual void Drop();
	virtual void Land();
	virtual void Hover(int delay);
	virtual void Pop(int num, int total);
	
	void Move();
	void Stress(bool stress);
	void Stop(bool stop);

private:
	virtual void ChangeState(BlockState newState);
	
	bool stress;
	bool stop;
};
