#pragma once

#include "chain.h"
#include "garbage.h"

class GarbageBlock;

/**
 * One row of garbage
 */
class GarbageChunk
{
  public:
	GarbageChunk(int num, GarbageBlock* newGB, GarbageType type);
	~GarbageChunk();

	void SetGraphic(GarbageGraphicsType type);
	void SetChain(Chain* chain);
	void Pop(GarbageGraphicsType nextType, int delay, int total);

	Garbage* GetBlock(int num) { return blocks[num]; }

  private:
	Garbage* blocks[6];
	int numBlocks;
	GarbageBlock* gb;
};
