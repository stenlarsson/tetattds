#include "tetattds.h"
#include "util.h"
#include <stdarg.h>

void PrintStatus(char* format, ...)
{
	va_list args;
	va_start( args, format );
	
	vprintf( format, args );
	va_end( args );
}
