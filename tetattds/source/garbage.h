#pragma once

enum GarbageType
{
	GARBAGE_COMBO,
	GARBAGE_CHAIN,
	GARBAGE_EVIL
};

#include "baseblock.h"
class GarbageBlock;

#define GARBAGE_GRAPHIC_DISABLED (-1)

/**
 * One block of garbage
 */
class Garbage : public BaseBlock
{
public:
	Garbage(GarbageType type, BlockType blockType, GarbageBlock * gb);
	~Garbage();

	virtual void Drop();
	virtual void Land();
	virtual void Hover(int delay);
	virtual void Pop(int num, int total, int nextGraph);

	virtual void Tick();

	void SetGraphic(int newGraphic);
	GarbageBlock* GetGB() const { return gb; }

	BaseBlock* CreateBlock();

	virtual void ChangeState(BlockState newState);
	
	inline bool IsOtherGarbageType(BaseBlock *block) const {
		return block->GetType() ==
			(type == BLC_GARBAGE ? BLC_EVILGARBAGE : BLC_GARBAGE);
	}

private:
	const BlockType blockType;
	GarbageBlock* gb;
	int nextGraphic;
	bool bWantToDrop;
};
