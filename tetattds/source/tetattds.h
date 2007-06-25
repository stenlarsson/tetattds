#ifndef WIN32
#include <unistd.h>
#include <stdint.h>
#else
#define NOMINMAX
#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS
#define snprintf _snprintf
// SDL.h can be used instead of stdint.h
#include <SDL.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"

#define DEL(p) {if(p) {delete (p); (p)=NULL;}}

#define DEBUGMONGO printf("%s\n", __PRETTY_FUNCTION__)

#define ASSERT(x) if(!(x)) {\
	printf("assertion failed %s:%i\n", __FILE__, __LINE__);\
	for(;;);\
}

extern void PrintStatus(char* format, ...);

#define PROFILE(timer, function)\
	{\
		TIMER_DATA(timer) = 0;\
		TIMER_CR(timer) = TIMER_DIV_1024;\
		static unsigned int time = 0;\
		function;\
		time += TIMER_DATA(timer);\
		TIMER_CR(timer) = 0;\
		static int count = 0;\
		count++;\
		if(count == 60) {\
			printf(#function ": %f\n", time/(33554432.0/1024.0)/60.0);\
			count = 0;\
			time = 0;\
		}\
	}

#define PROFILE2(variable, timer, function)\
	TIMER_DATA(timer) = 0;\
	TIMER_CR(timer) = TIMER_DIV_1024;\
	function;\
	variable = TIMER_DATA(timer)/(33554432.0/1024.0);\
	TIMER_CR(timer) = 0;

// Determine the number of elements in an array
#define COUNT_OF(array) sizeof(array)/sizeof(array[0])

