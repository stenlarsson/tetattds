#include "tetattds.h"

#include <stdio.h>

#include <driver.h>
#include <theme.h>

#if 0
#include "menu1_bin.h"
#include "menu2_bin.h"
#endif

#include "game.h"
#include "wifi.h"
#include "mainmenudialog.h"
#include "leveldialog.h"
#include "wifimenudialog.h"
#include <textentrydialog.h>
#include "util.h"
#include "settings.h"
#include "sound.h"
#include "statusdialog.h"
#include "playfield.h"

#include "serverconnection.h"
#include "udpconnectionmanager.h"
#include "state.h"
#include "platformgraphics.h"

#include "localwifi.h"
#include "wifitypemenudialog.h"
#include "localconnectionmanager.h"
#include "servergame.h"
#include "loopbackconnection.h"
#include <lobby.h>

// TODO: Put these prototypes someplace else
void ShowSplashScreen();
void HideSplashScreen();
void InitSettings();
void SeedRandom();
void* GetMenuBackground();

static bool hasSetupWifi = false;
static bool hasSetupLocalWifi = false;
static ServerConnection* connection = NULL;
static ConnectionManager* connectionManager = NULL;

static bool hostGame = false;

static int level = 4;
extern char name[10];
extern Settings* settings;
static FwGui::Driver* gui;
#if 0
static TransferSoundData keySound;
static TransferSoundData menuSound;
#endif

static State* highscoreState;
static State* mainMenuState;
static State* endlessLevelState;
static State* vsSelfLevelState;
static State* endlessGameState;
static State* vsSelfGameState;
static State* wifiTypeMenuState;
static State* localWifiState;
static State* wifiState;
static State* wifiHostEntryState;
static State* setupWifiState;
static State* wifiMenuState;
static State* wifiLevelState;
static State* wifiChatEntryState;
static State* wifiGameState;
static State* splashScreenState;
static State* waitForServerAcceptState;
static State* waitForGameEndState;

static State* currentState = NULL;
static State* nextState = NULL;

class HighscoreState : public State {
public:
	virtual void Enter() {
	}
	virtual void Tick() {
		const char* name;
		int score;
	
		PrintStatus("   Endless highscores\n");
		for(int i = 0; i < NUM_HIGHSCORES; i++) {
			settings->GetHighscore(0, i, name, score);
			PrintStatus("Lv %2i  %-10.10s %5i\n", i+1, name, score);
		}
	
		PrintStatus("\n   Vs Self highscores\n");
		for(int i = 0; i < NUM_HIGHSCORES; i++) {
			settings->GetHighscore(1, i, name, score);
			PrintStatus("Lv %2i  %-10.10s %5i\n", i+1, name, score);
		}

		nextState = mainMenuState;
	}
	virtual void Exit() {
	}
};

class MainMenuState : public State {
public:
	virtual void Enter() {
		dialog = new MainMenuDialog();

		gui->SetActiveDialog(dialog);

		settings->Save();
		PlatformGraphics::InitSubScreen(false);
	}
	virtual void Tick() {

		switch (dialog->selection) {
		case MMSEL_NONE:
			break;

		case MMSEL_ENDLESS:
			nextState = endlessLevelState;
			break;

		case MMSEL_VS_SELF:
			nextState = vsSelfLevelState;
			break;

		case MMSEL_WIFI:
			nextState = wifiTypeMenuState;
			break;

		case MMSEL_HIGHSCORES:
			nextState = highscoreState;
			break;
		}
	}
	virtual void Exit() {
		// to get a random field every time
		SeedRandom();

		gui->SetActiveDialog(NULL);
		delete dialog;
		dialog = NULL;
	}
private:
	MainMenuDialog* dialog;
};

class WifiTypeMenuState : public State {
public:
	virtual void Enter() {
		dialog = new WifiTypeMenuDialog(!hasSetupLocalWifi, !hasSetupWifi);

		gui->SetActiveDialog(dialog);
	}
	virtual void Tick() {

		switch (dialog->selection) {
		case WTSEL_NONE:
			break;

		case WTSEL_HOST_INTERNET:
			hostGame = true;
			nextState = setupWifiState;
			break;
			
		case WTSEL_JOIN_INTERNET:
			hostGame = false;
			nextState = wifiHostEntryState;
			break;
			
		case WTSEL_HOST_LOCAL:
			hostGame = true;
			nextState = localWifiState;
			break;

		case WTSEL_JOIN_LOCAL:
			hostGame = false;
			nextState = localWifiState;
			break;

		case WTSEL_QUIT:
			nextState = mainMenuState;
			break;
		}
	}
	virtual void Exit() {
		// to get a random field every time
		SeedRandom();

		gui->SetActiveDialog(NULL);
		delete dialog;
		dialog = NULL;
	}
private:
	WifiTypeMenuDialog* dialog;
};

