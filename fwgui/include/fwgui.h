#pragma once

#ifndef WIN32
#include <stdint.h>
#else
#define _CRT_SECURE_NO_DEPRECATE
#include <SDL.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#define FWGUI_ASSERT(x) if(!(x)) {\
	printf("assertion failed %s:%i\n", __FILE__, __LINE__);\
	for(;;);\
}
