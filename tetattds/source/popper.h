#pragma once

#include "chain.h"

class PlayField;
class EffectHandler;
class Block;

class Popper
{
  public:
	Popper(PlayField* newPF, EffectHandler* newEH);
	~Popper();

	void AddBlock(Block* block, int blocknum);
	void Pop();
	void Tick();

  private:
	void SortChain(Chain* chain);
	void ClearChain(Chain* chain);
	Chain* GetFreeChain();

	Chain chains[MAX_CHAINS];
	bool bUsedThisFrame[MAX_CHAINS];
	bool bFree[MAX_CHAINS];
	Chain* newChain;
	PlayField* pf;
	EffectHandler* eh;
};
