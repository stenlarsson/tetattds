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
		sprites = new Sprite[MAX_SPRITES];

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

/**
 * Return next free sprite, or NULL if no free are available.
 */
Sprite* Sprite::GetSprite(int x, int y, int priority, SpriteSize size, bool flipX, bool flipY)
{
	Sprite* sprite = firstFreeSprite;

	if(sprite == NULL)
	{
		printf("BUG:out of sprites.\nPlease report me.\n"); // BUG! =D
		for(;;);
	}

	firstFreeSprite = sprite->nextFreeSprite;
	sprite->x = x;
	sprite->y = y;

	sprite->attr0 = 0;
	sprite->attr1 = 0;
	sprite->attr2 = 0;

	switch(size)
	{
	case SSIZE_8x8:
		sprite->attr0 |= ATTR0_SQUARE;
		sprite->attr1 |= ATTR1_SIZE_8;
		break;

	case SSIZE_16x16:
		sprite->attr0 |= ATTR0_SQUARE;
		sprite->attr1 |= ATTR1_SIZE_16;
		break;

	case SSIZE_32x16:
		sprite->attr0 |= ATTR0_WIDE;
		sprite->attr1 |= ATTR1_SIZE_32;
		break;
	}

	sprite->attr0 |= ATTR0_COLOR_256;
	if(flipX)
		sprite->attr1 |= ATTR1_FLIP_X;
	if(flipY)
		sprite->attr1 |= ATTR1_FLIP_Y;
	sprite->attr2 |= ATTR2_PRIORITY(priority);

	sprite->anim = NULL;

	return sprite;
}

void Sprite::ReleaseSprite(Sprite* sprite)
{
	DEL(sprite->anim);
	sprite->Disable();

	sprite->nextFreeSprite = firstFreeSprite;
	firstFreeSprite = sprite;
}

void Sprite::Draw()
{
	if(anim != NULL)
	{
		tile = anim->GetFrame();
	}

	if(y < -16 || y > 192 || x < -16 || x > 256) // No drawing of sprites offscreen please
		spriteEntries[spriteIndex].attribute[0] = ATTR0_DISABLED;
	else
	{
		spriteEntries[spriteIndex].attribute[0] = (y & 0xFF) | attr0;
		spriteEntries[spriteIndex].attribute[1] = (x & 0x1FF) | attr1;
		spriteEntries[spriteIndex].attribute[2] = ((tile<<3) & 0x3FF) | attr2;
	}
}

void Sprite::Disable()
{
	spriteEntries[spriteIndex].attribute[0] = ATTR0_DISABLED;
}
