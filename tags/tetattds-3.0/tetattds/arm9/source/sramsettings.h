#pragma once

#include "settings.h"

class SramSettings : public Settings
{
public:
	SramSettings();
	virtual ~SramSettings();

	virtual void Load();
	virtual void Save();
	
private:
	void MemCopy(void* dest, const void* src, size_t size);
	int MemCompare(void* dest, const void* src, size_t size);
	void ZeroMemory(void* dest, size_t size);
};
