#pragma once

#include "sprite.h"

Sprite* GetMarker(bool large) {
	return Sprite::GetSprite(
		0, 0,
		MARKER_PRIORITY,
		large ? SSIZE_32x16 : SSIZE_16x16,
		Anim(large ? TILE_LARGE_MARKER : TILE_SMALL_MARKER),
		false, false);
}

