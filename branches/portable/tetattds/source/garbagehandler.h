#pragma once

#include "chain.h"
#include "garbage.h"

class PlayField;
class GarbageBlock;

/** Helper struct maintaining per-block information */
struct GBInfo
{
	GBInfo() 
		: block(NULL), chain(NULL), PopOrder(0xFFFF), bPop(false), bDropBlock(false)
	{
	}

	GarbageBlock* block;
	/** The local chain the GarbageBlock is involved in. */
	Chain* chain;
	/** The pop priority of the block */
	int PopOrder;
	/** Should the block pop this tick or not. */
	bool bPop;
	/** Set when opponent signals that a chain should be dropped. */
	bool bDropBlock;
};

/**
 * GarbageHandler is a helper class for PlayField that handles garbage.
 */
class GarbageHandler
{
public:
	GarbageHandler(PlayField* newPF);
	~GarbageHandler();

	/**
	 * Adds garbage to be dropped, num is number of blocks for type==GARBAGE_COMBO and
	 * number of lines for type==GARBAGE_CHAIN and type==GARBAGE_EVIL.
	 * player is the number of the player who sent the garbage,
	 * to be used for coloring.
	 */
	void AddGarbage(int num, int player, GarbageType type);
	void Tick();
	void AddPop(GarbageBlock* newPop, Chain* chain, int order);
	void Pop();

private:
	/** Helper method to allocate a garbage block in the memory pool */
	void AllocGarbage(int num, GarbageType type);
	
	int NextFree();
	void DropGarbage();

	PlayField * const pf;
	
	GBInfo Blocks[MAX_GARBAGE];
	int numBlocks;

	bool bDropGarbage;
};
