#include "tetattds.h"
#include "util.h"
#include <stdarg.h>

void PrintSpinner()
{
	static int frame = 0;
	static char frames[] = {'|', '/', '-', '\\'};
	printf("%c\e[1D", frames[frame]);
	frame = (frame + 1) % sizeof(frames);
}

void PrintStatus(char* format, ...)
{
	va_list args;
	va_start( args, format );
	
	vprintf( format, args );
	va_end( args );
}
