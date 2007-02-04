#include "tetattds.h"
#include "effpop.h"
#include "sprite.h"
#include "anim.h"

EffPop::EffPop(int x, int y, int /*strength*/)
{
	Anim anim;

	//int i;

	/*if(strength > 1)
	{
		anim.Init(9, ANIM_ONCE);
		anim.AddFrame(TILE_EGG_SHELL+0,60);
		anim.AddFrame(TILE_EGG_SHELL+1,60);
		//anim.AddFrame(TILE_EGG_SHELL+8,1);
		anim.AddFrame(TILE_EGG_SHELL+2,60);
		//anim.AddFrame(TILE_EGG_SHELL+8,1);
		anim.AddFrame(TILE_EGG_SHELL+3,60);
		anim.AddFrame(TILE_EGG_SHELL+4,60);
		anim.AddFrame(TILE_EGG_SHELL+5,60);
		anim.AddFrame(TILE_EGG_SHELL+6,60);
		anim.AddFrame(TILE_EGG_SHELL+7,60);
	}
	else
	{
		anim.Init(8, ANIM_ONCE);
		for(i=0;i<8;i++)
			anim.AddFrame(TILE_EGG_SHELL+i,60);*/
		anim.Init(9, ANIM_ONCE);
		anim.AddFrame(TILE_EGG_SHELL+0,3);
		anim.AddFrame(TILE_EGG_SHELL+1,3);
		anim.AddFrame(TILE_EGG_SHELL+2,3);
		anim.AddFrame(TILE_EGG_SHELL+3,3);
		anim.AddFrame(TILE_EGG_SHELL+4,3);
		anim.AddFrame(TILE_EGG_SHELL+5,3);
		anim.AddFrame(TILE_EGG_SHELL+6,3);
		anim.AddFrame(TILE_EGG_SHELL+7,3);
		//! \todo Hack to get the last frame.
		anim.AddFrame(TILE_EGG_SHELL+26,3);

	/*}*/

	int off = BLOCKSIZE>>1; // Offset
	
	spriteA = Sprite::GetSprite(x - off, y - off, 1, SSIZE_16x16, 0);
	spriteA->SetAnim(&anim);
	
	spriteB = Sprite::GetSprite(x + off, y - off, 1, SSIZE_16x16, ATTR1_FLIP_X);
	spriteB->SetAnim(&anim);

	spriteC = Sprite::GetSprite(x - off, y + off, 1, SSIZE_16x16, ATTR1_FLIP_Y);
	spriteC->SetAnim(&anim);

	spriteD = Sprite::GetSprite(x + off, y + off, 1, SSIZE_16x16, ATTR1_FLIP_X | ATTR1_FLIP_Y);
	spriteD->SetAnim(&anim);

	mov = 3;
	duration = 9 * 3;
}

EffPop::~EffPop()
{
	Sprite::ReleaseSprite(spriteA);
	Sprite::ReleaseSprite(spriteB);
	Sprite::ReleaseSprite(spriteC);
	Sprite::ReleaseSprite(spriteD);
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
	spriteA->Tick();
	spriteB->Tick();
	spriteC->Tick();
	spriteD->Tick();
	spriteA->Move(-mov, -mov);
	spriteB->Move(mov, -mov);
	spriteC->Move(-mov, mov);
	spriteD->Move(mov, mov);
	if(mov > 0)
		if(duration & BIT(0))
			mov--;
	duration--;
}
