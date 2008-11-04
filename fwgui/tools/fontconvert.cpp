#include <ft2build.h>
#include <string.h>
#include <stdio.h>
#include <xlocale.h>
#include <libgen.h>
#include <stdlib.h>
#include FT_FREETYPE_H
#include <iostream>

#include "fontfile.h"
using namespace FwGui;

using namespace std;

#define START_CHAR 32
#define END_CHAR 126

#define fix_endian(x) (fixendian ? swap_endian(x) : (x))

template<typename T>
T swap_endian(T x) {
	T y;
	char* a = (char*)&x;
	char* b = (char*)&y;
	b[0] = a[3];
	b[1] = a[2];
	b[2] = a[1];
	b[3] = a[0];
	return y;
}

void usage(char* base)
{
	cout << "Usage: " << base << " [options] input.ttf output.bin" << endl;
	cout << "Options:" << endl;
	cout << "\t--size px     Fontsize. (default 12)" << endl;
	cout << "\t--start char  First character to encode. (default 32)" << endl;
	cout << "\t--end char    Last character to encode. (default 126)" << endl;
	cout << "\t--auto        Force use of autohinter." << endl;
	cout << "\t--fix-endian  Store with other endian." << endl;
}

int main(int argc, char *argv[])
{
	cout << "fontconvert v1.0 by Sten Larsson" << endl;

	char* base = basename(argv[0]);

	int size = 0;
	int startChar = 32;
	int endChar = 126;
	bool autohinter = false;
	bool fixendian = false;

	int arg = 1;
	while(arg < argc && argv[arg][0] == '-')
	{
		if(strcmp(argv[arg], "--help") == 0)
		{
			usage(base);
			return 0;
		}
		else if(strcmp(argv[arg], "--size") == 0)
		{
			if(arg + 1 >= argc)
			{
				usage(base);
				return 1;
			}
			size = atoi(argv[arg+1]);
			arg++;
		}
		else if(strcmp(argv[arg], "--start") == 0)
		{
			if(arg + 1 >= argc)
			{
				usage(base);
				return 1;
			}
			startChar = atoi(argv[arg+1]);
			arg++;
		}
		else if(strcmp(argv[arg], "--end") == 0)
		{
			if(arg + 1 >= argc)
			{
				usage(base);
				return 1;
			}
			endChar = atoi(argv[arg+1]);
			arg++;
		}
		else if(strcmp(argv[arg], "--auto") == 0)
		{
			autohinter = true;
		}
		else if(strcmp(argv[arg], "--fix-endian") == 0)
		{
			fixendian = true;
		}

		arg++;
	}

	if(arg + 2 != argc)
	{
		cerr << "Files not specified." << endl;
		return 1;
	}
	char* infilename = argv[arg];
	char* outfilename = argv[arg+1];

	if(size == 0)
	{
		cerr << "Font size not specified or invalid." << endl;
		return 1;
	}

	FT_Library library;
	int error;
	error = FT_Init_FreeType(&library);
	if(error)
	{
		cerr << "Failed to initialize FreeType." << endl;
		return 1;
	}

	FT_Face face;
	error = FT_New_Face(
		library,
		infilename,
		0,
		&face);
	if(error == FT_Err_Unknown_File_Format)
	{
		cerr << "Unsupported input font." << endl;
		return 1;
	}
	else if(error)
	{
		cerr << "Failed to load input font." << endl;
		return 1;
	}

	error = FT_Set_Pixel_Sizes(
		face,           // handle to face object
		0,              // pixel_width
		size); // pixel_height
	if(error)
	{
		cerr << "Failed to set pixel size." << endl;
		return 1;
	}

	// don't know how much we need...
	unsigned char* data = new unsigned char[10*1024*1024];
	FontFileHeader* header = (FontFileHeader*)data;
	Glyph* glyphs = (Glyph*)(data + sizeof(FontFileHeader));
	unsigned char* dataEndPtr = data + sizeof(FontFileHeader) + sizeof(Glyph) * (endChar - startChar + 1);

	FT_Int32 flags = FT_LOAD_RENDER;
	if(autohinter)
	{
		flags |= FT_LOAD_FORCE_AUTOHINT;
	}
	
	Glyph* currentGlyph = glyphs;
	for(int i = startChar; i <= endChar; i++)
	{
		error = FT_Load_Char(face, i, flags);
		if(error)
		{
			cerr << "Failed to render character " << i << "." << endl;
			return 1;
		}

		currentGlyph->width = fix_endian(face->glyph->bitmap.width);
		currentGlyph->height = fix_endian(face->glyph->bitmap.rows);
		currentGlyph->left = fix_endian(face->glyph->bitmap_left);
		currentGlyph->top = fix_endian(-face->glyph->bitmap_top);
		currentGlyph->advancex = fix_endian(face->glyph->advance.x >> 6);
		currentGlyph->advancey = fix_endian(face->glyph->advance.y >> 6);
		currentGlyph->offset = fix_endian((unsigned int)(dataEndPtr - data));
		unsigned char* destptr = dataEndPtr;
		for(int y = 0; y < face->glyph->bitmap.rows; y++)
		{
			unsigned char* srcptr =
				face->glyph->bitmap.buffer +
				y * face->glyph->bitmap.pitch;

			for(int x = 0; x < face->glyph->bitmap.width; x++)
			{
				*destptr++ = *srcptr++;
			}
		}

		currentGlyph++;
		dataEndPtr = destptr;
	}

	// set up header
	header->type = fix_endian(FONT_SIGNATURE);
	header->filesize = fix_endian((unsigned int)(dataEndPtr - data));
	header->startChar = fix_endian(startChar);
	header->endChar = fix_endian(endChar);
	header->height = fix_endian(face->size->metrics.height >> 6);

	FILE* rawFile = fopen(outfilename, "wb");
	if(rawFile == NULL)
	{
		cerr << "Failed to open output file " << outfilename << "." << endl;
		perror(base);
		return 1;
	}

	int count = fwrite(data, (unsigned int)(dataEndPtr - data), 1, rawFile);
	if(count != 1)
	{
		cerr << "Failed to write font data." << endl;
		cerr << base << ": " << strerror(ferror(rawFile)) << endl;
		return 1;
	}

	fclose(rawFile);

	cout << outfilename << " saved successfully." << endl;

	return 0;
}
