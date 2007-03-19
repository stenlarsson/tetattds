#pragma once

#include "effect.h"

class EffReady : public Effect
{
public:
	EffReady();
	~EffReady();

	void Draw();
	void Tick();
};
