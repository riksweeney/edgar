#include "../headers.h"

void compressFile(char *sourceName)
{
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

	fp = fopen(sourceName, "rb");

	fseek(fp, 0L, SEEK_END);

	compressedSize = ftell(fp);

	compressedSize -= sizeof(unsigned long);

	fseek(fp, 0L, SEEK_SET);

	fread(&fileSize, sizeof(unsigned long), 1, fp);

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
