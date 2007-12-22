#pragma once

#include "baseblock.h"
#include "garbage.h"
#include <vector>

class Garbage;
struct Chain;

enum GarbageGraphicsType
{
	GARBAGE_GRAPHICS_NONE   = 0,
	GARBAGE_GRAPHICS_SINGLE = TILE_GARBAGE_BLUE + TILE_GARBAGE_SINGLE_OFFSET,
	GARBAGE_GRAPHICS_TOP    = TILE_GARBAGE_BLUE + TILE_GARBAGE_TOP_OFFSET,
	GARBAGE_GRAPHICS_MIDDLE = TILE_GARBAGE_BLUE + TILE_GARBAGE_MIDDLE_OFFSET,
	GARBAGE_GRAPHICS_BOTTOM = TILE_GARBAGE_BLUE + TILE_GARBAGE_BOTTOM_OFFSET,
	GARBAGE_GRAPHICS_EVIL   = TILE_GARBAGE_GRAY
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
	inline int GetNum() const { return blocks.size(); };
	inline bool IsEmpty() const { return GetNum() <= 0; }
	inline GarbageType GetType() const { return type; }

private:
	std::vector<Garbage*> blocks;
	unsigned int numFalling;
	BlockState state;
	int popDelay;
	GarbageType type;
};