class LevelState : public State {
public:
	LevelState(State* acceptState, State* cancelState)
		: dialog(NULL), acceptState(acceptState), cancelState(cancelState) {
	}
	virtual void Enter() {
		dialog = new LevelDialog(level);
		gui->SetActiveDialog(dialog);
	}
	virtual void Tick() {
		if (dialog->level >= 0) {
			level = dialog->level;
			nextState = acceptState;
		} else if (dialog->level == -2) {
			nextState = cancelState;
		}
	}
	virtual void Exit() {
		gui->SetActiveDialog(NULL);
		delete dialog;
		dialog = NULL;
	}

private:
	LevelDialog* dialog;
	State* acceptState;
	State* cancelState;
};

class TextEntryState : public State {
public:
	TextEntryState(const char* title, State* acceptState, State* cancelState) 
		: title(title), dialog(NULL), acceptState(acceptState), cancelState(cancelState) {
	}
	virtual void Enter() {
		dialog = new FwGui::TextEntryDialog(title, "", MAX_CHAT_LENGTH);
		gui->SetActiveDialog(dialog);
	}
	virtual void Tick() {
		if (dialog->ok) {
			if(strlen(GetText()) == 0) {
				nextState = cancelState;
			} else {
				nextState = acceptState;
			}
		}
	}
	virtual void Exit() {
		gui->SetActiveDialog(NULL);
		delete dialog;
		dialog = NULL;
	}
	const char* GetText() {
		return dialog->GetText();
	}
	void SetText(const char* text) {
		dialog->SetText(text);
	}
private:
	const char* title;
	FwGui::TextEntryDialog* dialog;
	State* acceptState;
	State* cancelState;
};

class WifiHostEntryState : public TextEntryState {
public:
	WifiHostEntryState()
		: TextEntryState("SERVER ADDRESS", setupWifiState, mainMenuState) {
	}
	virtual void Enter() {
		TextEntryState::Enter();
		SetText(settings->GetServerAddress());
	}
	virtual void Exit() {
		const char* address = GetText();
		if(*address != '\0')
			settings->SetServerAddress(address);
		TextEntryState::Exit();
	}
};

class WifiChatEntryState : public TextEntryState {
public:
	WifiChatEntryState() 
		: TextEntryState("CHAT MESSAGE", wifiMenuState, wifiMenuState) {
	}

	virtual void Exit() {
		const char *text = GetText();
		if(strlen(text) != 0) {
			ChatMessage message;
			strcpy(message.text, text);
			message.text[MAX_CHAT_LENGTH] = '\0';
			connection->SendMessage(message);
		}
		TextEntryState::Exit();
	}
};

class WifiMenuState : public State {
public:
	virtual void Enter() {
		SetInfoMessage message;
		message.level = level;
		message.ready = false;
		message.typing = false;
		connection->SendMessage(message);
		
		dialog = new WifiMenuDialog();
		gui->SetActiveDialog(dialog);
		
		char info[1024];
		snprintf(
			info,
			sizeof(info),
			"You have %i wins. Selected level %i.",
			connection->GetWins(),
			level+1);
		dialog->SetInfo(info);
	}
	
	virtual void Tick() {
		SetInfoMessage message;
		switch(dialog->selection) {
		case WMSEL_NONE:
			if(connection->IsState(SERVERSTATE_GAME_STARTED)) {
				nextState = wifiGameState;
			}
			break;

		case WMSEL_READY:
			message.level = level;
			message.ready = true;
			message.typing = false;
			connection->SendMessage(message);
			break;

		case WMSEL_NOT_READY:
			message.level = level;
			message.ready = false;
			message.typing = false;
			connection->SendMessage(message);
			break;

		case WMSEL_LEVEL:
			nextState = wifiLevelState;
			break;
			
		case WMSEL_MESSAGE:
			{
				SetInfoMessage message;
				message.level = level;
				message.ready = false;
				message.typing = true;
				connection->SendMessage(message);		
				nextState = wifiChatEntryState;
			}
			break;

		case WMSEL_QUIT:
			connection->Shutdown();
			nextState = mainMenuState;
			break;
		}
		dialog->selection = WMSEL_NONE;
	}
	
