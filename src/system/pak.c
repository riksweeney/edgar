#include "../headers.h"

static unsigned char *uncompressFile(char *);
static SDL_RWops *uncompressFileRW(char *);
static unsigned char *readFile(char *);

static FileData *fileData;
static char pakFile[MAX_PATH_LENGTH];
static int fileCount;

void initPakFile()
{
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
	
	fileData = (FileData *)malloc(fileCount * sizeof(FileData));
	
	if (fileData == NULL)
	{
		printf("Could not allocate %d bytes for FileData\n", fileCount * sizeof(FileData));
		
		exit(1);
	}
	
	fseek(fp, offset, SEEK_SET);
	
	fread(fileData, sizeof(FileData), fileCount, fp);
	
	printf("Loaded up PAK file with %d entries\n", fileCount);
	
	fclose(fp);
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
	return readFile(name);
	
	if (0 == 1)
	{
		return uncompressFile(name);
	}
}

Mix_Music *loadMusicFromPak(char *name)
{
	SDL_RWops *rw;
	Mix_Music *music;
	
	rw = uncompressFileRW(name);
	
	music = Mix_LoadMUS_RW(rw);
	
	SDL_FreeRW(rw);
	
	return music;
}

static SDL_RWops *uncompressFileRW(char *name)
{
	int i, index;
	unsigned long size;
	unsigned char *source, *dest;
	FILE *fp;
	SDL_RWops *rw;
	
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
	
	free(source);
	
	fclose(fp);
	
	rw = SDL_RWFromMem(dest, fileData[index].fileSize);
	
	return rw;
}

static unsigned char *readFile(char *name)
{
	long length;
	FILE *fp;
	unsigned char *buffer;
	
	fp = fopen(name, "rb");
	
	if (fp == NULL)
	{
		printf("Failed to open %s\n", name);
		
		exit(1);
	}
	
	fseek(fp, 0L, SEEK_END);

	length = ftell(fp);
	
	buffer = (unsigned char *)malloc(length * sizeof(unsigned char));
	
	if (buffer == NULL)
	{
		printf("Failed to allocate %ld bytes to load %s\n", length, name);
	}
	
	fseek(fp, 0L, SEEK_SET);
	
	fread(buffer, length, 1, fp);
	
	buffer[length - 1] = '\0';
	
	fclose(fp);
	
	return buffer;
}

static unsigned char *uncompressFile(char *name)
{
	int i, index;
	unsigned long size;
	unsigned char *source, *dest;
	FILE *fp;
	
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
	
	free(source);
	
	fclose(fp);
	
	dest[size - 1] = '\0';
	
	return dest;
}

void freePakFile()
{
	if (fileData != NULL)
	{
		free(fileData);
	}
}
