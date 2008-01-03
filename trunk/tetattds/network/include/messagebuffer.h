#pragma once

#define BUFFER_SIZE 2048

class MessageBuffer
{
public:
	MessageBuffer();
	~MessageBuffer();

	bool Put(void* data, unsigned int length, void* from);
	bool Get(void* data, unsigned int* length, void** from);
	
private:
	unsigned int Write(unsigned int pos, void* data, unsigned int length);
	unsigned int Read(unsigned int pos, void* data, unsigned int length);
	
	char buffer[BUFFER_SIZE];
	unsigned int readCursor;
	unsigned int writeCursor;
};
