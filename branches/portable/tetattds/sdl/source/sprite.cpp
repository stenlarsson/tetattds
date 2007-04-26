#include "tetattds.h"
#include "sprite.h"
#include "fieldgraphics.h"
#include <SDL.h>

#define MAX_SPRITES 128

extern SDL_Surface *sprites;

static int allocatedSpriteCount = 0;

void* Sprite::operator new (size_t size)
{
	ASSERT(allocatedSpriteCount < MAX_SPRITES);

	allocatedSpriteCount++;
	return ::operator new(size);
}

void Sprite::operator delete (void *p)
{
	ASSERT(allocatedSpriteCount > 0);
	
	allocatedSpriteCount--;
	::operator delete(p);
}

Sprite::Sprite(
	int x, int y, 
	int priority,
	SpriteSize size,
	Anim const & anim,
	bool flipX, bool flipY)
	: anim(anim),
		x(x), y(y+192),
	  priority(priority),
		size(size),
		flipX(flipX), flipY(flipY)
{
}

Sprite::~Sprite()
{
}

void Sprite::Draw()
{
	// TODO: This doesn't really work, since we need to process
	//       the sprites in a very specific order?
	
	int tile = anim.GetFrame();

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
			tilerect.y = (tile / 8) * 16 + 8;
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