	virtual void Exit() {
		gui->SetActiveDialog(NULL);
		delete dialog;
		dialog = NULL;
	}
private:
	WifiMenuDialog* dialog;
};

class GameState : public State {
public:
	GameState(State* finishState)
		: finishState(finishState) {
	}
	virtual void Enter() {
		PlatformGraphics::InitMainScreen();
		gameEndTimer = GAME_END_DELAY;
	}
	virtual void Tick() {
#ifdef DEBUG		// In debug mode, you can step frames with the select button
		if(!g_game->paused || keysDown() & KEY_SELECT)
#else
		if(!g_game->paused)
#endif
			g_game->Tick();
		
		g_game->Draw();

		if(g_game->field->IsState(PFS_DEAD))
		{
			gameEndTimer--;
			if(gameEndTimer == 0)
			{
				nextState = finishState;
			}
		}
	}
	virtual void Exit() {
		gui->SetListener(NULL);
		delete g_game;
		g_game = NULL;
	}
private:
	State* finishState;
	int gameEndTimer;
};

class EndlessGameState : public GameState {
public:
	EndlessGameState()
		: GameState(mainMenuState) {
	}
	virtual void Enter() {
		GameState::Enter();
		g_game = new Game(level, false, NULL, NULL);
		g_game->Start();
		gui->SetListener(g_game);
	}
	virtual void Exit() {
		settings->UpdateHighscore(0, level, name, g_game->GetScore());
		GameState::Exit();
	}
};
	
class VsSelfGameState : public GameState {
public:
	VsSelfGameState()
		: GameState(mainMenuState) {
	}
	virtual void Enter() {
		GameState::Enter();
		g_game = new Game(level, true, NULL, NULL);
		g_game->Start();
		gui->SetListener(g_game);
	}
	virtual void Exit() {
		settings->UpdateHighscore(1, level, name, g_game->GetScore());
		GameState::Exit();
	}
};

class WifiGameState : public GameState {
public:
	WifiGameState()
		: GameState(waitForGameEndState) {
	}
	virtual void Enter() {
		GameState::Enter();
		g_game = new Game(level, true, connection, connectionManager);
		g_game->Start();
		gui->SetListener(g_game);
	}
	virtual void Tick() {
		GameState::Tick();
		
		if(connection->IsState(SERVERSTATE_GAME_ENDED))
		{
			g_game->field->SetState(PFS_DEAD);
		}
	}
};

class WaitForServerState : public State {
public:
	WaitForServerState(ServerState desiredState, State* acceptState)
		: dialog(NULL), desiredState(desiredState), acceptState(acceptState) {
	}
	virtual void Enter() {
		dialog = new StatusDialog("PLEASE WAIT");
		gui->SetActiveDialog(dialog);
		dialog->SetStatus("Waiting for server...");
	}
	virtual void Tick() {
		if (dialog->abort) {
			dialog->abort = false;
			connection->Shutdown();
		}
		
		if(connection->IsState(desiredState)) {
			nextState = acceptState;
		}
	}
	virtual void Exit() {
	}
private:
	StatusDialog* dialog;
	ServerState desiredState;
	State* acceptState;
};

class SetupWifiState : public State {
public:
	SetupWifiState() 
		: dialog(NULL) {
	}
	virtual void Enter() {
		PlatformGraphics::InitSubScreen(true);
		g_fieldGraphics->ClearChat();

		dialog = new StatusDialog("PLEASE WAIT");
		gui->SetActiveDialog(dialog);
		if(hasSetupWifi) {
			nextState = wifiState;
			return;
		}
		SetupWifi();
		status = ASSOCSTATUS_DISCONNECTED;
	}
	virtual void Tick() {
		if (dialog->abort) {
			nextState = mainMenuState;
			return;
		}
		
		WIFI_ASSOCSTATUS newStatus = (WIFI_ASSOCSTATUS)Wifi_AssocStatus();
		if(status != newStatus) {
			status = newStatus;
			static const char* statusMessages[] = {
				"Disconnected",
				"Searching...",
				"Authenticating...",
				"Associating...",
				"Acquiring DHCP...",
				"Associated",
				"Cannot connect"
			};
			char statusString[1024];
			sprintf(
				statusString,
				"Connecting to Access Point\n%s",
				(status <= 6) ? statusMessages[status] : "???");
			dialog->SetStatus(statusString);
				
			if(status == ASSOCSTATUS_ASSOCIATED) {
				hasSetupWifi = true;
				nextState = wifiState;
			}
		}
	}

