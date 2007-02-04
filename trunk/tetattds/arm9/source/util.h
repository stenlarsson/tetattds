#pragma once

void* Decompress(const void* source, int* size);
void Decompress(void* destination, const void* source);
void PrintSpinner();
void PrintStatus(char* format, ...);
