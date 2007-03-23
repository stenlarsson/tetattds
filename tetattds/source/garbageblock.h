#pragma once

#include "baseblock.h"
#include "garbage.h"

class GarbageChunk;
class Garbage;
class Chain;

enum GarbageGraphicsType
{
	GARBAGE_GRAPHICS_NONE,
	GARBAGE_GRAPHICS_SINGLE,
	GARBAGE_GRAPHICS_TOP,
	GARBAGE_GRAPHICS_MIDDLE,
	GARBAGE_GRAPHICS_BOTTOM,
	GARBAGE_GRAPHICS_EVIL
};

/**
 * Several rows of garbage
 */
class GarbageBlock
{
  public:
	GarbageBlock(int num, GarbageType type);
	~GarbageBlock();

	void Drop();
	void Land();
	void Hover(int delay);
	void InitPop(Chain* chain);
	void Pop(int delay, int total, Chain* newchain);
	void Tick();

	void SetGraphic();
	void RemoveBlock();

	BaseBlock* GetBlock(int num);
	int GetNum();
	bool IsEmpty() {return GetNum() <= 0;}
	GarbageType GetType() {return type;}

  private:
	GarbageChunk* chunks[MAX_GARBAGE_SIZE];
	int lines;
	int numBlocks;
	int numFalling;
	enum BlockState state;
	int popDelay;
	GarbageType type;
};
