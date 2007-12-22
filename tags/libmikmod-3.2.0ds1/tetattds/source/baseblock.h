#pragma once

#include "anim.h"

struct Chain;
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
	BaseBlock(Anim const & anim, BlockType type, BlockState state, Chain* chain, bool needPopCheck);
	virtual ~BaseBlock();

	virtual void Tick();
	
	virtual enum BlockType GetType() { return type; }
	virtual enum BlockState GetState() { return state; }
	inline bool IsState(enum BlockState const & s) { return GetState() == s; } 
	inline bool SameType(BaseBlock * other) { return other->GetType() == GetType(); }

	virtual void Drop() = 0;
	virtual void Land() = 0;
	virtual void Hover(int delay) = 0;
	inline void DropAndLand() { Drop(); Land(); }

	inline bool NeedPopCheck() const { return needPopCheck; }
	inline void PopChecked() { needPopCheck = false; }
	inline void ForcePopCheck() { needPopCheck = true; }
	inline void SetPop() { popped = true; }
	inline bool IsPopped() const { return popped; }
	inline Chain* GetChain() const { return chain; }
	void SetChain(Chain* newChain);

	bool CheckDrop()
	{
		bool dropTime = (dropTimer <= 0);
		if(dropTime)
			dropTimer = BLOCK_DROP_TIMER;
		return dropTime;
	}

	int GetTile() const;

protected:
	virtual void ChangeState(BlockState newState) = 0;
	
	Anim anim;
	BlockType type;
	BlockState state;
	int popOffset;
	int dieOffset;
	int dropTimer;
	int stateDelay;
	BlockState nextState;

private:
	bool needPopCheck;
	Chain* chain;
	bool popped;
};
