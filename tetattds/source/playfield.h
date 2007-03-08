#pragma once

#include "chain.h"
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

	void Init(int xOffset, int yOffset);
	void RandomizeField();
	void Start();
	void Tick();
	void KeyInput(Input input);
	void TouchDown(int col, int row);
	void TouchHeld(int col, int row);
	void TouchUp();
	void PixelsToColRow(int x, int y, int& col, int& row);
	int ColRowToPos(int col, int row);
	void AddScore(int Score) { score += Score; }
	void DelayScroll(int delay) { iScrollPause += delay; if(iScrollPause > MAX_STOP_TIME) iScrollPause = MAX_STOP_TIME; }
	const int* GetScore() { return &score; }
	
	int GetHeight() { int max=0; for(int i = 0;i<6;i++){if(fieldHeight[i] > max){max = fieldHeight[i];}}return max; }

	int GetFieldX(int i) { return fieldX[i]; }
	int GetFieldY(int i) { return fieldY[i]; }
	BaseBlock** GetField(int i) { return &field[i]; }
	PFState GetState() { return state; }
	void SetState(PFState state) { this->state = state; }

	void AddGarbage(int num, int player, GarbageType type) { gh->AddGarbage(num, player, type); }
	
	bool SwapBlocks(int pos);

	void GetFieldState(char* dest);

	int GetMarkerPos() { return markerPos; }
	int GetTouchPos() { return ColRowToPos(touchCol, touchRow); }
	float GetScrollOffset() { return scrollOffset; }
	int GetTimeTicks() { return timeTicks; }
	int GetScrollPause() { return iScrollPause; }
	ControlMode GetControlMode() { return controlMode; }
	
	// TODO: Should really have row here and not position x
	bool IsLineOfFieldEmpty(int x);

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

	BaseBlock* field[PF_WIDTH*PF_HEIGHT];
	int fieldX[PF_WIDTH*PF_HEIGHT];
	int fieldY[PF_WIDTH*PF_HEIGHT];
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
