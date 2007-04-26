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
	static void InitSprites() {}; // Compatibility with nds version
	static void* operator new (size_t size);
	static void operator delete (void *p);
	
	Sprite(
		int x, int y,
		int priority,
		SpriteSize size,
		Anim const & anim,
		bool flipX, bool flipY);
	~Sprite();

	void Draw();
	void Disable();
	void Tick() { anim.Tick(); }
	bool IsDone() { return anim.IsDone(); }
	void Move(int x, int y) { this->x += x; this->y += y; }
	void SetPos(int x, int y) { this->x = x; this->y = y+192; }
	void GetPos(int& x, int& y) { x = this->x; y = this->y-192; }

private:
	Anim anim;
	int x;
	int y;
	int priority;
	SpriteSize size;
	bool flipX;
	bool flipY;
};
