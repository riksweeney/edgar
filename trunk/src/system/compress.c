#include "../headers.h"

int compressFile(char *sourceName, char *destName)
{
	int ret, flush;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK_SIZE];
	unsigned char out[CHUNK_SIZE];
	FILE *source, *dest;

	source = fopen(sourceName, "rb");

	if (source == NULL)
	{
		printf("Failed to read %s for compressing\n", sourceName);

		exit(1);
	}

	dest = fopen(destName, "wb");

	if (dest == NULL)
	{
		fclose(source);

		printf("Failed to write %s for compressing\n", destName);

		exit(1);
	}

	/* allocate deflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;

	ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);

	if (ret != Z_OK)
	{
		fclose(source);
		fclose(dest);

		return ret;
	}

	/* compress until end of file */
	do
	{
		strm.avail_in = fread(in, 1, CHUNK_SIZE, source);

		if (ferror(source))
		{
			(void)deflateEnd(&strm);

			fclose(source);
			fclose(dest);

			return Z_ERRNO;
		}

		flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
		strm.next_in = in;

		/* run deflate() on input until output buffer not full, finish
			compression if all of source has been read in */
		do
		{
			strm.avail_out = CHUNK_SIZE;
			strm.next_out = out;
			ret = deflate(&strm, flush);    /* no bad return value */
			have = CHUNK_SIZE - strm.avail_out;

			if (fwrite(out, 1, have, dest) != have || ferror(dest))
			{
				(void)deflateEnd(&strm);

				fclose(source);
				fclose(dest);

				return Z_ERRNO;
			}

		}

		while (strm.avail_out == 0);

		/* done when last data in file processed */
	}

	while (flush != Z_FINISH);

	/* clean up and return */
	(void)deflateEnd(&strm);

	fclose(source);
	fclose(dest);

	return Z_OK;
}

int decompressFile(char *sourceName, char *destName)
{
	int ret;
	unsigned have;
	z_stream strm;
	unsigned char in[CHUNK_SIZE];
	unsigned char out[CHUNK_SIZE];
	FILE *source, *dest;

	source = fopen(sourceName, "rb");

	if (source == NULL)
	{
		printf("Failed to read %s for decompressing\n", sourceName);

		exit(1);
	}

	dest = fopen(destName, "wb");

	if (dest == NULL)
	{
		fclose(source);

		printf("Failed to write %s for decompressing\n", destName);

		exit(1);
	}

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;

	ret = inflateInit(&strm);

	if (ret != Z_OK)
	{
		fclose(source);
		fclose(dest);

		return ret;
	}

	/* decompress until deflate stream ends or end of file */
	do
	{
		strm.avail_in = fread(in, 1, CHUNK_SIZE, source);

		if (ferror(source))
		{
			(void)inflateEnd(&strm);

			fclose(source);
			fclose(dest);

			return Z_ERRNO;
		}

		if (strm.avail_in == 0)
		{
			break;
		}

		strm.next_in = in;

		/* run inflate() on input until output buffer not full */
		do
		{
			strm.avail_out = CHUNK_SIZE;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);

			switch (ret)
			{
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;     /* and fall through */
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&strm);

					fclose(source);
					fclose(dest);

					return ret;
			}

			have = CHUNK_SIZE - strm.avail_out;

			if (fwrite(out, 1, have, dest) != have || ferror(dest))
			{
				(void)inflateEnd(&strm);

				fclose(source);
				fclose(dest);

				return Z_ERRNO;
			}
		}

		while (strm.avail_out == 0);

		/* done when inflate() says it's done */
	}

	while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);

	fclose(source);
	fclose(dest);

	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}
