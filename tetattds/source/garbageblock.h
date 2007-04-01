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
	/**
	 * Called from Garbage::~Garbage, to signal that the block goes away
	 * The Garbage is in turn destroyed in PlayField when it is replaced
	 * by a normal block (i.e. right after the call to Garbage::CreateBlock)
	 */
	void RemoveBlock();

	BaseBlock* GetBlock(int num);
	inline int GetNum() const { return numBlocks; };
	inline bool IsEmpty() const { return GetNum() <= 0; }
	inline GarbageType GetType() const { return type; }

private:
	GarbageChunk* chunks[MAX_GARBAGE_SIZE];
	int lines;
	int numBlocks;
	int numFalling;
	enum BlockState state;
	int popDelay;
	GarbageType type;
};
