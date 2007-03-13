#pragma once

#include "anim.h"

enum SpriteSize
{
	SSIZE_8x8,
	SSIZE_16x16,
	SSIZE_32x16
};

class Sprite
{
public:
	static void InitSprites();
	static Sprite* GetSprite(int x, int y, int priority, SpriteSize size, bool flipX, bool flipY);
	static void ReleaseSprite(Sprite* sprite);

	void SetAnim(Anim* newAnim) { ASSERT(newAnim != NULL); anim = newAnim->Copy(); }
	void SetTile(int newTile) { tile = newTile; }
	void Draw();
	void Disable();
	void Tick() { ASSERT(anim != NULL); anim->Tick(); tile = anim->GetFrame(); }
	bool IsDone() { ASSERT(anim != NULL); return (anim->GetType() == ANIM_STATIC); }
	void Move(int x, int y) { this->x += x; this->y += y; }
	void SetPos(int x, int y) { this->x = x; this->y = y; }
	void GetPos(int& x, int& y) { x = this->x; y = this->y; }

private:
	static Sprite* sprites_data;
	static Sprite* firstFreeSprite;
	int spriteIndex;
	Sprite* nextFreeSprite;

	Sprite() {}
	~Sprite() {}

	Anim* anim;
	int x;
	int y;
	int tile;
	int priority;
	SpriteSize size;
	bool flipX;
	bool flipY;
};
