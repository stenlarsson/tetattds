#pragma once
#include <vector>

class Block;

struct Chain
{
	Chain() 
		: numBlocks(0),
		  length(0),
		  chainNum(0),
		  activeBlocks(0),
		  bSentCombo(false),
		  popCount(0),
		  bUsedThisFrame(false),
		  garbage()
	{
		ClearBlocks();
	}
	
	~Chain()
	{
		ASSERT(activeBlocks == 0);
	}
	
	inline bool IsActive() const
	{
		ASSERT(activeBlocks >= 0);
		return activeBlocks > 0;
	}
	
	inline void ClearBlocks()
	{
		std::fill_n(blocks, 100, (Block*)NULL);
		std::fill_n(blockNum, 100, -1);
	}
	
	Block* blocks[100];
	int blockNum[100];
	int numBlocks;
	int length;
	int chainNum;
	int activeBlocks;
	bool bSentCombo;
	int popCount;
	bool bUsedThisFrame;
	std::vector<unsigned int> garbage;
};