	virtual void Exit() {
#ifdef ARM9
		unsigned long ip = Wifi_GetIP();
		char str[256];
		snprintf(str, 256, "My IP: %lu.%lu.%lu.%lu",
			(ip >>  0) & 0xFF,
			(ip >>  8) & 0xFF,
			(ip >> 16) & 0xFF,
			(ip >> 24) & 0xFF);
		g_fieldGraphics->AddChat(str);
#endif

		gui->SetActiveDialog(NULL);
		delete dialog;
		dialog = NULL;
	}
private:
	StatusDialog* dialog;
	WIFI_ASSOCSTATUS status;
};

class WifiState : public State {
	virtual void Enter() {
		currentSubState = NULL;

		if(hostGame) {
			UdpSocket* socket = new UdpSocket();
			if(!socket->Bind(13687)) {
				nextState = mainMenuState;
				return;
			}

			serverGame = new ServerGame();
			connectionManager = new UdpConnectionManager(MAX_PLAYERS, socket, serverGame);
			connection = new ServerConnection(name);
			LoopbackConnection* a = new LoopbackConnection(serverGame);
			LoopbackConnection* b = new LoopbackConnection(connection);
			a->Connect(b);
			connectionManager->SetBroadcastExtraConnection(a);
		} else {
			serverGame = NULL;
			connection = new ServerConnection(name);
			UdpConnectionManager* udpConnectionManager =
				new UdpConnectionManager(1, new UdpSocket(), connection);
			connectionManager = udpConnectionManager;
			Connection* result =
				udpConnectionManager->CreateConnection(settings->GetServerAddress(), 13687);
			if (result == NULL) {
				nextState = mainMenuState;
				return;
			}
		}
		
		currentSubState = waitForServerAcceptState;
		currentSubState->Enter();
	}

	virtual void Tick() {
		if(currentSubState != NULL) {
			currentSubState->Tick();
		}
		
		if (nextState == mainMenuState) {
			return;
		}

		if (nextState != NULL) {
			currentSubState->Exit();
			currentSubState = nextState;
			nextState = NULL;
			currentSubState->Enter();
		}
		
		if (connection->IsState(SERVERSTATE_DISCONNECTED)) {
			nextState = mainMenuState;
			return;
		}

		connectionManager->Tick();
		if(serverGame != NULL) {
			serverGame->Tick();
		}
	}
	
	virtual void Exit() {
		if(currentSubState != NULL) {
			currentSubState->Exit();
		}
		currentSubState = NULL;
		delete connectionManager;
		connectionManager = NULL;
		delete connection;
		connection = NULL;
	}
private:
	State * currentSubState;
	ServerGame* serverGame;
};

class LocalWifiState : public State {
	virtual void Enter() {
		PlatformGraphics::InitSubScreen(true);
		currentSubState = NULL;
		
		if(!hasSetupLocalWifi) {
			SetupLocalWifi();
			hasSetupLocalWifi = true;
		}
		
		if(hostGame) {
			serverGame = new ServerGame();
			LocalConnectionManager* localConnectionManager =
				new LocalConnectionManager(MAX_PLAYERS, serverGame);
			connectionManager = localConnectionManager;
			if(!localConnectionManager->HostGame()) {
				nextState = mainMenuState;
				return;
			}
			connection = new ServerConnection(name);
			LoopbackConnection* a = new LoopbackConnection(serverGame);
			LoopbackConnection* b = new LoopbackConnection(connection);
			a->Connect(b);
			connectionManager->SetBroadcastExtraConnection(a);
		} else {
			serverGame = NULL;
			connection = new ServerConnection(name);
			LocalConnectionManager* localConnectionManager =
				new LocalConnectionManager(1, connection);
			connectionManager = localConnectionManager;
			if(localConnectionManager->JoinGame() == NULL) {
				nextState = mainMenuState;
				return;
			}
		}
		
		currentSubState = waitForServerAcceptState;
		currentSubState->Enter();
	}

	virtual void Tick() {
		if(currentSubState != NULL) {
			currentSubState->Tick();
		}
		
		if (nextState == mainMenuState) {
			return;
		}

		if (nextState != NULL) {
			currentSubState->Exit();
			currentSubState = nextState;
			nextState = NULL;
			currentSubState->Enter();
		}
		
		if (connection->IsState(SERVERSTATE_DISCONNECTED)) {
			nextState = mainMenuState;
			return;
		}

		connectionManager->Tick();
		if(serverGame != NULL) {
			serverGame->Tick();
		}
	}
	
