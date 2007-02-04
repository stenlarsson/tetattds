#pragma once

#include "sprite.h"
#include "baseblock.h"
#include "garbage.h"

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

class Game
{
public:
	Game(
		int level,
		bool sendToSelf,
		ServerConnection* connection);
	~Game();

	void AddGarbage(int num, int player, GarbageType type);

	void SendGarbage(int num, GarbageType type);

	void PlayerDied();
	
	enum BlockType GetRandomBlockType(bool grayBlock);
	
	double GetScrollSpeed() { return scrollSpeed; }
	const LevelData* GetLevelData();
	
	int GetScore();
	
	bool paused;
	PlayField* field;

	void Start();
	void HandleInput();
	void Tick();
	void Draw();
	void SendFieldState();
	
private:
	ServerConnection* connection;
	bool running;
	int lastX;
	int lastY;
	bool firstTouch;
	int level;
	bool sendToSelf;
	double scrollSpeed;
};

extern Game* g_game;
