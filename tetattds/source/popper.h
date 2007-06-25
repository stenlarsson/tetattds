#pragma once

#include <vector>

class PlayField;
class EffectHandler;
class Block;
class Chain;

class Popper
{
public:
	Popper(PlayField* newPF, EffectHandler* newEH);
	~Popper();

	void AddBlock(Block* block, int blocknum);
	void Pop();
	void Tick();

private:
	std::vector<Chain*> chains;
	Chain* newChain;
	PlayField* pf;
	EffectHandler* eh;
};
