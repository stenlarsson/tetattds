#pragma once

#include "anim.h"
#include "chain.h"

class GarbageBlock;
class Sprite;

enum BlockType
{
	BLC_GREEN = TILE_BLOCK_GREEN,
	BLC_YELLOW = TILE_BLOCK_YELLOW,
	BLC_RED = TILE_BLOCK_RED,
	BLC_PINK = TILE_BLOCK_PINK,
	BLC_CYAN = TILE_BLOCK_CYAN,
	BLC_BLUE = TILE_BLOCK_BLUE,
	BLC_GRAY = TILE_BLOCK_GRAY,
	BLC_GARBAGE = TILE_GARBAGE_BLUE,
	BLC_EVILGARBAGE = TILE_GARBAGE_GRAY
};

enum BlockState
{
	BST_IDLE,
	BST_FALLING,
	BST_MOVING,
	BST_POSTMOVE,
	BST_HOVER,
	BST_FLASH,
	BST_POP,
	BST_POP2,
	BST_POP3,
	BST_DEAD
};

class BaseBlock
{
  public:
	BaseBlock(Anim const & anim, BlockType type);
	virtual ~BaseBlock();

	virtual void Tick() = 0;
	virtual enum BlockType GetType() { return type; }
	virtual enum BlockState GetState() { return state; }
	inline bool IsState(enum BlockState const & s) { return GetState() == s; } 

	virtual void Drop();
	virtual void Land();
	virtual void Move();
	virtual void Hover(int delay);
	virtual void Pop(int num, int total);
	virtual void Stress(bool stress);
	virtual void Stop(bool stop);

	virtual bool NeedPopCheck() { return bNeedPopCheck; }
	virtual void PopCheck() { bNeedPopCheck = false; }
	virtual void SetPop() { bPopped = true; }
	virtual bool IsPopped() { return bPopped; }
	virtual Chain* GetChain() { return chain; }
	virtual void SetChain(Chain* newChain) { if(chain != NULL) chain->activeBlocks--; chain = newChain; if(chain != NULL) chain->activeBlocks++;}

	virtual bool CheckDrop() { if(dropTimer <= 0) { dropTimer = BLOCK_DROP_TIMER; return true; } return false; }
	virtual void ChangeState(enum BlockState newState) = 0;

	int GetTile();

  protected:

	Anim anim;
	enum BlockType type;
	enum BlockState state;
	bool bNeedPopCheck;
	int popOffset;
	int dieOffset;
	int dropTimer;
	int stateDelay;
	enum BlockState nextState;
	Chain* chain;
	bool bPopped;
	bool bStress;
	bool bStop;
};
