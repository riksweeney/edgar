/*
Copyright (C) 2009-2019 Parallel Realities

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
Foundation, 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
*/

#include "../headers.h"

#include "error.h"

static unsigned char *decompressFile64(char *);

void compressFile(char *sourceName)
{
	int result;
	unsigned char *source, *dest;
	int32_t fileSize, ensuredSize;
	unsigned long compressedSize;
	FILE *fp;

	fp = fopen(sourceName, "rb");

	fseek(fp, 0L, SEEK_END);

	fileSize = ftell(fp);

	ensuredSize = fileSize * 1.01 + 12;

	fseek(fp, 0L, SEEK_SET);

	source = malloc(fileSize * sizeof(unsigned char));

	if (source == NULL)
	{
		showErrorAndExit("Failed to allocate %ld bytes to compress file %s", fileSize * sizeof(unsigned char), sourceName);
	}

	dest = malloc(ensuredSize * sizeof(unsigned char));

	if (dest == NULL)
	{
		showErrorAndExit("Failed to allocate %ld bytes to compress file %s", ensuredSize * sizeof(unsigned char), sourceName);
	}

	compressedSize = ensuredSize;

	fread(source, fileSize, 1, fp);

	fclose(fp);

	result = compress2(dest, &compressedSize, source, fileSize, 9);

	if (result != Z_OK)
	{
		switch (result)
		{
			case Z_BUF_ERROR:
				showErrorAndExit("Compression of file %s failed. Buffer too small", sourceName);
			break;

			case Z_MEM_ERROR:
				showErrorAndExit("Compression of file %s failed. Insufficient memory", sourceName);
			break;

			default:
				showErrorAndExit("Compression of file %s failed. Stream size incorrectly defined", sourceName);
			break;
		}
	}

	fp = fopen(sourceName, "wb");

	fileSize = SWAP32(fileSize);

	result = fwrite(&fileSize, sizeof(int32_t), 1, fp);

	if (result != 1)
	{
		showErrorAndExit("Failed to write original filesize: %s", strerror(errno));
	}

	result = fwrite(dest, compressedSize, 1, fp);

	if (result != 1)
	{
		showErrorAndExit("Failed to write compressed data: %s", strerror(errno));
	}

	free(source);

	free(dest);

	fclose(fp);
}

unsigned char *decompressFile(char *sourceName)
{
	unsigned char *source, *dest;
	int32_t compressedSize, int32;
	unsigned long fileSize;
	FILE *fp;
	int read, result;

	fp = fopen(sourceName, "rb");

	if (fp == NULL)
	{
		showErrorAndExit("Could not open file %s", sourceName);
	}

	fseek(fp, 0L, SEEK_END);

	compressedSize = ftell(fp);

	compressedSize -= sizeof(int32_t);

	fseek(fp, 0L, SEEK_SET);

	read = fread(&int32, sizeof(int32_t), 1, fp);

	fileSize = SWAP32(int32);

	if (read != 1)
	{
		fclose(fp);

		return decompressFile64(sourceName);
	}

	source = malloc(compressedSize * sizeof(unsigned char));

	if (source == NULL)
	{
		fclose(fp);

		return decompressFile64(sourceName);
	}

	dest = malloc((fileSize + 1) * sizeof(unsigned char));

	if (dest == NULL)
	{
		free(source);

		fclose(fp);

		return decompressFile64(sourceName);
	}

	read = fread(source, compressedSize, 1, fp);

	fclose(fp);

	if (read != 1)
	{
		free(source);

		free(dest);

		return decompressFile64(sourceName);
	}

	result = uncompress(dest, &fileSize, source, compressedSize);

	dest[fileSize] = '\0';

	free(source);

	if (result != Z_OK)
	{
		if (dest != NULL)
		{
			free(dest);
		}

		return decompressFile64(sourceName);
	}

	return dest;
}

static unsigned char *decompressFile64(char *sourceName)
{
	unsigned char *source, *dest;
	int64_t long64;
	unsigned long compressedSize, fileSize;
	FILE *fp;
	int read, result;

	printf("%s appears to be a 64 bit file.\nFalling back to old loading routine...\n", sourceName);

	fp = fopen(sourceName, "rb");

	if (fp == NULL)
	{
		showErrorAndExit("Could not open file %s", sourceName);
	}

	fseek(fp, 0L, SEEK_END);

	compressedSize = ftell(fp);

	compressedSize -= sizeof(int64_t);

	fseek(fp, 0L, SEEK_SET);

	read = fread(&long64, sizeof(int64_t), 1, fp);

	fileSize = SWAP64(long64);

	if (read != 1)
	{
		showErrorAndExit("Failed to read original filesize: %s", strerror(errno));
	}

	source = malloc(compressedSize * sizeof(unsigned char));

	if (source == NULL)
	{
		showErrorAndExit("Failed to allocate %ld bytes to decompress file %s", compressedSize * sizeof(unsigned char), sourceName);
	}

	dest = malloc((fileSize + 1) * sizeof(unsigned char));

	if (dest == NULL)
	{
		showErrorAndExit("Failed to allocate %ld bytes to decompress file %s", (fileSize + 1) * sizeof(unsigned char), sourceName);
	}

	read = fread(source, compressedSize, 1, fp);

	if (read != 1)
	{
		showErrorAndExit("Failed to read compressed data: %s", strerror(errno));
	}

	result = uncompress(dest, &fileSize, source, compressedSize);

	dest[fileSize] = '\0';

	fclose(fp);

	free(source);

	if (result != Z_OK)
	{
		switch (result)
		{
			case Z_BUF_ERROR:
				showErrorAndExit("Decompression of file %s failed. Buffer too small", sourceName);
			break;

			case Z_MEM_ERROR:
				showErrorAndExit("Decompression of file %s failed. Insufficient memory", sourceName);
			break;

			default:
				showErrorAndExit("Decompression of file %s failed. Data is corrupt", sourceName);
			break;
		}
	}

	return dest;
}
