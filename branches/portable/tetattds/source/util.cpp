#include "tetattds.h"
#include "util.h"
#include <stdarg.h>

void* Decompress(const void* source, int* size)
{
	u32 header = *(u32*)source;
	ASSERT((header & 0xFF) == 0x10);

	int uncompressedSize = header >> 8;

	void* temp = malloc(uncompressedSize);

	swiDecompressLZSSWram((void*)source, temp);

	*size = uncompressedSize;
	return temp;
}

void Decompress(void* destination, const void* source)
{
	u32 header = *(u32*)source;
	ASSERT((header & 0xFF) == 0x10);

	int uncompressedSize = header >> 8;
	
	void* temp = malloc(uncompressedSize);

	swiDecompressLZSSWram((void*)source, temp);

	memcpy(destination, temp, uncompressedSize);
	free(temp);
}

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
