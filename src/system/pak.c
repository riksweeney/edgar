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
#include "pak.h"
#include "load_save.h"

static unsigned char *uncompressFile(char *, int);
static SDL_RWops *uncompressFileRW(char *);

static FileData *fileData;
static char pakFile[MAX_PATH_LENGTH];
static int fileCount;

void initPakFile()
{
	#if DEV == 0
		int i;
		long offset;
		FILE *fp;

		snprintf(pakFile, sizeof(pakFile), "%s%s", INSTALL_PATH, PAK_FILE);

		fp = fopen(pakFile, "rb");

		if (fp == NULL)
		{
			printf("Failed to open PAK file %s\n", pakFile);

			exit(1);
		}

		fseek(fp, -(sizeof(long) + sizeof(int)), SEEK_END);

		fread(&offset, sizeof(long), 1, fp);
		fread(&fileCount, sizeof(int), 1, fp);
		
		offset = SWAP32(offset);
		fileCount = SWAP32(fileCount);

		fileData = (FileData *)malloc(fileCount * sizeof(FileData));

		if (fileData == NULL)
		{
			printf("Could not allocate %d bytes for FileData\n", fileCount * sizeof(FileData));

			exit(1);
		}

		fseek(fp, offset, SEEK_SET);

		fread(fileData, sizeof(FileData), fileCount, fp);

		printf("Loaded up PAK file with %d entries\n", fileCount);
		
		for (i=0;i<fileCount;i++)
		{
			fileData[i].offset = SWAP32(fileData[i].offset);
			fileData[i].compressedSize = SWAP32(fileData[i].compressedSize);
			fileData[i].fileSize = SWAP32(fileData[i].fileSize);
		}

		fclose(fp);
	#else
		pakFile[0] = '\0';
		fileCount = 0;
		fileData = NULL;
	#endif
}

SDL_Surface *loadImageFromPak(char *name)
{
	SDL_RWops *rw;
	SDL_Surface *surface;

	rw = uncompressFileRW(name);

	surface = IMG_Load_RW(rw, TRUE);

	return surface;
}

Mix_Chunk *loadSoundFromPak(char *name)
{
	SDL_RWops *rw;
	Mix_Chunk *chunk;

	rw = uncompressFileRW(name);

	chunk = Mix_LoadWAV_RW(rw, TRUE);

	return chunk;
}

unsigned char *loadFileFromPak(char *name)
{
	return uncompressFile(name, FALSE);
}

Mix_Music *loadMusicFromPak(char *name)
{
	Mix_Music *music;

	#if DEV == 1
		FILE *fp;

		fp = fopen(name, "rb");

		if (fp == NULL)
		{
			return NULL;
		}

		fclose(fp);

		music = Mix_LoadMUS(name);

		return music;
	#else
		unsigned char *file;
		char temp[MAX_PATH_LENGTH];

		printf("Uncompressing %s\n", name);

		file = uncompressFile(name, TRUE);

		printf("Uncompressed to %s\n", file);

		STRNCPY(temp, (char *)file, sizeof(temp));

		printf("Loading '%s'\n", temp);

		music = Mix_LoadMUS(temp);

		if (music == NULL)
		{
			printf("Couldn't load %s\n", temp);
		}

		return music;
	#endif
}

TTF_Font *loadFontFromPak(char *name, int size)
{
	SDL_RWops *rw;
	TTF_Font *font;

	rw = uncompressFileRW(name);

	font = TTF_OpenFontRW(rw, TRUE, size);

	return font;
}

static SDL_RWops *uncompressFileRW(char *name)
{
	int i, index;
	unsigned long size;
	unsigned char *source, *dest;
	FILE *fp;
	SDL_RWops *rw;

	index = i = -1;

	#if DEV == 1
		fp = fopen(name, "rb");

		if (fp == NULL)
		{
			printf("Failed to open %s\n", name);

			exit(1);
		}

		fseek(fp, 0L, SEEK_END);

		size = ftell(fp);

		fseek(fp, 0L, SEEK_SET);

		dest = (unsigned char *)malloc(size * sizeof(unsigned char));

		if (dest == NULL)
		{
			printf("Failed to allocate %ld bytes to load %s\n", size * sizeof(unsigned char), name);

			exit(1);
		}

		fread(dest, size, 1, fp);

		source = NULL;
	#else
		index = -1;

		for (i=0;i<fileCount;i++)
		{
			if (strcmpignorecase(fileData[i].filename, name) == 0)
			{
				index = i;

				break;
			}
		}

		if (index == -1)
		{
			printf("Failed to find %s in PAK file\n", name);

			exit(1);
		}

		fp = fopen(pakFile, "rb");

		if (fp == NULL)
		{
			printf("Failed to open PAK file %s\n", pakFile);

			exit(1);
		}

		fseek(fp, fileData[index].offset, SEEK_SET);

		source = (unsigned char *)malloc(fileData[index].compressedSize * sizeof(unsigned char));

		if (source == NULL)
		{
			printf("Failed to allocate %ld bytes to load %s from PAK\n", fileData[index].compressedSize * sizeof(unsigned char), name);

			exit(1);
		}

		dest = (unsigned char *)malloc(fileData[index].fileSize * sizeof(unsigned char));

		if (dest == NULL)
		{
			printf("Failed to allocate %ld bytes to load %s from PAK\n", fileData[index].fileSize * sizeof(unsigned char), name);

			exit(1);
		}

		fread(source, fileData[i].compressedSize, 1, fp);

		size = fileData[index].fileSize;

		uncompress(dest, &size, source, fileData[index].compressedSize);

		if (size != fileData[index].fileSize)
		{
			printf("Failed to decompress %s. Expected %ld, got %ld\n", fileData[index].filename, fileData[index].fileSize, size);

			exit(1);
		}
	#endif

	if (source != NULL)
	{
		free(source);
	}

	fclose(fp);

	rw = SDL_RWFromMem(dest, size);

	return rw;
}

