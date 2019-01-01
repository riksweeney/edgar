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

#include "headers.h"

FILE *pak;
int fileID;
int32_t totalFiles;
FileData *fileData;
char **filenames;

static void getFilenames(char *, int *);
static void cleanup(void);
static void compressFile(char *);
static void testPak(char *);
static int compare (const void *, const void *);

int main(int argc, char *argv[])
{
	int i, arraySize;
	int32_t length;
	char versionName[5];
	FILE *versionFile;

	if (argc == 3)
	{
		if (strcmpignorecase(argv[1], "-test") == 0)
		{
			testPak(argv[2]);

			exit(0);
		}
	}

	else if (argc < 3)
	{
		printf("Usage   : pak <directory names> <outputname>\n");
		printf("Example : pak data music gfx sound font edgar.pak\n");

		exit(1);
	}
	
	atexit(cleanup);

	pak = fopen(argv[argc - 1], "wb");

	arraySize = 1000;
	
	filenames = malloc(arraySize * sizeof(char *));

	for (i=1;i<argc-1;i++)
	{
		getFilenames(argv[i], &arraySize);
	}
	
	qsort(filenames, totalFiles, sizeof(char *), compare);

	totalFiles++;

	printf("Will compress %d files\n", totalFiles);

	printf("Compressing 00%%...\r");

	fileData = malloc(totalFiles * sizeof(FileData));

	if (fileData == NULL)
	{
		printf("Failed to create File Data\n");

		exit(1);
	}

	snprintf(versionName, sizeof(versionName), "%0.2f", VERSION);

	versionFile = fopen(versionName, "wb");

	fprintf(versionFile, "%s", versionName);

	fclose(versionFile);

	for (i=0;i<totalFiles-1;i++)
	{
		compressFile(filenames[i]);
	}

	compressFile(versionName);

	remove(versionName);

	length = ftell(pak);

	for (i=0;i<totalFiles;i++)
	{
		if (fileData[i].fileSize == 0)
		{
			break;
		}

		fileData[i].offset = SWAP32(fileData[i].offset);
		fileData[i].compressedSize = SWAP32(fileData[i].compressedSize);
		fileData[i].fileSize = SWAP32(fileData[i].fileSize);

		fwrite(&fileData[i], sizeof(FileData), 1, pak);
	}

	length = SWAP32(length);
	totalFiles = SWAP32(totalFiles);

	fwrite(&length, sizeof(int32_t), 1, pak);
	fwrite(&totalFiles, sizeof(int32_t), 1, pak);

	fclose(pak);

	printf("Compressing 100%%\nCompleted\n");

	return 0;
}

static void getFilenames(char *dirName, int *arraySize)
{
	DIR *dirp, *dirp2;
	struct dirent *dfile;
	char filename[MAX_FILE_LENGTH];
	char **ptr;

	dirp = opendir(dirName);

	while ((dfile = readdir(dirp)))
	{
		if (dfile->d_name[0] == '.')
		{
			continue;
		}

		sprintf(filename, "%s/%s", dirName, dfile->d_name);

		dirp2 = opendir(filename);

		if (dirp2)
		{
			closedir(dirp2);

			getFilenames(filename, arraySize);
		}

		else
		{
			filenames[totalFiles] = malloc(MAX_FILE_LENGTH);
			
			STRNCPY(filenames[totalFiles], filename, MAX_FILE_LENGTH);
			
			totalFiles++;
			
			if (totalFiles == *arraySize)
			{
				*arraySize += 500;
				
				ptr = realloc(filenames, *arraySize * sizeof(char *));
				
				if (ptr)
				{
					filenames = ptr;
				}
				
				else
				{
					printf("Failed to allocate %d bytes whilst indexing files\n", *arraySize * (int)sizeof(char *));

					exit(1);
				}
			}
		}
	}

	closedir(dirp);
}

static int compare(const void *a, const void *b)
{
	char* aa = *(char**)a;
	char* bb = *(char**)b;
	
	return strcmpignorecase(aa, bb);
}

static void cleanup()
{
	int i;
	
	if (filenames != NULL)
	{
		for (i=0;i<totalFiles-1;i++)
		{
			if (filenames[i] != NULL)
			{
				free(filenames[i]);
			}
		}
		
		free(filenames);
	}
	
	if (fileData != NULL)
	{
		free(fileData);
	}
}

