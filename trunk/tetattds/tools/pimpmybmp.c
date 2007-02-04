/*****
 * pimpmybmp v1.1.2; Copyright (c) 2007 Gustav Munkby
 *
 * Version History:
 *  - v1.0.0 - Initial Ruby implementation
 *  - v1.1.0 - Converted to C
 *             Added support for 16 color bitmaps
 *  - v1.1.1 - Fixed flipped output
 *  - v1.1.2 - Output files in working directory
 *             Fixed palette minimization
 *             Ensured gcc -Wall produces no warnings
 ******************************************************/

/* TODO: Document program parameters */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Copy oldname into newname, exchanging the extension.
 * If the old filename does not have an extension, an
 * extension is added to the end of the old filename.
 * Also removes any path from the old filename.
 */
const char *new_ext(
	char *newname, const char *oldname, const char *ext)
{
	char * s = strrchr(oldname, '/');
	char * b = strrchr(oldname, '\\');
	if (s != NULL || b != NULL)
		strcpy(newname, (b > s) ? b + 1 : s + 1);
	else
		strcpy(newname, oldname);
	char * p = strrchr(newname, '.');
	if (!p || strchr(p, '/') || strchr(p, '\\'))
		strcat(newname, ".");
	else
		p[1] = '\0';
	return strcat(newname, ext);
}

/**
 * Read in the named bitmap, and write out the appropriate files.
 */