static unsigned char *uncompressFile(char *name, int writeToFile)
{
	int i, index;
	unsigned long size;
	unsigned char *source, *dest, filename[MAX_PATH_LENGTH];
	FILE *fp;

	index = i = -1;

	#if DEV == 1
		fp = fopen(name, "rb");

		if (fp == NULL)
		{
			printf("Failed to open %s\n", name);

			exit(1);
		}

		fseek(fp, 0L, SEEK_END);

		size = ftell(fp);

		fseek(fp, 0L, SEEK_SET);

		dest = (unsigned char *)malloc((size + 2) * sizeof(unsigned char));

		if (dest == NULL)
		{
			printf("Failed to allocate %ld bytes to load %s\n", (size + 2) * sizeof(unsigned char), name);

			exit(1);
		}

		fread(dest, size, 1, fp);

		dest[size] = '\n';
		dest[size + 1] = '\0';

		source = NULL;
	#else
		index = -1;

		for (i=0;i<fileCount;i++)
		{
			if (strcmpignorecase(fileData[i].filename, name) == 0)
			{
				index = i;

				break;
			}
		}

		if (index == -1)
		{
			printf("Failed to find %s in PAK file\n", name);

			exit(1);
		}

		fp = fopen(pakFile, "rb");

		if (fp == NULL)
		{
			printf("Failed to open PAK file %s\n", pakFile);

			exit(1);
		}

		fseek(fp, fileData[index].offset, SEEK_SET);

		source = (unsigned char *)malloc(fileData[index].compressedSize * sizeof(unsigned char));

		if (source == NULL)
		{
			printf("Failed to allocate %ld bytes to load %s from PAK\n", fileData[index].compressedSize * sizeof(unsigned char), name);

			exit(1);
		}

		dest = (unsigned char *)malloc((fileData[index].fileSize + 1) * sizeof(unsigned char));

		if (dest == NULL)
		{
			printf("Failed to allocate %ld bytes to load %s from PAK\n", fileData[index].fileSize * sizeof(unsigned char), name);

			exit(1);
		}

		fread(source, fileData[index].compressedSize, 1, fp);

		size = fileData[index].fileSize;

		uncompress(dest, &size, source, fileData[index].compressedSize);

		dest[size] = '\0';

		if (size != fileData[index].fileSize)
		{
			printf("Failed to decompress %s. Expected %ld, got %ld\n", fileData[index].filename, fileData[index].fileSize, size);

			exit(1);
		}

		if (writeToFile == TRUE)
		{
			printf("Writing to %s\n", getGameSavePath());

			fclose(fp);

			snprintf((char *)filename, sizeof(filename), "%spakdata", getGameSavePath());

			printf("Writing %s to %s\n", filename, getGameSavePath());

			fp = fopen((char *)filename, "wb");

			if (fp == NULL)
			{
				perror("Failed to write pak data to temp file");

				exit(1);
			}

			printf("Writing data\n");

			fwrite(dest, size, 1, fp);

			printf("Wrote %s to %s\n", name, filename);

			free(dest);
		}
	#endif

	if (source != NULL)
	{
		free(source);
	}

	fclose(fp);

	return (writeToFile == TRUE ? filename : dest);
}

int existsInPak(char *name)
{
	int i, exists;
	FILE *fp;

	fp = NULL;
	exists = FALSE;
	i = 0;

	#if DEV == 1
		fp = fopen(name, "rb");

		if (fp == NULL)
		{
			exists = FALSE;
		}

		else
		{
			fclose(fp);

			exists = TRUE;
		}
	#else
		for (i=0;i<fileCount;i++)
		{
			if (strcmpignorecase(fileData[i].filename, name) == 0)
			{
				exists = TRUE;

				break;
			}
		}
	#endif

	return exists;
}

void freePakFile()
{
	#if DEV == 0
		if (fileData != NULL)
		{
			free(fileData);
		}
	#endif
}
