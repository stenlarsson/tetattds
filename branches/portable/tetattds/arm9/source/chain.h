#pragma once
#include <vector>

class BaseBlock;

struct Chain
{
	BaseBlock* blocks[100];
	int blockNum[100];
	int numBlocks;
	int length;
	int chainNum;
	int activeBlocks;
	bool bSentCombo;
	int popCount;
	std::vector<unsigned int> garbage;
};
