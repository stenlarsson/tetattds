#pragma once

#include "effect.h"
#include "game.h"

class Sprite;

enum ComboType
{
	COMBO_4 = TILE_COMBO - 4,
	COMBO_2X = TILE_CHAIN - 2
};

#define NUM_EGGS 6

class EffCombo : public Effect
{
public:
	EffCombo(int pos, ComboType type, int count);
	~EffCombo();

	void Draw();
	void Tick();

private:
	Sprite* sign;
	Sprite* eggs[COMBO_NUM_EGGS];
	double eggRad;
};
