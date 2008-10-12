#include "tetattds.h"
#include "effpop.h"
#include "sprite.h"
#include "anim.h"

EffPop::EffPop(int pos, int /*strength*/)
	: Effect(pos, 9*3+1),
		mov(3)
{
	AnimFrame frames[] = {
		AnimFrame(TILE_EGG_SHELL+0,3),
		AnimFrame(TILE_EGG_SHELL+1,3),
		AnimFrame(TILE_EGG_SHELL+2,3),
		AnimFrame(TILE_EGG_SHELL+3,3),
		AnimFrame(TILE_EGG_SHELL+4,3),
		AnimFrame(TILE_EGG_SHELL+5,3),
		AnimFrame(TILE_EGG_SHELL+6,3),
		AnimFrame(TILE_EGG_SHELL+7,3),
		//! \todo Hack to get the last frame.
		AnimFrame(TILE_EGG_SHELL+26,3),
	};

	Anim anim(ANIM_ONCE, frames, COUNT_OF(frames));

	int off = BLOCKSIZE>>1; // Offset
	
	spriteA = new Sprite(XOffset - off, YOffset - off, 1, SSIZE_16x16, anim, false, false);
	spriteB = new Sprite(XOffset + off, YOffset - off, 1, SSIZE_16x16, anim, true, false);
	spriteC = new Sprite(XOffset - off, YOffset + off, 1, SSIZE_16x16, anim, false, true);
	spriteD = new Sprite(XOffset + off, YOffset + off, 1, SSIZE_16x16, anim, true, true);
}

EffPop::~EffPop()
{
	delete spriteA;
	delete spriteB;
	delete spriteC;
	delete spriteD;
}

void EffPop::Draw()
{
	spriteA->Draw();
	spriteB->Draw();
	spriteC->Draw();
	spriteD->Draw();
}

void EffPop::Tick()
{
	Effect::Tick();
	spriteA->Tick();
	spriteB->Tick();
	spriteC->Tick();
	spriteD->Tick();
	spriteA->Move(-mov, -mov);
	spriteB->Move(mov, -mov);
	spriteC->Move(-mov, mov);
	spriteD->Move(mov, mov);
	if(mov > 0)
		if(duration & 1)
			mov--;
}
