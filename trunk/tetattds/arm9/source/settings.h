#pragma once

#define NUM_MODES 2
#define NUM_HIGHSCORES 10
#define MAX_NAME_LENGTH 10
#define MAX_SERVER_ADDRESS_LENGTH 100

struct InternalSettings;

class Settings
{
public:
	Settings();
	~Settings();

	void Load();
	void Save();
	
	void GetHighscore(int mode, int level, const char* & name, int & score);
	void UpdateHighscore(int mode, int level, char* name, int score);
	
	const char* GetServerAddress();
	void SetServerAddress(const char* address);
	
private:
	void MemCopy(void* dest, const void* src, size_t size);
	int MemCompare(void* dest, const void* src, size_t size);
	void ZeroMemory(void* dest, size_t size);
	
	InternalSettings* settings;
};
