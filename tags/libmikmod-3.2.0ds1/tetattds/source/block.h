#pragma once

#include "baseblock.h"

enum StressState
{
	SS_NORMAL,
	SS_STOP,
	SS_STRESS,
};

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
	void SetStress(StressState stress);

private:
	virtual void ChangeState(BlockState newState);

	StressState stress;
};
