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

#include "headers.h"
#include "system/pak.h"

FILE *pak;
int totalFiles, fileID = 0;
FileData *fileData;

int countFiles(char *);
void cleanup(void);
void recurseDirectory(char *);

int main(int argc, char *argv[])
{
	int i, length;

	if (argc < 3)
	{
		printf("Usage   : pak <directory names> <outputname>\n");
		printf("Example : pak data music gfx sound font data.pak\n");
		exit(1);
	}

	pak = fopen(argv[argc - 1], "wb");

	for (i=1;i<argc-1;i++)
	{
		totalFiles += countFiles(argv[i]);
	}

	printf("Will compress %d files\n", totalFiles);

	printf("Compressing 00%%...\r");

	fileData = (FileData *)malloc(totalFiles * sizeof(FileData));

	if (fileData == NULL)
	{
		printf("Failed to create File Data\n");

		exit(1);
	}

	atexit(cleanup);

	for (i=1;i<argc-1;i++)
	{
		recurseDirectory(argv[i]);
	}

	length = ftell(pak);

	for (i=0;i<totalFiles;i++)
	{
		if (fileData[i].fileSize == 0)
		{
			break;
		}

		fwrite(&fileData[i], sizeof(FileData), 1, pak);
	}

	length = SDL_SwapLE32(length);
	totalFiles = SDL_SwapLE32(totalFiles);

	fwrite(&length, sizeof(long), 1, pak);
	fwrite(&totalFiles, sizeof(int), 1, pak);

	fclose(pak);

	printf("Compressing 100%%\nCompleted\n");

	return 0;
}

int countFiles(char *dirName)
{
	DIR *dirp, *dirp2;
	struct dirent *dfile;
	char filename[MAX_FILE_LENGTH];
	int count = 0;

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

			count += countFiles(filename);
		}

		else
		{
			count++;
		}
	}

	return count;
}

void cleanup()
{

}

void recurseDirectory(char *dirName)
{
	DIR *dirp, *dirp2;
	struct dirent *dfile;
	FILE *infile;
	char filename[1024];
	int compressionResult;
	unsigned long fileSize, compressedSize, ensuredSize;
	gzFile fp;
	float percentage;
	unsigned char *buffer, *output;

	dirp = opendir(dirName);

	if (dirp == NULL)
	{
		printf("%s: Directory does not exist or is not accessable\n", dirName);

		return;
	}

	while ((dfile = readdir(dirp)))
	{
		if (dfile->d_name[0] == '.')
		{
			continue;
		}

		snprintf(filename, sizeof(filename), "%s/%s", dirName, dfile->d_name);

		dirp2 = opendir(filename);

		if (dirp2)
		{
			closedir(dirp2);

			recurseDirectory(filename);
		}

		else
		{
			infile = fopen(filename, "rb");

			if (!infile)
			{
				printf("Couldn't open %s for reading!\n", filename);

				closedir(dirp);
				gzclose(pak);

				exit(1);
			}

			fseek(infile, 0L, SEEK_END);

			fileSize = ftell(infile);

			ensuredSize = fileSize * 1.01 + 12;

			compressedSize = ensuredSize;

			fclose(infile);

			buffer = (unsigned char *)malloc((fileSize + 1) * sizeof(unsigned char));

			if (buffer == NULL)
			{
				printf("Could not create buffer\n");

				exit(1);
			}

			output = (unsigned char *)malloc(ensuredSize * sizeof(unsigned char));

			if (output == NULL)
			{
				printf("Could not create output\n");

				exit(1);
			}

			fp = gzopen(filename, "rb");

			if (!fp)
			{
				printf("Couldn't open %s for reading!\n", filename);

				closedir(dirp);

				gzclose(pak);

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
					
					exit(1);
				}
				
				if (compressionResult == Z_MEM_ERROR)
				{
					printf("Out of RAM\n");
					
					exit(1);
				}
				
				if (compressionResult == Z_STREAM_ERROR)
				{
					printf("Stream error\n");
					
					exit(1);
				}
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
	}

	closedir(dirp);
}
