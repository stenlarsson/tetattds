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

	void SetOffset(int x, int y) { spriteA->Move(x,y); spriteB->Move(x,y); spriteC->Move(x,y); spriteD->Move(x,y); }

  private:
	int mov;
	Sprite* spriteA;
	Sprite* spriteB;
	Sprite* spriteC;
	Sprite* spriteD;
};
