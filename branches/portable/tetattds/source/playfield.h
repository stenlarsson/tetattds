#pragma once

#include <algorithm>
#include "garbagehandler.h"

enum ControlMode
{
	MM_NONE,
	MM_KEY,
	MM_TOUCH
};

class BaseBlock;
class EffectHandler;
class Popper;

enum PFState
{
	PFS_INIT,
	PFS_START,
	PFS_PLAY,
	PFS_DIE,
	PFS_DEAD
};

class PlayField
{
  public:
	PlayField(EffectHandler *effects);
	~PlayField();

	void Init();
	void RandomizeField();
	void Start();
	void Tick();
	void KeyInput(Input input);
	void TouchDown(int col, int row);
	void TouchHeld(int col, int row);
	void TouchUp();
	void PixelsToColRow(int x, int y, int& col, int& row) const;
	int ColRowToPos(int col, int row) const;
	void AddScore(int Score) { score += Score; }
	void DelayScroll(int delay) { iScrollPause = std::min(MAX_STOP_TIME, iScrollPause+delay); }
	const int GetScore() const { return score; }
	
	int GetHeight() const { return *std::max_element(fieldHeight, fieldHeight+PF_WIDTH); }

	BaseBlock* GetField(int i) const { return field[i]; }
	PFState GetState() const { return state; }
	bool IsState(PFState const & s) const { return state == s; }
	void SetState(PFState state) { this->state = state; }

	void AddGarbage(int num, int player, GarbageType type) { gh->AddGarbage(num, player, type); }
	
	bool SwapBlocks(int pos);

	void GetFieldState(char* dest) const;

	int GetMarkerPos() const { return markerPos; }
	int GetTouchPos() const { return ColRowToPos(touchCol, touchRow); }
	float GetScrollOffset() const { return scrollOffset; }
	int GetTimeTicks() const { return timeTicks; }
	int GetScrollPause() const { return iScrollPause; }
	ControlMode GetControlMode() const { return controlMode; }
	
	// TODO: Should really have row here and not position x
	bool IsLineOfFieldEmpty(int x) const;

	/**
	 * Insert the garbage corresponding to the block B starting
	 * on the field position X, and adding the corresponding
	 * number of field blocks. If leftAlign is passed then the
	 * last inserted field block will be on the left side.
	 * If there is no room for the garbage, nothing is inserted
	 * and false is returned.
	 */
	// TODO: Should really have row here and not position for x
	bool InsertGarbage(int x, GarbageBlock *b, bool leftAlign);

  private:
	void RandomizeRow(int row);
	void ScrollField();
	void DropBlocks();
	void CheckForPops();
	void ClearDeadBlocks();
	bool ShouldScroll();
	void StateCheck();
	void CheckHeight();

	BaseBlock* field[PF_NUM_BLOCKS];
	int fieldHeight[PF_WIDTH];
	int markerPos;
	Popper* popper;

	bool bFastScroll;
	double scrollOffset;
	int iScrollPause;

	EffectHandler* effects;

	int iSwapTimer;

	enum PFState state;
	int stateDelay;

	bool bTooHigh;
	int iDieTimer;

	// music info
	bool bMusicNormal;
	bool bMusicDanger;
	int normalMusicDelay;

	// Various stats
	int score;
	int timeTicks;
	int scrolledRows;
	

	GarbageHandler* gh;

	int touchCol;
	int touchRow;

	ControlMode controlMode;
};
