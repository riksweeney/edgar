/*
Copyright (C) 2009 Parallel Realities

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "../headers.h"

void compressFile(char *sourceName)
{
	unsigned char *source, *dest;
	uLongf fileSize, compressedSize, ensuredSize;
	FILE *fp;

	fp = fopen(sourceName, "rb");

	fseek(fp, 0L, SEEK_END);

	fileSize = ftell(fp);

	ensuredSize = fileSize * 1.01 + 12;

	fseek(fp, 0L, SEEK_SET);

	source = (unsigned char *)malloc(fileSize * sizeof(unsigned char));

	if (source == NULL)
	{
		printf("Failed to allocate %ld bytes to compress save file\n", fileSize * sizeof(unsigned char));

		exit(1);
	}

	dest = (unsigned char *)malloc(ensuredSize * sizeof(unsigned char));

	if (dest == NULL)
	{
		printf("Failed to allocate %ld bytes to compress save file\n", ensuredSize * sizeof(unsigned char));

		exit(1);
	}

	compressedSize = ensuredSize;

	fread(source, fileSize, 1, fp);

	fclose(fp);

	compress2(dest, &compressedSize, source, fileSize, 9);

	printf("Compressed from %ld to %ld\n", fileSize, compressedSize);

	fp = fopen(sourceName, "wb");

	fwrite(&fileSize, sizeof(uLongf), 1, fp);

	fwrite(dest, compressedSize, 1, fp);

	free(source);

	free(dest);

	fclose(fp);
}

unsigned char *decompressFile(char *sourceName)
{
	unsigned char *source, *dest;
	uLongf compressedSize, fileSize;
	FILE *fp;

	fp = fopen(sourceName, "rb");

	if (fp == NULL)
	{
		printf("Could not open save file %s\n", sourceName);

		exit(1);
	}

	fseek(fp, 0L, SEEK_END);

	compressedSize = ftell(fp);

	compressedSize -= sizeof(uLongf);

	fseek(fp, 0L, SEEK_SET);

	fread(&fileSize, sizeof(uLongf), 1, fp);

	source = (unsigned char *)malloc(compressedSize * sizeof(unsigned char));

	if (source == NULL)
	{
		printf("Failed to allocate %ld bytes to compress save file\n", compressedSize * sizeof(unsigned char));

		exit(1);
	}

	dest = (unsigned char *)malloc((fileSize + 1) * sizeof(unsigned char));

	if (dest == NULL)
	{
		printf("Failed to allocate %ld bytes to compress save file\n", (fileSize + 1) * sizeof(unsigned char));

		exit(1);
	}

	fread(source, compressedSize, 1, fp);

	printf("Decompressing from %ld to %ld\n", compressedSize, fileSize);

	uncompress(dest, &fileSize, source, compressedSize);

	dest[fileSize] = '\0';

	fclose(fp);

	free(source);

	return dest;
}
