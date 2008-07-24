#pragma once

#include "sprite.h"
#include "baseblock.h"
#include "garbage.h"
#include <inputlistener.h>

class PlayField;
class ServerConnection;
class ConnectionManager;

struct LevelData
{
	double scrollSpeed;
	int numBlockTypes;
	int flashTime;
	int popStartOffset;
	int popTime;
	int effComboDuration;
};

class Game : public FwGui::InputListener
{
public:
	Game(
		int level,
		bool sendToSelf,
		ServerConnection* connection,
		ConnectionManager* connectionManager);
	~Game();

	void ReceiveGarbage(int num, int player, GarbageType type);

	void SendGarbage(int num, GarbageType type);

	void PlayerDied();
	
	enum BlockType GetRandomBlockType(bool grayBlock);
	
	double GetScrollSpeed() { return scrollSpeed; }
	const LevelData* GetLevelData();
	
	int GetScore();
	
	bool paused;
	PlayField* field;

	void Start();
	virtual void KeyDown(FwGui::Key key);
	virtual void KeyUp(FwGui::Key key);
	virtual void TouchDown(int x, int y);
	virtual void TouchUp(int x, int y);
	virtual void TouchDrag(int x, int y);
	void Tick();
	void Draw();
	void SendFieldState();
	void SendFieldStateDelta();
	
private:
	ServerConnection* connection;
	ConnectionManager* connectionManager;
	bool running;
	int touchedArea;
	int level;
	bool sendToSelf;
	double scrollSpeed;
	bool heldKeys[FWGUI_NUM_KEYS];
	int heldKeysDelay[FWGUI_NUM_KEYS];
	int col;
	int row;
	int sendFieldStateTimer;
	int sendFieldStateDeltaTimer;
	char lastFieldState[12*6];
};

extern Game* g_game;
