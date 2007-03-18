#include <nds.h>
#include "tetattds.h"
#include "sprite.h"

#define MAX_SPRITES 128

SpriteEntry* spriteEntries = (SpriteEntry*)OAM;

Sprite* Sprite::sprites = NULL;
Sprite* Sprite::firstFreeSprite = NULL;

/**
 * Set up linked list of sprites
 */
void Sprite::InitSprites()
{
	if(sprites == NULL)
		sprites = (Sprite*)new char[sizeof(Sprite)*MAX_SPRITES];

	for(int i = 0; i < MAX_SPRITES-1; i++)
	{
		sprites[i].spriteIndex = i;
		sprites[i].nextFreeSprite = &sprites[i+1];
		sprites[i].Disable();
	}

	sprites[MAX_SPRITES-1].spriteIndex = MAX_SPRITES-1;
	sprites[MAX_SPRITES-1].nextFreeSprite = NULL;
	sprites[MAX_SPRITES-1].Disable();

	firstFreeSprite = &sprites[0];
}

void* Sprite::operator new (size_t size)
{
	Sprite* sprite = firstFreeSprite;

	if(sprite == NULL)
	{
		printf("BUG:out of sprites.\nPlease report me.\n"); // BUG! =D
		for(;;);
	}

	firstFreeSprite = sprite->nextFreeSprite;
	
	return sprite;
}

void Sprite::operator delete (void *p)
{
	Sprite *sprite = (Sprite *)p;
	sprite->nextFreeSprite = firstFreeSprite;
	firstFreeSprite = sprite;	
}

Sprite::Sprite(
	int x, int y,
	int priority, 
	SpriteSize size,
	Anim const & anim,
	bool flipX, bool flipY)
	: anim(anim),
		x(x), y(y),
	  attr0(ATTR0_COLOR_256),
		attr1(0),
		attr2(ATTR2_PRIORITY(priority))
{
	switch(size)
	{
	case SSIZE_8x8:
		attr0 |= ATTR0_SQUARE;
		attr1 |= ATTR1_SIZE_8;
		break;

	case SSIZE_16x16:
		attr0 |= ATTR0_SQUARE;
		attr1 |= ATTR1_SIZE_16;
		break;

	case SSIZE_32x16:
		attr0 |= ATTR0_WIDE;
		attr1 |= ATTR1_SIZE_32;
		break;
	}

	if(flipX)
		attr1 |= ATTR1_FLIP_X;
	if(flipY)
		attr1 |= ATTR1_FLIP_Y;
}

Sprite::~Sprite()
{
	Disable();
}

void Sprite::Draw()
{
	if(y < -16 || y > 192 || x < -16 || x > 256) // No drawing of sprites offscreen please
		spriteEntries[spriteIndex].attribute[0] = ATTR0_DISABLED;
	else
	{
		int tile = anim.GetFrame();

		spriteEntries[spriteIndex].attribute[0] = (y & 0xFF) | attr0;
		spriteEntries[spriteIndex].attribute[1] = (x & 0x1FF) | attr1;
		spriteEntries[spriteIndex].attribute[2] = ((tile<<3) & 0x3FF) | attr2;
	}
}

void Sprite::Disable()
{
	spriteEntries[spriteIndex].attribute[0] = ATTR0_DISABLED;
}
