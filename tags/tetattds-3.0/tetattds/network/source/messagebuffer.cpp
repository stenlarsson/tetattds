#include "messagebuffer.h"
#include <string.h>
#include <stdio.h>

#define min(x, y) ((x) < (y) ? (x) : (y))

// BufferHeader is used to store how long the message is
// This is needed when we want to read
struct BufferHeader
{
	BufferHeader()
	{
	}

	BufferHeader(void* from, unsigned int length)
	:	from(from),
		length(length)
	{
	}
		
	void* from;
	unsigned int length;
};

MessageBuffer::MessageBuffer()
:	readCursor(0),
	writeCursor(0)
{
	memset(buffer, 0, BUFFER_SIZE);
}

MessageBuffer::~MessageBuffer()
{
}

// Put message in buffer
// Returns false if buffer was full
bool MessageBuffer::Put(void* data, unsigned int length, void* from)
{
	// get free space
	int free = readCursor - writeCursor;
	if(free <= 0) {
		free += BUFFER_SIZE;
	}

	// check that it's enough
	// >= since otherwise we cannot distinguish between empty or full
	if(length + sizeof(BufferHeader) >= (unsigned int)free) {
		return false;
	}
	
	BufferHeader header(from, length);
	
	// write header
	unsigned int pos = Write(writeCursor, &header, sizeof(BufferHeader));
	// write data
	writeCursor = Write(pos, data, length);

	return true;
}

// Get message from buffer
// Returns false if there was no message to get
bool MessageBuffer::Get(void* data, unsigned int* length, void** from)
{
	// check that there is data available
	if(readCursor == writeCursor) {
		return false;
	}
	
	BufferHeader header;

	// read header
	unsigned int pos = Read(readCursor, &header, sizeof(BufferHeader));
	// read data
	readCursor = Read(pos, data, header.length);

	*length = header.length;
	*from = header.from;
	return true;
}

// Write length data at pos
// Returns next pos in buffer
// Write does not use writeCursor directly since we do not want
// to update writeCursor until all writes are complete
unsigned int MessageBuffer::Write(unsigned int pos, void* data, unsigned int length)
{
	// write up to end of buffer
	unsigned int firstLength = min(length, BUFFER_SIZE - pos);
	memcpy(buffer + pos, data, firstLength);
	// write rest (might be zero length)
	memcpy(buffer, (char*)data + firstLength, length - firstLength);

	return (pos + length) % BUFFER_SIZE;
}

// Read length data at pos
// Returns next pos in buffer
// Read does not use readCursor directly since we do not want
// to update readCursor until all reads are complete
unsigned int MessageBuffer::Read(unsigned int pos, void* data, unsigned int length)
{
	// read up to end of buffer
	unsigned int firstLength = min(length, BUFFER_SIZE - pos);
	memcpy(data, buffer + pos, firstLength);
	// read rest (might be zero length)
	memcpy((char*)data + firstLength, buffer, length - firstLength);

	return (pos + length) % BUFFER_SIZE;
}
