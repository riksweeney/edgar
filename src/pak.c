#include "headers.h"

FILE *pak;
int dirs = 0, files = 0;
int totalFiles = 0;
unsigned char *buffer, *output;

int countFiles(char *);
void cleanup(void);
void recurseDirectory(char *);

int main(int argc, char *argv[])
{
	int i, files = 0;

	if (argc < 3)
	{
		printf("Usage   : pak <directory names> <outputname>\n");
		printf("Example : pak data music gfx sound data.pak\n");
		exit(1);
	}

	pak = fopen(argv[argc - 1], "wb");

	for (i=1;i<argc-1;i++)
	{
		files += countFiles(argv[i]);
	}

	printf("There are %d files\n", files);

	printf("Paking...000%%\n");

	fflush(stdout);

	output = NULL;
	buffer = NULL;

	atexit(cleanup);

	for (i=1;i<argc-1;i++)
	{
		recurseDirectory(argv[i]);
	}
	/*
	unsigned int pos = ftell(pak);

	for (int i = 0 ; i < files ; i++)
	{
		if (fileData[i].fSize == 0)
		{
			break;
		}

		fwrite(&fileData[i], sizeof(FileData), 1, pak);
	}

	unsigned int numberOfFiles = totalFiles;

	pos = SDL_SwapLE32(pos);
	numberOfFiles = SDL_SwapLE32(numberOfFiles);

	fwrite(&pos, sizeof(unsigned int), 1, pak);
	fwrite(&numberOfFiles, sizeof(unsigned int), 1, pak);

	fclose(pak);

	printf("\nPak: All Done. Added %d files\n", numberOfFiles);
	*/
	return 0;
}

int countFiles(char *dirName)
{
	DIR *dirp, *dirp2;
	struct dirent *dfile;
	char filename[1024];
	int count = 0;

	dirp = opendir(dirName);

	while ((dfile = readdir(dirp)))
	{
		if (dfile->d_name[0] == '.')
		{
			continue;
		}

		sprintf(filename, "%s/%s", dirName, dfile->d_name);

		printf("Opening file %s\n", filename);

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
	if (buffer != NULL)
	{
		free(buffer);
	}

	if (output != NULL)
	{
		free(output);
	}
}

void recurseDirectory(char *dirName)
{
	DIR *dirp, *dirp2;
	struct dirent *dfile;
	FILE *infile;
	char filename[1024];
	unsigned long fileSize, compressedSize, ensuredSize;
	gzFile fp;

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

		sprintf(filename, "%s/%s", dirName, dfile->d_name);

		printf("Trying to open %s\n", filename);

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

			fseek(infile, SEEK_SET, SEEK_END);

			fileSize = ftell(infile);

			ensuredSize = fileSize * 1.01 + 12;

			compressedSize = 0;

			fclose(infile);

			if (buffer != NULL)
			{
				free(buffer);
			}

			buffer = (unsigned char *)malloc(fileSize * sizeof(unsigned char));

			if (buffer == NULL)
			{
				printf("Could not create buffer\n");

				exit(1);
			}

			if (output != NULL)
			{
				free(output);
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

			compressedSize = gzread(fp, buffer, fileSize);

			printf("Read %ld into buffer\n", compressedSize);

			gzclose(fp);

			printf("Trying to compress %s with a max size of %ld\n", filename, ensuredSize);

			compress2(output, &compressedSize, buffer, ensuredSize, 9);

			printf("%s %ld -> %ld\n", filename, fileSize, compressedSize);
		}
	}

	closedir(dirp);

	dirs++;
}
