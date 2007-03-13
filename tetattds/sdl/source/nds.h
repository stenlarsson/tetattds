#pragma once

#include <stdint.h>
typedef uint16_t u16;

#define BIT(n) (1 << (n))
#undef KEY_B
#undef KEY_START

enum NDSKeys
{
	KEY_A     = BIT(0),
	KEY_B     = BIT(1),
	KEY_START = BIT(2),
	KEY_UP    = BIT(3),
	KEY_DOWN  = BIT(4),
	KEY_LEFT  = BIT(5),
	KEY_RIGHT = BIT(6),
	KEY_X     = BIT(7),
	KEY_Y     = BIT(8),
	KEY_L     = BIT(9),
	KEY_R     = BIT(10),
	KEY_TOUCH = BIT(11),
};

extern uint32_t heldKeys, downKeys, upKeys;

inline uint32_t keysHeld() { return heldKeys; }
inline uint32_t keysDown() { return downKeys; }
inline uint32_t keysUp() { return upKeys; }

void scanKeys();

struct touchPosition {
	int px, py;
};

touchPosition touchReadXY();