unsigned int process_bitmap(
	const char *fname,
	unsigned int tilesize,
	unsigned int metasize,
	unsigned int create_map)
{
	printf("fname: %s\n", fname);
	/* Read bitmap header */
	FILE* f = fopen(fname, "rb");
	if (f == NULL) return 2;
	unsigned char head[54];
	char newname[1024];
	if (fread(head, 1, sizeof(head), f) == 0) return 3;
	if (memcmp("BM",(const char *)head,2) != 0) return 4;
	if (*(unsigned int*)(head+14) != 40) return 5;
	unsigned short depth = *(unsigned short*)(head+28);
	if (depth != 8 && depth != 4) return 6;
	if (*(unsigned int*)(head+30) != 0) return 7;

	/* Read palette */
	unsigned int offset = *(unsigned int*)(head+10);
	unsigned int palette[256];
	unsigned short palsize = offset - ftell(f);
	if (sizeof(palette) < palsize) palsize = sizeof(palette);
	memset(palette, 0, sizeof(palette));
	if (fread(palette, 1, palsize, f) == 0) return 8;

	/* Read bitmap data */
	if (fseek(f, offset, SEEK_SET) < 0) return 9;
	unsigned int width = *(unsigned int*)(head+18),
	             height = *(unsigned int*)(head+22);
	unsigned char *data = calloc(1, height * width + 3);
	if (depth == 8) {
		{int i = height-1; for (; i >= 0; i--) {
			if (fread(data+i*width, 1, ((width+3)/4)*4, f) == 0) return 10;
		}}
	}
	else {
		{int y = height-1; for (; y >= 0; y--) {
			{int x = 0; for (; x < ((width+3)/4)*4; x+=2) {
			  unsigned char t = fgetc(f);
			  data[y*width+x] = t >> 4;
			  data[y*width+x+1] = t & 0xf;
			}}
		}}
	}

	{
  		/* Minimize the palette */
		unsigned char palremap[256];
		{unsigned int i = 0; for (; i < 256; i++) {
			palremap[i] = (unsigned char)i;
		}}
		{int i = 0; for (; i < 256; i++) {
			{int j = i + 1; for (; j < 256; j++) {
				if (palremap[j] == j && palette[i] == palette[j]) {
					palremap[j] = i;
				}
			}}
		}}
		/* TODO: Move duplicates to end of palette */
		{int i = 0; for (; i < height * width; i++) {
			data[i] = palremap[data[i]];
		}}
	}
	
	{
		/* Reorganize the bits to match the tile order */
		unsigned char *data2 = malloc(height * width);
		if (data2 == NULL) return 12;
		memcpy(data2, data, height * width);
		unsigned char *p = data;
		{int ty = 0; for (; ty < height; ty += metasize) {
			{int tx = 0; for (; tx < width; tx += metasize) {
				{int y = ty; for (; y < ty + metasize; y += tilesize) {
					{int x = tx; for (; x < tx + metasize; x += tilesize) {
						{int cy = y; for (; cy < y + tilesize; cy++) {
							{int cx = x; for (; cx < x + tilesize; cx++) {
								*p++ = data2[cy*width+cx];
							}}
						}}
					}}
				}}
			}}
		}}
		free(data2);
	}

	FILE *fRaw = fopen(new_ext(newname, fname, "raw"), "wb");
	printf("Writing %s\n", newname);
	if (fRaw == NULL) return 13;

	if (create_map) {
		/* Create a tile-map */
		FILE *fMap = fopen(new_ext(newname, fname, "map"), "wb");
		printf("Writing %s\n", newname);
		if (fMap == NULL) return 14;
		
		int configs[4][7] = {
			{0x000, 0,           1, 0,           1},
		 	{0x800, tilesize-1, -1, 0,           1},
			{0x400, 0,           1, tilesize-1, -1},
		 	{0xC00, tilesize-1, -1, tilesize-1, -1},
		};
		unsigned short index = 0;
		unsigned int inc = tilesize*tilesize;
		{unsigned char *p = data; for (; p < data+width*height; p += inc) {
			{unsigned char *q = data; for (; q < data+index*inc; q += inc) {
				{int c = 0; for (; c < sizeof(configs)/sizeof(configs[0]); c++) {
					{int ay = 0, by = configs[c][1];
					 for (; ay < tilesize; by+=configs[c][2], ay++) {
						{int ax = 0, bx = configs[c][3];
						 for (; ax < tilesize; bx+=configs[c][4], ax++) {
							if (p[ay*tilesize+ax] != q[by*tilesize+bx]) {
								goto nomatch;
							}
						}}
					}}
					unsigned short map = (unsigned int)(configs[c][0] | ((q-data)/inc));
					fwrite(&map, 1, 2, fMap);					
					goto match;
nomatch:            (void)1; /* BAH */
				}}
			}}
			fwrite(&index, 1, 2, fMap);
			if (data+index*inc != p) {
				memcpy(data+index*inc, p, inc);
			}
			index++;
			fwrite(p, 1, inc, fRaw);
match:      (void)1; /* BAH */
		}}
		
		if (fclose(fMap) != 0) return 15;
	}
	else {
		/* Write raw directly */
		fwrite(data, 1, width*height, fRaw);
	}
	if (fclose(fRaw) != 0) return 16;
	
	/* Convert the palette from bmp-format to gba-format */
	FILE *fPal = fopen(new_ext(newname, fname, "pal"), "wb");
	printf("Writing %s\n", newname);
	{int i = 0; for (; i < 256; i++) {
		unsigned int c = palette[i];
		unsigned short d = (c&0xF80000)>>19 | (c&0xF800)>>6 | (c&0xF8)<<7;
		if (fwrite(&d, 1, 2, fPal) == 0) return 17;
	}}
	if (fclose(fPal) != 0) return 18;
	
	return 0;
}

int main (int argc, char const* argv[])
{
	printf("pimpmybmp v1.1.2; Copyright (c) 2007 Gustav Munkby\n");
	unsigned int tilesize = 8, metasize = -1, map = 0;
	{int i = 1; for (; i < argc; i++) {
		if (argv[i][0] == '-') {
			char dummy;
			if (sscanf(argv[i],"-t%u%1s", &tilesize, &dummy) == 1) {}
			else if (sscanf(argv[i],"-T%u%1s", &metasize, &dummy) == 1) {}
			else if (strcmp(argv[i], "-m") == 0) {
				map = 1;
			}
			else {
				printf("Illegal option '%s'\n", argv[i]);
				return 1;
			}
		}
	}}
	if (metasize == -1) metasize = tilesize;
	if (metasize < tilesize) {
		printf("Metatile size (%u) must be larger than tile size (%u)\n", metasize, tilesize);
		return 1;
	}
	unsigned int result = 0;
	{int i = 1; for (; i < argc; i++) {
		if (argv[i][0] != '-') {
			unsigned int err = process_bitmap(argv[i], tilesize, metasize, map);
			if (err != 0) {
				printf("Processing of '%s' failed (%d)\n", argv[i], err);
				result = 2;
			}
		}
	}}
	return result;
}
