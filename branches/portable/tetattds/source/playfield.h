#pragma once

#include "marker.h"
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
	PlayField();
	~PlayField();

	void Init(int xOffset, int yOffset);
	void RandomizeField();
	void Start();
	void Tick();
	void Draw();
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
	Marker marker;
	Marker touchMarker;
	int markerPos;
	Popper* popper;

	bool bFastScroll;
	double scrollOffset;
	int iScrollPause;

	EffectHandler* eh;

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