static void compressFile(char *filename)
{
	FILE *infile;
	int compressionResult;
	int fileSize, ensuredSize;
	unsigned long compressedSize;
	gzFile fp;
	float percentage;
	unsigned char *buffer, *output;

	infile = fopen(filename, "rb");

	if (!infile)
	{
		printf("Couldn't open %s for reading!\n", filename);

		exit(1);
	}

	fseek(infile, 0L, SEEK_END);

	fileSize = ftell(infile);

	if (fileSize == 0)
	{
		printf("%s is an empty file.\n", filename);

		exit(1);
	}

	ensuredSize = fileSize * 1.01 + 12;

	compressedSize = ensuredSize;

	fclose(infile);

	buffer = malloc((fileSize + 1) * sizeof(unsigned char));

	if (buffer == NULL)
	{
		printf("Could not create buffer\n");

		exit(1);
	}

	output = malloc(ensuredSize * sizeof(unsigned char));

	if (output == NULL)
	{
		printf("Could not create output\n");

		exit(1);
	}

	fp = gzopen(filename, "rb");

	if (!fp)
	{
		printf("Couldn't open %s for reading!\n", filename);

		exit(1);
	}

	gzread(fp, buffer, fileSize);

	gzclose(fp);

	compressionResult = compress2(output, &compressedSize, buffer, fileSize, 9);

	if (compressionResult != Z_OK)
	{
		printf("Compression of %s failed\n", filename);

		if (compressionResult == Z_BUF_ERROR)
		{
			printf("Buffer too small\n");
		}

		else if (compressionResult == Z_MEM_ERROR)
		{
			printf("Out of RAM\n");
		}

		else if (compressionResult == Z_STREAM_ERROR)
		{
			printf("Stream error\n");
		}

		else
		{
			printf("Unknown error\n");
		}

		exit(1);
	}

	percentage = fileID;

	percentage /= totalFiles;

	percentage *= 100;

	printf("Compressing %02d%%...\r", (int)percentage);

	STRNCPY(fileData[fileID].filename, filename, MAX_FILE_LENGTH);

	fileData[fileID].fileSize = fileSize;
	fileData[fileID].compressedSize = compressedSize;
	fileData[fileID].offset = ftell(pak);

	fwrite(output, compressedSize, 1, pak);

	fileID++;

	free(buffer);

	free(output);
}

static void testPak(char *pakFile)
{
	FileData *fileData;
	int i;
	int32_t offset, fileCount;
	unsigned long size;
	FILE *fp;
	unsigned char *source, *dest;

	fp = fopen(pakFile, "rb");

	if (fp == NULL)
	{
		printf("Failed to open PAK file %s\n", pakFile);

		exit(1);
	}

	fseek(fp, -(sizeof(int32_t) + sizeof(int32_t)), SEEK_END);

	fread(&offset, sizeof(int32_t), 1, fp);
	fread(&fileCount, sizeof(int32_t), 1, fp);

	offset = SWAP32(offset);
	fileCount = SWAP32(fileCount);

	fileData = malloc(fileCount * sizeof(FileData));

	if (fileData == NULL)
	{
		printf("Could not allocate %d bytes for FileData\n", fileCount * (int)sizeof(FileData));

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

		printf("'%s' at offset %d : %d -> %d\n", fileData[i].filename, fileData[i].offset, fileData[i].compressedSize, fileData[i].fileSize);
	}

	for (i=0;i<fileCount;i++)
	{
		printf("Testing %s...", fileData[i].filename);

		fseek(fp, fileData[i].offset, SEEK_SET);

		source = malloc(fileData[i].compressedSize * sizeof(unsigned char));

		if (source == NULL)
		{
			printf("\nFailed to allocate %d bytes to load %s from PAK\n", fileData[i].compressedSize * (int)sizeof(unsigned char), fileData[i].filename);

			exit(1);
		}

		dest = malloc((fileData[i].fileSize + 1) * sizeof(unsigned char));

		if (dest == NULL)
		{
			printf("\nFailed to allocate %d bytes to load %s from PAK\n", fileData[i].fileSize * (int)sizeof(unsigned char), fileData[i].filename);

			exit(1);
		}

		fread(source, fileData[i].compressedSize, 1, fp);

		size = fileData[i].fileSize;

		uncompress(dest, &size, source, fileData[i].compressedSize);

		dest[size] = '\0';

		if (size != fileData[i].fileSize)
		{
			printf("\nFailed to decompress %s. Expected %d, got %ld\n", fileData[i].filename, fileData[i].fileSize, size);

			exit(1);
		}

		free(source);

		free(dest);

		printf("OK\n");
	}

	fclose(fp);

	free(fileData);

	printf("Test completed. No errors found\n");
}
