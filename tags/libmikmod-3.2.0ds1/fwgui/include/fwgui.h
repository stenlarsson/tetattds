#pragma once

#ifndef WIN32
#include <stdint.h>
#else
#define NOMINMAX
#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#define snprintf _snprintf
#define strdup _strdup
#include <SDL.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#define FWGUI_ASSERT(x) if(!(x)) {\
	printf("assertion failed %s:%i\n", __FILE__, __LINE__);\
	for(;;);\
}
