#pragma once
#include <vector>

class Block;

struct Chain
{
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
