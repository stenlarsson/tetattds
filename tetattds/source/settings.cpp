#include "tetattds.h"
#include "settings.h"

struct InternalSettings
{
	char names[NUM_MODES][NUM_HIGHSCORES][MAX_NAME_LENGTH];
	int scores[NUM_MODES][NUM_HIGHSCORES];
	char serverAddress[MAX_SERVER_ADDRESS_LENGTH];
};

/* Settings *****************************************************************/

Settings::Settings()
:	settings(new InternalSettings)
{
	memset(settings, 0, sizeof(InternalSettings));
	strcpy(settings->serverAddress, "walkyrie.se");
}

Settings::~Settings()
{
	delete settings;
}

void Settings::GetHighscore(int mode, int level, const char* & name, int & score)
{
	name = settings->names[mode][level];
	score = settings->scores[mode][level];
}

void Settings::UpdateHighscore(int mode, int level, char* name, int score)
{
	if(score > settings->scores[mode][level])
	{
		strncpy(settings->names[mode][level], name, MAX_NAME_LENGTH);
		settings->scores[mode][level] = score;
	}
}

const char* Settings::GetServerAddress()
{
	return settings->serverAddress;
}

void Settings::SetServerAddress(const char* address)
{
	strncpy(settings->serverAddress, address, MAX_SERVER_ADDRESS_LENGTH);
}

/* FatSettings **************************************************************/

FatSettings::FatSettings()
{
}

FatSettings::~FatSettings()
{
}

void FatSettings::Load()
{
	FILE* f = fopen("tetattds.dat", "rb");
	if(f == NULL) {
		printf("Failed to open tetattds.dat for reading.\n");
		return;
	}
	
	fread(settings, sizeof(InternalSettings), 1, f);
	fclose(f);
}

void FatSettings::Save()
{
	FILE* f = fopen("tetattds.dat", "wb");
	if(f == NULL) {
		printf("Failed to open tetattds.dat for writing.\n");
		return;
	}
	
	fwrite(settings, sizeof(InternalSettings), 1, f);
	fclose(f);
}
