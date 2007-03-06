#pragma once

#include "sprite.h"

class Marker
{
  public:
	Marker(bool large) {
		if(large)
		{
			sprite = Sprite::GetSprite(0, 0, MARKER_PRIORITY, SSIZE_32x16, 0);
			sprite->SetTile(TILE_LARGE_MARKER);
		}
		else
		{
			sprite = Sprite::GetSprite(0, 0, MARKER_PRIORITY, SSIZE_16x16, 0);
			sprite->SetTile(TILE_SMALL_MARKER);
		}
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