	virtual void Exit() {
		if(currentSubState != NULL) {
			currentSubState->Exit();
		}
		currentSubState = NULL;
		delete connectionManager;
		connectionManager = NULL;
		delete connection;
		connection = NULL;
		delete serverGame;
		serverGame = NULL;
	}
private:
	State * currentSubState;
	ServerGame* serverGame;
};

class SplashScreenState : public State {
public:
	virtual void Enter() {
		ShowSplashScreen();

		// load music
		Sound::InitMusic();
		Sound::LoadMusic();
	}

	void Tick() {
		nextState = mainMenuState;
	}
	
	void Exit() {
		HideSplashScreen();
		
		InitSettings();
	}
};

void InitStates()
{
	/* Main game state transitions
	
	mainMenuState -> select (endlessLevelState, vsSelfLevelState, wifiTypeMenuState, highScoreState)
	endlessLevelState -> choice (endlessGameState, mainMenuState)
	endlessGameState -> always (mainMenuState)
	vsSelfLevelState -> choice (vsSelfGameState, mainMenuState)
	vsSelfGameState -> always (mainMenuState)
	highScoreState -> always (mainMenuState)
	wifiTypeMenuState -> choice (setupWifiState, localWifiState, wifiHostEntryState, mainMenuState)
	wifiHostEntryState -> choice (setupWifiState, mainMenuState)
	setupWifiState -> choice (wifiState, mainMenuState)
	wifiState -> always (mainMenuState)
	localWifiState -> always (mainMenuState)
	*/
	
	/* Wifi state transitions (inside wifiState and localWifiState)
	
	waitForServerAcceptState -> always (wifiMenuState)
	wifiMenuState -> select (wifiGameState, wifiLevelState, wifiChatEntryState)
	wifiGameState -> always (waitForGameEndState)
	waitForGameEndState -> always (wifiMenuState)
	wifiLevelState -> always (wifiMenuState)
	wifiChatEntryState -> always (wifiMenuState)
	*/

	highscoreState = new HighscoreState;
	mainMenuState = new MainMenuState;
	wifiTypeMenuState = new WifiTypeMenuState;
	endlessGameState = new EndlessGameState;
	vsSelfGameState = new VsSelfGameState;
	wifiState = new WifiState;
	localWifiState = new LocalWifiState;
	setupWifiState = new SetupWifiState;	
	wifiHostEntryState = new WifiHostEntryState;
	wifiMenuState = new WifiMenuState;
	wifiChatEntryState = new WifiChatEntryState;
	waitForGameEndState = new WaitForServerState(SERVERSTATE_GAME_ENDED, wifiMenuState);
	wifiGameState = new WifiGameState;
	splashScreenState = new SplashScreenState;
	waitForServerAcceptState = new WaitForServerState(SERVERSTATE_ACCEPTED, wifiMenuState);
	endlessLevelState = new LevelState(endlessGameState, mainMenuState);
	vsSelfLevelState = new LevelState(vsSelfGameState, mainMenuState);
	wifiLevelState = new LevelState(wifiMenuState, wifiMenuState);

	currentState = splashScreenState;
	currentState->Enter();
}

bool StateTick()
{
	currentState->Tick();

	if (nextState != NULL)
	{
		currentState->Exit();
		currentState = nextState;
		nextState = NULL;
		currentState->Enter();
#if 0
		playSound(&menuSound);
#endif
	}

	if(!gui->Tick())
		return false;
	Sound::UpdateMusic();
	
	if(hasSetupLocalWifi) {
		LOBBY_Update();
	}

	return true;
}


void InitGui()
{
	gui = new FwGui::Driver();
	FwGui::backgroundImage = GetMenuBackground();
	FwGui::selectedColor = FwGui::Color(255, 255, 255);
	FwGui::labelTextColor = FwGui::Color(0, 0, 0);
	FwGui::enabledButtonColor = FwGui::Color(0, 0, 0, 190);
	FwGui::disabledButtonColor = FwGui::Color(171, 171, 171, 190);
	FwGui::buttonTextColor = FwGui::Color(255, 255, 255);
	FwGui::enabledEditBoxColor = FwGui::Color(255, 255, 255, 190);
#if 0
	keySound.data = menu1_bin;
	keySound.len = menu1_bin_size;
	keySound.rate = 22050;
	keySound.vol = 64;
	keySound.pan = 64;
	keySound.format = 2;
	FwGui::keyClickSound = &keySound;
	menuSound.data = menu2_bin;
	menuSound.len = menu2_bin_size;
	menuSound.rate = 22050;
	menuSound.vol = 64;
	menuSound.pan = 64;
	menuSound.format = 2;
#endif
}
