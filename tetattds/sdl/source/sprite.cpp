#include "tetattds.h"
#include "sprite.h"
#include "fieldgraphics.h"
#include <SDL.h>

#define MAX_SPRITES 128

extern SDL_Surface *sprites;
Sprite* Sprite::sprites_data = NULL;
Sprite* Sprite::firstFreeSprite = NULL;

/**
 * Set up linked list of sprites
 */
void Sprite::InitSprites()
{
	if(sprites_data == NULL)
		sprites_data = new Sprite[MAX_SPRITES];

	for(int i = 0; i < MAX_SPRITES-1; i++)
	{
		sprites_data[i].spriteIndex = i;
		sprites_data[i].nextFreeSprite = &sprites_data[i+1];
		sprites_data[i].Disable();
	}

	sprites_data[MAX_SPRITES-1].spriteIndex = MAX_SPRITES-1;
	sprites_data[MAX_SPRITES-1].nextFreeSprite = NULL;
	sprites_data[MAX_SPRITES-1].Disable();

	firstFreeSprite = &sprites_data[0];
}

/**
 * Return next free sprite, or NULL if no free are available.
 */
Sprite* Sprite::GetSprite(int x, int y, int priority, SpriteSize size, Anim const & anim, bool flipX, bool flipY)
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
	sprite->priority = priority;
	sprite->size = size;
	sprite->flipX = flipX;
	sprite->flipY = flipY;
	sprite->anim = new Anim(anim);

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
	// TODO: This doesn't really work, since we need to process
	//       the sprites in a very specific order?
	
	// No drawing of sprites offscreen please
	if(y < -16 || y > 192 || x < -16 || x > 256)
		return;
		
	int tile = anim->GetFrame();

	// TODO: Deal with flipped sprites
	switch (size)
	{
	case SSIZE_16x16:
		{
			SDL_Rect tilerect, destrect;
			tilerect.w = destrect.w = tilerect.h = destrect.h = 16;
			tilerect.x = (tile % 8) * 16;
			tilerect.y = (tile / 8) * 16;
			destrect.x = x;
			destrect.y = y;
			SDL_BlitSurface(sprites, &tilerect, g_fieldGraphics->framebuffer, &destrect);
		}
		break;
	
	case SSIZE_32x16:
		{
			SDL_Rect tilerect, destrect;
			tilerect.w = destrect.w = 16;
			tilerect.h = destrect.h = 8;
			tilerect.x = (tile % 8) * 16;
			tilerect.y = (tile / 8) * 16;
			destrect.x = x;
			destrect.y = y;
			SDL_BlitSurface(sprites, &tilerect, g_fieldGraphics->framebuffer, &destrect);
			tilerect.x = ((tile + 1) % 8) * 16;
			destrect.y = y + 8;
			SDL_BlitSurface(sprites, &tilerect, g_fieldGraphics->framebuffer, &destrect);
			tilerect.y = ((tile + 1) / 8) * 16;
			destrect.x = x + 16;
			SDL_BlitSurface(sprites, &tilerect, g_fieldGraphics->framebuffer, &destrect);
			tilerect.x = (tile % 8) * 16;
			destrect.y = y;
			SDL_BlitSurface(sprites, &tilerect, g_fieldGraphics->framebuffer, &destrect);
		}		
		break;

	case SSIZE_8x8:
		// TODO: Implement
		break;
	}
}

void Sprite::Disable()
{
	// Nothing to do here, we must always refresh
}
