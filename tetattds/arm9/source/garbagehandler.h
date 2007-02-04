#pragma once

#include "chain.h"
#include "garbage.h"

class PlayField;
class GarbageBlock;

class GarbageHandler
{
  public:
	GarbageHandler(PlayField* newPF);
	~GarbageHandler();

	// Adds garbage to be dropped, num is number of blocks for type==GARBAGE_COMBO and
	// number of lines for type==GARBAGE_CHAIN and type==GARBAGE_EVIL.
	// player is the number of the player who sent the garbage, used for coloring.
	void AddGarbage(int num, int player, GarbageType type);
	
	void Tick();

	void AddPop(GarbageBlock* newPop, Chain* chain, int order);
	void Pop();

  private:
	int NextFree();					// Gets the next free Blocks-number
	void DropGarbage();				// Drops GarbageBlocks specified by bDropChain
	GarbageBlock* Blocks[MAX_GARBAGE];	// Our GarbageBlocks
	Chain* BlockChain[MAX_GARBAGE];		// Keeps track of the local chain the GarbageBlock is involved in.
	bool bDropped[MAX_GARBAGE];		// Set when a block is dropped to the playfield.
	bool bPop[MAX_GARBAGE];			// Whether a certain GarbageBlock should pop this tick or not.
	int PopOrder[MAX_GARBAGE];			// The order to pop garbageblocks.
	int numBlocks;						// Current number of active GarbageBlocks.
//	int iDropChain[MAX_GARBAGE];		// Opponents chain number, to keep track of when to drop what.
	bool bDropBlock[MAX_GARBAGE];		// Set when opponent signals that a chain should be dropped.
	bool bDropGarbage;					// Set when there's something to drop.

	PlayField* pf;
};
