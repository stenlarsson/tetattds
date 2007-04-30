#include "tetattds.h"
#include <math.h>
#include "effcombo.h"
#include "sprite.h"
#include "anim.h"
#include "game.h"
#include "playfield.h"
#include "util.h"

EffCombo::EffCombo(int x, int y, ComboType type, int count)
	: Effect(x, y, g_game->GetLevelData()->effComboDuration),
		eggRad(COMBO_EGG_RADIUS)
{
	Anim anim(type + count);
	sign = new Sprite(x, y, COMBO_COUNTER_PRIORITY, SSIZE_16x16, anim, false, false);

	Anim anim2(TILE_EGG);
	for(int i = 0; i < COMBO_NUM_EGGS; i++)
		eggs[i] = new Sprite(x, y, COMBO_EGG_PRIORITY, SSIZE_16x16, anim2, false, false);
}

EffCombo::~EffCombo()
{
	delete sign;
	delete_each(eggs, eggs+COMBO_NUM_EGGS);
}

void EffCombo::Draw()
{
	std::for_each(eggs, eggs+COMBO_NUM_EGGS, std::mem_fun(&Sprite::Draw));
	sign->Draw();
}

void EffCombo::Tick()
{
	Effect::Tick();
	int t = abs(duration - 80);
	double r = eggRad + 16;
	double tetha;
	for(int i = 0; i < COMBO_NUM_EGGS; i++)
	{
		tetha = (t+60*i) * (3.141529 / 180) * 7;
		eggs[i]->SetPos((int)(r*cos(tetha)) + XOffset, (int)(r*sin(tetha)) + YOffset);
	}
	YOffset -= duration>>5;
	sign->Move(0,-(duration>>5));
	eggRad *= 0.9;
}
