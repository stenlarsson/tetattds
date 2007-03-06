#pragma once

#define ASSERT(x) if(!(x)) {\
	printf("assertion failed %s:%i\n", __FILE__, __LINE__);\
	for(;;);\
}
