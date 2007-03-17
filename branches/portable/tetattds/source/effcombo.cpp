#include "tetattds.h"
#include <math.h>
#include "effcombo.h"
#include "sprite.h"
#include "anim.h"
#include "game.h"
#include "playfield.h"

EffCombo::EffCombo(int x, int y, int type)
{
	ASSERT(g_game != NULL);

	XOffset = x;
	YOffset = y;
	Anim anim(type);
	sign = Sprite::GetSprite(x, y, COMBO_COUNTER_PRIORITY, SSIZE_16x16, anim, false, false);
	const LevelData* data = g_game->GetLevelData();
	duration = data->effComboDuration;

	Anim anim2(TILE_EGG);
	for(int i = 0; i < COMBO_NUM_EGGS; i++)
		eggs[i] = Sprite::GetSprite(x, y, COMBO_EGG_PRIORITY, SSIZE_16x16, anim2, false, false);
	eggRad = COMBO_EGG_RADIUS;
}

EffCombo::~EffCombo()
{
	Sprite::ReleaseSprite(sign);
	for(int i = 0; i < COMBO_NUM_EGGS; i++)
		Sprite::ReleaseSprite(eggs[i]);
}

void EffCombo::Draw()
{
	for(int i = 0; i < COMBO_NUM_EGGS; i++)
		eggs[i]->Draw();
	sign->Draw();
}

void EffCombo::Tick()
{
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
	duration--;
	eggRad *= 0.9;
}

void EffCombo::SetOffset(int x, int y)
{
	for(int i = 0; i < COMBO_NUM_EGGS; i++)
		eggs[i]->Move(x, y);
	sign->Move(x, y);
	XOffset += x;
	YOffset += y;
}
