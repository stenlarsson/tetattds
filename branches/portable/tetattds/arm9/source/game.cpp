#include "tetattds.h"
#include "game.h"
#include "playfield.h"
#include "serverconnection.h"
#include "connectionmanager.h"
#include "fifo.h"
#include "sound.h"

Game* g_game = NULL;

static enum BlockType blockTypes[] =
{
	BLC_GREEN,
	BLC_RED,
	BLC_YELLOW,
	BLC_CYAN,
	BLC_PINK,
	BLC_BLUE
};

LevelData levelData[10] =
{ // next speedlevel == current speedlevel * 1.8
//Speed, Blocks, Flashtime, PopOffset, PopTime, Comboeffect Duration
	{0.005, 5, 48, 14, 9, 50},
	{0.009, 5, 46, 13, 8, 50},
	{0.016, 5, 44, 12, 8, 50},
	{0.029, 5, 42, 11, 7, 50},
	{0.052, 5, 40, 10, 7, 50},
	{0.094, 6, 38,  9, 6, 45},
	{0.170, 6, 36,  8, 6, 40},
	{0.306, 6, 34,  7, 5, 35},
	{0.551, 6, 32,  6, 4, 30},
	{0.992, 6, 30,  5, 3, 25}
};

const LevelData* Game::GetLevelData()
{
	int i = 0;
	for(i = 0;i<9;i++)
		if(levelData[i+1].scrollSpeed > scrollSpeed)
			break;
	
	return &levelData[i];
}

Game::Game(int level,
		bool sendToSelf,
		ServerConnection* connection)
:	connection(connection),
	level(level),
	sendToSelf(sendToSelf)
{
	field = NULL;
	scanKeys();

	touchedArea = 0;
	lastX = INT_MAX;
	lastY = INT_MAX;

	field = new PlayField();
	field->Init(88, 0);
	scrollSpeed = levelData[level].scrollSpeed;
	paused = false;
}

Game::~Game()
{
	DEL(field);
}

void Game::Start()
{
	field->RandomizeField();
	field->Start();
}

void Game::HandleInput()
{
	static int upDownTime = KEY_HOLD_DELAY;	// keeps track of when to autopress keys while holding
	static int leftRightTime = KEY_HOLD_DELAY;
	bool bUpDownHeld = false;					// any key held this frame i wonder?
	bool bLeftRightHeld = false;
	
	scanKeys();

	if(keysDown() & KEY_START)
	{
		if(connection == NULL)
			paused = !paused;
	}

	if(!paused)
	{
		if(keysDown() & KEY_UP)
		{
			field->KeyInput(INPUT_UP);
		}
		if(keysHeld() & KEY_UP)
		{
			bUpDownHeld = true;
			if(--upDownTime < 0)
			{
				field->KeyInput(INPUT_UP);
				upDownTime = KEY_REPEAT_DELAY;
			}
		}
	
		if(keysDown() & KEY_LEFT)
		{
			field->KeyInput(INPUT_LEFT);
		}
		if(keysHeld() & KEY_LEFT)
		{
			bLeftRightHeld = true;
			if(--leftRightTime < 0)
			{
				field->KeyInput(INPUT_LEFT);
				leftRightTime = KEY_REPEAT_DELAY;
			}
		}
	
		if(keysDown() & KEY_RIGHT)
		{
			field->KeyInput(INPUT_RIGHT);
		}
		if(keysHeld() & KEY_RIGHT)
		{
			bLeftRightHeld = true;
			if(--leftRightTime < 0)
			{
				field->KeyInput(INPUT_RIGHT);
				leftRightTime = KEY_REPEAT_DELAY;
			}
		}
	
		if(keysDown() & KEY_DOWN)
		{
			field->KeyInput(INPUT_DOWN);
		}
		if(keysHeld() & KEY_DOWN)
		{
			bUpDownHeld = true;
			if(--upDownTime < 0)
			{
				field->KeyInput(INPUT_DOWN);
				upDownTime = KEY_REPEAT_DELAY;
			}
		}
	
		if(keysDown() & (KEY_A | KEY_B | KEY_X | KEY_Y))
		{
			field->KeyInput(INPUT_SWAP);
		}
	
		if(keysHeld() & (KEY_L | KEY_R))
		{
			field->KeyInput(INPUT_RAISE);
		}

		if(keysHeld() & KEY_TOUCH)
		{
			touchPosition touchXY = touchReadXY();
			int x = touchXY.px;
			int y = touchXY.py;
			int dx = x - lastX;
			int dy = y - lastY;
			lastX = x;
			lastY = y;
	
			if(dx*dx + dy*dy < TOUCH_DELTA)
			{
				if (touchedArea != 2 &&
					x >= 206 && x <= 247 &&
					y >= 142 && y <= 183)
				{
					field->KeyInput(INPUT_RAISE);
					touchedArea = 1;
				}
				else if (touchedArea != 1)
				{
					int col, row;
					field->PixelsToColRow(x, y, col, row);
					if(touchedArea == 0)
					{
						field->TouchDown(col, row);
						touchedArea = 2;
					}
					else
					{
						field->TouchHeld(col, row);
					}
				}
			}
		}
		else if(keysUp() & KEY_TOUCH)
		{
			field->TouchUp();
			touchedArea = 0;
			lastX = INT_MAX;
			lastY = INT_MAX;
		}
	}
	
	if(!bUpDownHeld)	// No key held?
		upDownTime = KEY_HOLD_DELAY;	// Reset delay
	if(!bLeftRightHeld)
		leftRightTime = KEY_HOLD_DELAY;
}

void Game::Tick()
{
	if(scrollSpeed < MAX_SCROLL_SPEED)
		scrollSpeed *= SCROLL_SPEED_INCREASE;
	
	DEBUGVERBOSE("Game: field->Tick\n");
	field->Tick();
}

void Game::Draw()
{
	field->Draw();
}

void Game::AddGarbage(int num, int player, GarbageType type)
{
	if(field->GetState() == PFS_PLAY)
		field->AddGarbage(num, player, type);
}

void Game::SendGarbage(int num, GarbageType type)
{
	if(connection != NULL)
	{
		GarbageMessage message;
		message.num = num;
		message.player = connection->GetMyPlayerNum();
		message.type = (int)type;
		connection->SendMessage(message);
	}
	else if(sendToSelf)
	{
		field->AddGarbage(num, 0, type);
	}
}

void Game::PlayerDied()
{
	if(connection != NULL)
	{
		DiedMessage message;
		connection->SendMessage(message);
		SendFieldState();
	}
}

void Game::SendFieldState()
{
	FieldStateMessage message;
	field->GetFieldState(message.field);
	connection->SendMessage(message);
}

enum BlockType Game::GetRandomBlockType(bool grayBlock)
{
	if(grayBlock && (connection != NULL || sendToSelf == true))
		if(rand()%1024 > 1024*(1-GRAY_BLOCK_CHANCE))
			return BLC_GRAY;

	return blockTypes[rand()%levelData[level].numBlockTypes];
}

int Game::GetScore()
{
	return *field->GetScore();
}
