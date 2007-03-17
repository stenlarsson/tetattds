#pragma once

enum GarbageType
{
	GARBAGE_COMBO = 0x10000,
	GARBAGE_CHAIN = 0x20000,
	GARBAGE_EVIL = 0x40000
};

#include "baseblock.h"
#include "garbageblock.h"



/**
 * One block of garbage
 */
class Garbage : public BaseBlock
{
  public:
	Garbage(GarbageType type, GarbageBlock * gb);
	~Garbage();

	void Drop();
	void Land();
	void Hover(int delay);
	void Pop(int num, int total, int nextGraph);

	void Tick();

	void SetBlockType(enum BlockType newType);
	void SetGraphic(int newGraphic);
	GarbageBlock* GetGB(){ return gb; }

	void SetChain(Chain* newChain) { if(chain != NULL) chain->activeBlocks--; chain = newChain; if(chain != NULL) chain->activeBlocks++;}

	BaseBlock* CreateBlock();

	void ChangeState(enum BlockState newState);

  private:
	enum BlockType blockType;
	GarbageBlock* gb;
	int nextGraphic;
	bool bWantToDrop;
};
