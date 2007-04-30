#pragma once
#include <vector>
#include "garbage.h"

class Block;

struct GarbageInfo
{
	int size;
	GarbageType type;
};

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
	
	void AddBlock(Block* block, int blocknum)
	{
		blocks[numBlocks] = block;
		blockNum[numBlocks] = blocknum;
		numBlocks++;
		if(!bUsedThisFrame) 
		{
			// Increases chain length every frame it's involved in popping new blocks
			length++;
			popCount = 0;
			bUsedThisFrame = true;
		}
	}
	
	void Sort()
	{
		bool changed = true;
		while(changed)
		{
			changed = false;
			for(int i = 0; i < numBlocks-1; i++)
			{
				if(blockNum[i+1] < blockNum[i])
				{
					std::swap(blockNum[i], blockNum[i+1]);
					std::swap(blocks[i], blocks[i+1]);
					changed = true;
				}
				else if(blockNum[i] == blockNum[i+1])
				{
					for(int ii = i; ii < numBlocks-1; ii++)
					{
						blockNum[ii] = blockNum[ii+1];
						blocks[ii] = blocks[ii+1];
					}

					numBlocks--;
					blockNum[numBlocks] = -1;
					blocks[numBlocks] = NULL;

					changed = true;
				}
			}
		}
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
	std::vector<GarbageInfo> garbage;
};
