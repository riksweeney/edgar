/*
Copyright (C) 2009-2010 Parallel Realities

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

#include "error.h"

void compressFile(char *sourceName)
{
	int read;
	unsigned char *source, *dest;
	unsigned long fileSize, compressedSize, ensuredSize;
	FILE *fp;

	fp = fopen(sourceName, "rb");

	fseek(fp, 0L, SEEK_END);

	fileSize = ftell(fp);

	ensuredSize = fileSize * 1.01 + 12;

	fseek(fp, 0L, SEEK_SET);

	source = (unsigned char *)malloc(fileSize * sizeof(unsigned char));

	if (source == NULL)
	{
		showErrorAndExit("Failed to allocate %ld bytes to compress save file", fileSize * sizeof(unsigned char));
	}

	dest = (unsigned char *)malloc(ensuredSize * sizeof(unsigned char));

	if (dest == NULL)
	{
		showErrorAndExit("Failed to allocate %ld bytes to compress save file", ensuredSize * sizeof(unsigned char));
	}

	compressedSize = ensuredSize;

	read = fread(source, fileSize, 1, fp);

	fclose(fp);

	compress2(dest, &compressedSize, source, fileSize, 9);

	fp = fopen(sourceName, "wb");

	fwrite(&fileSize, sizeof(unsigned long), 1, fp);

	fwrite(dest, compressedSize, 1, fp);

	free(source);

	free(dest);

	fclose(fp);
}

unsigned char *decompressFile(char *sourceName)
{
	unsigned char *source, *dest;
	unsigned long compressedSize, fileSize;
	FILE *fp;
	int read;

	fp = fopen(sourceName, "rb");

	if (fp == NULL)
	{
		showErrorAndExit("Could not open save file %s", sourceName);
	}

	fseek(fp, 0L, SEEK_END);

	compressedSize = ftell(fp);

	compressedSize -= sizeof(unsigned long);

	fseek(fp, 0L, SEEK_SET);

	read = fread(&fileSize, sizeof(unsigned long), 1, fp);

	source = (unsigned char *)malloc(compressedSize * sizeof(unsigned char));

	if (source == NULL)
	{
		showErrorAndExit("Failed to allocate %ld bytes to compress save file", compressedSize * sizeof(unsigned char));
	}

	dest = (unsigned char *)malloc((fileSize + 1) * sizeof(unsigned char));

	if (dest == NULL)
	{
		showErrorAndExit("Failed to allocate %ld bytes to compress save file", (fileSize + 1) * sizeof(unsigned char));
	}

	read = fread(source, compressedSize, 1, fp);

	uncompress(dest, &fileSize, source, compressedSize);

	dest[fileSize] = '\0';

	fclose(fp);

	free(source);

	return dest;
}
