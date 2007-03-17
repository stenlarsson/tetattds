#pragma once

#include "sprite.h"

class Marker
{
  public:
	Marker(bool large) {
		int tile = large ? TILE_LARGE_MARKER : TILE_SMALL_MARKER;
		sprite = Sprite::GetSprite(0, 0, MARKER_PRIORITY, SSIZE_32x16, Anim(tile), false, false);
		Hide();
	}

	~Marker() {
		Sprite::ReleaseSprite(sprite);
	}

	void Draw(int x, int y) { sprite->SetPos(x, y); sprite->Draw(); }
	void Hide() { sprite->Disable(); }

  private:
	Sprite* sprite;
};
