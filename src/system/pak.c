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
#include "pak.h"
#include "error.h"
#include "load_save.h"

static unsigned char *uncompressFile(char *, int);
static unsigned char *uncompressFileRW(char *, unsigned long *);

static FileData *fileData;
static char pakFile[MAX_PATH_LENGTH];
static int32_t fileCount;

void initPakFile()
{
	#if defined(PAK_FILE) && DEV == 0
		int32_t offset;
		FILE *fp;
		int i;

		snprintf(pakFile, sizeof(pakFile), "%s%s", INSTALL_PATH, PAK_FILE);

		fp = fopen(pakFile, "rb");

		if (fp == NULL)
		{
			printf(_("Failed to open PAK file %s"), pakFile);
			printf("\n");
			printf("%s", _("If you compiled the game from source, you need to do a make install"));
			printf("\n");

			exit(0);
		}

		fseek(fp, -(sizeof(int32_t) + sizeof(int32_t)), SEEK_END);

		fread(&offset, sizeof(int32_t), 1, fp);
		fread(&fileCount, sizeof(int32_t), 1, fp);

		offset = SWAP32(offset);
		fileCount = SWAP32(fileCount);

		fileData = malloc(fileCount * sizeof(FileData));

		if (fileData == NULL)
		{
			showErrorAndExit("Failed to allocate %d bytes for FileData", fileCount * (int)sizeof(FileData));
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

void verifyVersion()
{
        #if defined(PAK_FILE) && DEV == 0
            char version[5];

            snprintf(version, sizeof(version), "%0.2f", VERSION);

			if (existsInPak(version) == FALSE)
			{
				showErrorAndExit("Game and PAK file versions do not match. Please reinstall the game.");
			}
        #endif
}

SDL_Surface *loadImageFromPak(char *name)
{
	unsigned long size;
	unsigned char *buffer;
	SDL_RWops *rw;
	SDL_Surface *surface;

	buffer = uncompressFileRW(name, &size);

	rw = SDL_RWFromMem(buffer, size);

	surface = IMG_Load_RW(rw, TRUE);
	
	if (surface == NULL)
	{
		showErrorAndExit("Failed to load image %s", name);
	}

	free(buffer);

	return surface;
}

Mix_Chunk *loadSoundFromPak(char *name)
{
	unsigned long size;
	unsigned char *buffer;
	SDL_RWops *rw;
	Mix_Chunk *chunk;

	if (existsInPak(name) == FALSE)
	{
		return NULL;
	}

	buffer = uncompressFileRW(name, &size);

	rw = SDL_RWFromMem(buffer, size);

	chunk = Mix_LoadWAV_RW(rw, TRUE);

	free(buffer);

	return chunk;
}

TTF_Font *loadFontFromPak(char *name, int fontSize)
{
	unsigned long size;
	unsigned char *buffer;
	SDL_RWops *rw;
	TTF_Font *font;

	buffer = uncompressFileRW(name, &size);

	rw = SDL_RWFromMem(buffer, size);

	font = TTF_OpenFontRW(rw, TRUE, fontSize);

	/*free(buffer);*/

	return font;
}

unsigned char *loadFileFromPak(char *name)
{
	return uncompressFile(name, FALSE);
}

Mix_Music *loadMusicFromPak(char *name)
{
	Mix_Music *music;

	#if !defined(PAK_FILE) || DEV == 1
		char fullName[MAX_PATH_LENGTH];
		FILE *fp;

		snprintf(fullName, sizeof(fullName), "%s%s", INSTALL_PATH, name);

		fp = fopen(fullName, "rb");

		if (fp == NULL)
		{
			return NULL;
		}

		fclose(fp);

		music = Mix_LoadMUS(fullName);

		return music;
	#else
		unsigned char *file;
		char temp[MAX_PATH_LENGTH];

		if (existsInPak(name) == FALSE)
		{
			return NULL;
		}

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

		free(file);

		return music;
	#endif
}

static unsigned char *uncompressFileRW(char *name, unsigned long *size)
{
	unsigned char *source, *dest;
	FILE *fp;
	#if defined(PAK_FILE) && DEV == 0
		int i, index;
	#endif

	#if !defined(PAK_FILE) || DEV == 1
		char fullName[MAX_PATH_LENGTH];
		snprintf(fullName, sizeof(fullName), "%s%s", INSTALL_PATH, name);

		fp = fopen(fullName, "rb");

		if (fp == NULL)
		{
			showErrorAndExit("Failed to open %s", name);
		}

		fseek(fp, 0L, SEEK_END);

		(*size) = ftell(fp);

		fseek(fp, 0L, SEEK_SET);

		dest = malloc((*size) * sizeof(unsigned char));

		if (dest == NULL)
		{
			showErrorAndExit("Failed to allocate %ld bytes to load %s", (*size) * (int)sizeof(unsigned char), name);
		}

		fread(dest, (*size), 1, fp);

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
			showErrorAndExit("Failed to find %s in PAK file", name);
		}

		fp = fopen(pakFile, "rb");

		if (fp == NULL)
		{
			showErrorAndExit("Failed to open PAK file %s", pakFile);
		}

		fseek(fp, fileData[index].offset, SEEK_SET);

		source = malloc(fileData[index].compressedSize * sizeof(unsigned char));

		if (source == NULL)
		{
			showErrorAndExit("Failed to allocate %d bytes to load %s from PAK", fileData[index].compressedSize * (int)sizeof(unsigned char), name);
		}

		dest = malloc(fileData[index].fileSize * sizeof(unsigned char));

		if (dest == NULL)
		{
			showErrorAndExit("Failed to allocate %d bytes to load %s from PAK", fileData[index].fileSize * (int)sizeof(unsigned char), name);
		}

		fread(source, fileData[i].compressedSize, 1, fp);

		(*size) = fileData[index].fileSize;

		uncompress(dest, size, source, fileData[index].compressedSize);

		if ((*size) != fileData[index].fileSize)
		{
			showErrorAndExit("Failed to decompress %s. Expected %ld, got %ld", fileData[index].filename, fileData[index].fileSize, (*size));
		}
	#endif

	if (source != NULL)
	{
		free(source);
	}

	fclose(fp);

	return dest;
}

static unsigned char *uncompressFile(char *name, int writeToFile)
{
	#if !defined(PAK_FILE) || DEV == 1
		char fullName[MAX_PATH_LENGTH];
	#else
		int i;
		int index = -1;
	#endif
	char *filename;
	unsigned long size;
	unsigned char *source, *dest;
	FILE *fp;

	filename = NULL;

	filename = malloc(MAX_PATH_LENGTH);

	if (filename == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for filename", MAX_PATH_LENGTH);
	}

	#if !defined(PAK_FILE) || DEV == 1
		snprintf(fullName, sizeof(fullName), "%s%s", INSTALL_PATH, name);

		fp = fopen(fullName, "rb");

		if (fp == NULL)
		{
			showErrorAndExit("Failed to open %s", name);
		}

		fseek(fp, 0L, SEEK_END);

		size = ftell(fp);

		fseek(fp, 0L, SEEK_SET);

		dest = malloc((size + 2) * sizeof(unsigned char));

		if (dest == NULL)
		{
			showErrorAndExit("Failed to allocate %ld bytes to load %s", (size + 2) * (int)sizeof(unsigned char), name);
		}

		fread(dest, size, 1, fp);

		dest[size] = '\n';
		dest[size + 1] = '\0';

		source = NULL;
	#else
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
			showErrorAndExit("Failed to find %s in PAK file", name);
		}

		fp = fopen(pakFile, "rb");

		if (fp == NULL)
		{
			showErrorAndExit("Failed to open PAK file %s", pakFile);
		}

		fseek(fp, fileData[index].offset, SEEK_SET);

		source = malloc(fileData[index].compressedSize * sizeof(unsigned char));

		if (source == NULL)
		{
			showErrorAndExit("Failed to allocate %ld bytes to load %s from PAK", fileData[index].compressedSize * (int)sizeof(unsigned char), name);
		}

		dest = malloc((fileData[index].fileSize + 1) * sizeof(unsigned char));

		if (dest == NULL)
		{
			showErrorAndExit("Failed to allocate %ld bytes to load %s from PAK", fileData[index].fileSize * (int)sizeof(unsigned char), name);
		}

		fread(source, fileData[index].compressedSize, 1, fp);

		size = fileData[index].fileSize;

		uncompress(dest, &size, source, fileData[index].compressedSize);

		dest[size] = '\0';

		if (size != fileData[index].fileSize)
		{
			showErrorAndExit("Failed to decompress %s. Expected %ld, got %ld", fileData[index].filename, fileData[index].fileSize, size);
		}

		if (writeToFile == TRUE)
		{
			fclose(fp);

			snprintf(filename, MAX_PATH_LENGTH, "%spakdata", getGameSavePath());

			printf("Writing to %s\n", filename);

			fp = fopen(filename, "wb");

			if (fp == NULL)
			{
				showErrorAndExit("Failed to write pak data to temp file: %s", strerror(errno));
			}

			fwrite(dest, size, 1, fp);

			free(dest);
		}
	#endif

	if (source != NULL)
	{
		free(source);
	}

	if (filename != NULL && writeToFile == FALSE)
	{
		free(filename);
	}

	fclose(fp);

	if (writeToFile == TRUE)
	{
		return (unsigned char *)filename;
	}

	else
	{
		return dest;
	}
}

int existsInPak(char *name)
{
	int exists;

	#if !defined(PAK_FILE) || DEV == 1
		char fullName[MAX_PATH_LENGTH];
		FILE *fp;

		snprintf(fullName, sizeof(fullName), "%s%s", INSTALL_PATH, name);

		fp = fopen(fullName, "rb");


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
		int i;
		exists = FALSE;

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
	#if defined(PAK_FILE) && DEV == 0
		if (fileData != NULL)
		{
			free(fileData);
		}
	#endif
}
