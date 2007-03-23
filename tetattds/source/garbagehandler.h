#pragma once

#include "chain.h"
#include "garbage.h"
#include <vector>
#include <list>

class PlayField;
class GarbageBlock;
class GBInfo;

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
	/** Call Tick for all active blocks, and remove redundant ones. */
	void Tick();
	/**
	 * Add another garbage block to the pop check. If order is positive
	 * the block is placed after existing blocks. if order is negative,
	 * it is placed before existing blocks.
	 */
	void AddPop(GarbageBlock* newPop, Chain* chain, int order);
	/** Run Pop for the blocks collected using AddPop. */
	void Pop();

private:
	void DropGarbage();

	PlayField * const pf;
	
	/** Blocks that a currently in play. */
	std::vector<GarbageBlock*> activeBlocks;
	/** Blocks waiting to be dropped into play. */
	std::list<GarbageBlock*> normalDrops, chainDrops;
	/** Additional information about blocks being popped. */
	std::list<GBInfo> popBlocks;
};
