#include "tetattds.h"
#include "game.h"
#include "playfield.h"
#include "serverconnection.h"
#include "connectionmanager.h"
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
:	paused(false),
	field(new PlayField(g_fieldGraphics->GetEffectHandler())),
	connection(connection),
	running(false),
	touchedArea(0),
	level(level),
	sendToSelf(sendToSelf),
	scrollSpeed(levelData[level].scrollSpeed),
	col(0),
	row(0)
{
	std::fill_n(heldKeys, FWGUI_NUM_KEYS, false);
	
	field->Init(PLAYFIELD_OFFSET_X, PLAYFIELD_OFFSET_Y);
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

void Game::KeyDown(FwGui::Key key)
{
	heldKeys[key] = true;
	heldKeysDelay[key] = KEY_HOLD_DELAY;

	if(key == FwGui::FWKEY_START)
	{
		if(connection == NULL)
			paused = !paused;
	}

	if(!paused)
	{
		switch(key) {
		case FwGui::FWKEY_UP:
			field->KeyInput(INPUT_UP);
			break;

		case FwGui::FWKEY_DOWN:
			field->KeyInput(INPUT_DOWN);
			break;

		case FwGui::FWKEY_LEFT:
			field->KeyInput(INPUT_LEFT);
			break;

		case FwGui::FWKEY_RIGHT:
			field->KeyInput(INPUT_RIGHT);
			break;

		case FwGui::FWKEY_A:
		case FwGui::FWKEY_B:
		case FwGui::FWKEY_X:
		case FwGui::FWKEY_Y:
			field->KeyInput(INPUT_SWAP);
			break;
		default:
			break;
		}
	}
}

void Game::KeyUp(FwGui::Key key)
{
	heldKeys[key] = false;
}

void Game::TouchDown(int x, int y)
{
	heldKeys[FwGui::FWKEY_TOUCH] = true;

	if (touchedArea != 2 &&
		x >= 206 && x <= 247 &&
		y >= 142 && y <= 183)
	{
		field->KeyInput(INPUT_RAISE);
		touchedArea = 1;
	}
	else if (touchedArea != 1)
	{
		field->PixelsToColRow(x, y, col, row);
		if(touchedArea == 0)
		{
			field->TouchDown(col, row);
			touchedArea = 2;
		}
	}
}

void Game::TouchDrag(int x, int y)
{
	if(touchedArea == 2) {
		field->PixelsToColRow(x, y, col, row);
	}
}
void Game::TouchUp(int x, int y)
{
	heldKeys[FwGui::FWKEY_TOUCH] = false;
	field->TouchUp();
	touchedArea = 0;
}

void Game::Tick()
{
	if(scrollSpeed < MAX_SCROLL_SPEED)
		scrollSpeed *= SCROLL_SPEED_INCREASE;

	if(heldKeys[FwGui::FWKEY_UP] && --heldKeysDelay[FwGui::FWKEY_UP] < 0)
	{
		field->KeyInput(INPUT_UP);
		heldKeysDelay[FwGui::FWKEY_UP] = KEY_REPEAT_DELAY;
	}
	if(heldKeys[FwGui::FWKEY_DOWN] && --heldKeysDelay[FwGui::FWKEY_DOWN] < 0)
	{
		field->KeyInput(INPUT_DOWN);
		heldKeysDelay[FwGui::FWKEY_DOWN] = KEY_REPEAT_DELAY;
	}
	if(heldKeys[FwGui::FWKEY_LEFT] && --heldKeysDelay[FwGui::FWKEY_LEFT] < 0)
	{
		field->KeyInput(INPUT_LEFT);
		heldKeysDelay[FwGui::FWKEY_LEFT] = KEY_REPEAT_DELAY;
	}
	if(heldKeys[FwGui::FWKEY_RIGHT] && --heldKeysDelay[FwGui::FWKEY_RIGHT] < 0)
	{
		field->KeyInput(INPUT_RIGHT);
		heldKeysDelay[FwGui::FWKEY_RIGHT] = KEY_REPEAT_DELAY;
	}
	if(heldKeys[FwGui::FWKEY_L] || heldKeys[FwGui::FWKEY_R])
	{
		field->KeyInput(INPUT_RAISE);
	}
	if(heldKeys[FwGui::FWKEY_TOUCH])
	{
		if(touchedArea == 1)
			field->KeyInput(INPUT_RAISE);
		else
			field->TouchHeld(col, row);
	}

	DEBUGVERBOSE("Game: field->Tick\n");
	field->Tick();
}

void Game::Draw()
{
	g_fieldGraphics->Draw(field);
}

void Game::AddGarbage(int num, int player, GarbageType type)
{
	if(field->IsState(PFS_PLAY))
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
