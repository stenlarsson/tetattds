#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define FWGUI_ASSERT(x) if(!(x)) {\
	printf("assertion failed %s:%i\n", __FILE__, __LINE__);\
	for(;;);\
}
