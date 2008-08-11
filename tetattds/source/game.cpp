#include "tetattds.h"
#include "game.h"
#include "playfield.h"
#include "platformgraphics.h"
#include "serverconnection.h"
#include "sound.h"
#include "wrapping.h"

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
		   ServerConnection* connection,
		   ConnectionManager* connectionManager)
:	paused(false),
	field(new PlayField(g_fieldGraphics->GetEffectHandler())),
	connection(connection),
	connectionManager(connectionManager),
	running(false),
	touchedArea(0),
	level(level),
	sendToSelf(sendToSelf),
	scrollSpeed(levelData[level].scrollSpeed),
	col(0),
	row(0),
	sendFieldStateDeltaTimer(0),
	fieldState(0),
	deltaStoreEnd(0)
{
	std::fill_n(heldKeys, FWGUI_NUM_KEYS, false);
	memset(deltaStoreBegins, 0, sizeof(deltaStoreBegins));

	if(connection != NULL) {
		// try to spread out sending of the first field state
		sendFieldStateDeltaTimer =
			connection->GetMyPlayerNum() *
			SEND_FIELDSTATE_DELTA_INTERVAL;
	}
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
			field->KeyInput(INPUT_SWAP);
			break;

		case FwGui::FWKEY_X:
			g_fieldGraphics->TogglePlayerOffset();
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
		else if(touchedArea == 2)
			field->TouchHeld(col, row);
	}

	DEBUGVERBOSE("Game: field->Tick\n");
	field->Tick();

	if(connection != NULL)
	{
		int myPlayerNum = connection->GetMyPlayerNum();
		PlayerInfo* me = connection->GetPlayerInfo(myPlayerNum);
		field->GetFieldState(me->fieldState);
		g_fieldGraphics->PrintPlayerInfo(myPlayerNum, me);

		if(sendFieldStateDeltaTimer-- <= 0) {
			SendFieldStateDelta();

			sendFieldStateDeltaTimer =
				SEND_FIELDSTATE_DELTA_INTERVAL *
				connection->GetAlivePlayersCount();
		}
	}
}

void Game::Draw()
{
	g_fieldGraphics->Draw(field);
}

void Game::ReceiveGarbage(int num, int player, GarbageType type)
{
	if(field->IsState(PFS_PLAY))
		field->ScheduleGarbage(num, player, type);
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
		ReceiveGarbage(num, 0, type);
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
	field->GetFieldState(lastFieldState);

	FieldStateMessage message;
	memcpy(message.field, lastFieldState, 12*6);
	message.playerNum = connection->GetMyPlayerNum();
	connectionManager->BroadcastMessage(message);
}

void Game::SendFieldStateDelta()
{
	char newFieldState[12*6];
	field->GetFieldState(newFieldState);

	// Compute where we must base our state
	wrapping8 baseState(fieldState);
	for(unsigned int i = 0; i < MAX_PLAYERS; i++) {
		uint8_t ackedState = connection->GetPlayerInfo(i)->ackedFieldState;
		if (ackedState < baseState)
			baseState = ackedState;
	}
	
	// Move read position into place
	uint8_t deltaStoreBegin = deltaStoreBegins[baseState];
	uint8_t deltaStoreOriginalEnd = deltaStoreEnd;
	
	// Compute new delta items from current field state
	uint8_t delta[12*6];
	unsigned int length = 0;
	for(unsigned int i = 0; i < sizeof(newFieldState); i++) {
		if(newFieldState[i] != lastFieldState[i]) {
			delta[length++] = deltaStore[deltaStoreEnd++] = i;
			delta[length++] = deltaStore[deltaStoreEnd++] = newFieldState[i];

			// Not needed thanks to deltaStore size and deltaStoreEnd datatype
			// if (deltaStoreEnd == sizeof(deltaStore))
			// 	deltaStoreEnd = 0;

			if (deltaStoreBegin == deltaStoreEnd || length == sizeof(delta)) {
				length = sizeof(delta);
				break;
			}
		}
	}
	memcpy(lastFieldState, newFieldState, sizeof(lastFieldState));
	
	if (length == sizeof(delta)) {
		// Clear delta store...
		deltaStoreBegin = deltaStoreEnd = 0;
		
		// Insert a missing delta marker
		deltaStore[deltaStoreEnd++] = 0;
		deltaStore[deltaStoreEnd++] = 255;
		
		// Every state now requires a full update
		memset(deltaStoreBegins, sizeof(deltaStoreBegins), 0);
	}
	else if (deltaStore[deltaStoreBegin+1] == 255) {
		// We have orders to send full state...
		length = sizeof(delta);
	}
	else {
		// Add the new start point (unless empty delta)
		if (length != 0) {
			deltaStoreBegins[++fieldState] = deltaStoreEnd;
		}
		
		// Compose new delta with existing deltas for sending
		uint8_t pos = deltaStoreBegin;
		while (pos != deltaStoreOriginalEnd) {
			// Send only data that is still valid...
			if (newFieldState[deltaStore[pos]] == deltaStore[pos+1]) {
				delta[length++] = deltaStore[pos];
				delta[length++] = deltaStore[pos+1];

				if(length == sizeof(delta)) {
					break;
				}
			}
			
			pos += 2;
			// Not needed thanks to deltaStore size and deltaStoreEnd datatype
			// if (pos == sizeof(deltaStore))
			// 	pos = 0;
		}
	}
	
	if (length == sizeof(delta)) {
		// Send full state instead of delta...
		memcpy(delta, newFieldState, sizeof(delta));
	}
	
	// Note that we should send the message even when length == 0,
	// since the other players want to see our ack numbers...
	FieldStateDeltaMessage message;
	message.playerNum = connection->GetMyPlayerNum();
	for(unsigned int i = 0; i < MAX_PLAYERS; i++) {
		message.acks[i] = connection->GetPlayerInfo(i)->seenFieldState;
	}
	message.acks[message.playerNum] = fieldState;
	message.length = length;
	memcpy(message.delta, delta, length);
	connectionManager->BroadcastMessage(message);
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
	return field->GetScore();
}
