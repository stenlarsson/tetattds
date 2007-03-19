#pragma once

#include "effect.h"
#include "sprite.h"

class EffPop : public Effect
{
public:
	EffPop(int x, int y, int strength);
	~EffPop();

	void Draw();
	void Tick();

private:
	int mov;
	Sprite* spriteA;
	Sprite* spriteB;
	Sprite* spriteC;
	Sprite* spriteD;
};
