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
	static void* operator new (size_t size);
	static void operator delete (void *p);

	Sprite(int x, int y, int priority, SpriteSize size, Anim const & anim, bool flipX, bool flipY);
	~Sprite();


	void Draw();
	void Disable();
	void Tick() { anim.Tick(); }
	bool IsDone() { return anim.IsDone(); }
	void Move(int x, int y) { this->x += x; this->y += y; }
	void SetPos(int x, int y) { this->x = x; this->y = y; }
	void GetPos(int& x, int& y) { x = this->x; y = this->y; }


private:
	static Sprite* sprites;
	static Sprite* firstFreeSprite;
	int spriteIndex;
	Sprite* nextFreeSprite;

	Anim anim;
	int x;
	int y;
	int attr0;
	int attr1;
	int attr2;
};
