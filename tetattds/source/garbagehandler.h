#pragma once

#include "garbage.h"
#include <vector>
#include <list>

class PlayField;
class GarbageBlock;
struct GBInfo;

/**
 * GarbageHandler is a helper class for PlayField that handles garbage.
 */
class GarbageHandler
{
public:
	GarbageHandler();
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
	 * Add another garbage block to the pop check. If first is false
	 * the block is placed after existing blocks. If first is true,
	 * the block is placed before existing blocks.
	 */
	void AddPop(GarbageBlock* newPop, Chain* chain, bool first);
	/** Run Pop for the blocks collected using AddPop. */
	void Pop();
	
	void DropGarbage(PlayField * pf);

private:
	/** Blocks that a currently in play. */
	std::vector<GarbageBlock*> activeBlocks;
	/** Blocks waiting to be dropped into play. */
	std::list<GarbageBlock*> normalDrops, chainDrops;
	/** Additional information about blocks being popped. */
	std::list<GBInfo> popBlocks;
};
