#pragma once
#include <stdlib.h>

#define ASSERT(x) if(!(x)) {\
	printf("assertion failed %s:%i\n", __FILE__, __LINE__);\
	for(;;);\
}

#ifdef ARM9
#include <safe_malloc.h>
#define malloc(x) safe_malloc(x)
#define free(x) safe_free(x)
#endif
