#include "headers.h"

#include "audio.h"
#include "geometry.h"

static Sound sound[MAX_SOUNDS];
extern Game game;
extern Entity player;

static void preCacheSound(char *);

static int soundIndex = 0;

void preCacheSounds(char *filename)
{
	char line[MAX_LINE_LENGTH];
	FILE *fp;

	fp = fopen(filename, "rb");

	if (fp == NULL)
	{
		printf("Failed to open sfx file: %s\n", filename);

		exit(1);
	}

	while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
	{
		if (line[strlen(line) - 1] == '\n')
		{
			line[strlen(line) - 1] = '\0';
		}

		preCacheSound(line);
	}

	fclose(fp);
}

static void preCacheSound(char *name)
{
	int i;
	Mix_Chunk *chunk = NULL;

	for (i=0;i<soundIndex;i++)
	{
		if (strcmpignorecase(sound[i].name, name) == 0)
		{
			return;
		}
	}

	if (soundIndex == MAX_SOUNDS)
	{
		printf("Ran out of space for sounds\n");

		abort();
	}

	printf("Precaching %s\n", name);

	chunk = loadSound(name);

	sound[soundIndex].effect = chunk;
	strcpy(sound[soundIndex].name, name);

	soundIndex++;
}

void playSound(char *name, int channelMin, int channelMax, int x, int y)
{
	int i, distance, volume, channel;
	Mix_Chunk *chunk = NULL;

	channel = channelMin;

	if (channelMin != channelMax)
	{
		for (i=channelMin;i<=channelMax;i++)
		{
			if (Mix_Playing(i) == 0)
			{
				channel = i;

				break;
			}
		}
	}

	for (i=0;i<soundIndex;i++)
	{
		if (strcmpignorecase(sound[i].name, name) == 0)
		{
			chunk = sound[i].effect;

			break;
		}
	}

	if (chunk == NULL)
	{
		if (soundIndex == MAX_SOUNDS)
		{
			printf("Ran out of space for sounds\n");

			abort();
		}

		chunk = loadSound(name);

		sound[soundIndex].effect = chunk;
		strcpy(sound[soundIndex].name, name);

		soundIndex++;
	}

	/* The further away the player is, the quieter the sound */

	distance = getDistance(player.x, player.y, x, y);

	volume = game.audioVolume;

	if (distance < SCREEN_WIDTH * 0.75)
	{
		volume = game.audioVolume;
	}

	else if (distance < SCREEN_WIDTH * 1)
	{
		volume = game.audioVolume / 2;
	}

	else if (distance < SCREEN_WIDTH * 1.25)
	{
		volume = game.audioVolume / 4;
	}

	else
	{
		return;
	}

	Mix_VolumeChunk(chunk, volume);

	playSoundChunk(chunk, channel);
}

Mix_Chunk *loadSound(char *name)
{
	char path[MAX_PATH_LENGTH];
	Mix_Chunk *chunk;

	sprintf(path, INSTALL_PATH"%s", name);

	/* Load the sound specified by the filename */

	printf("Loading sound %s\n", path);

	chunk = Mix_LoadWAV(path);

	if (chunk == NULL)
	{
		printf("Failed to load sound %s\n", path);

		exit(1);
	}

	return chunk;
}

void playSoundChunk(Mix_Chunk *chunk, int channel)
{
	/* Play the sound on the first free channel and only play it once */

	if (game.audio == TRUE)
	{
		Mix_PlayChannel(channel, chunk, 0);
	}
}

void freeSounds()
{
	int i;

	for (i=0;i<MAX_SOUNDS;i++)
	{
		if (sound[i].effect != NULL)
		{
			Mix_FreeChunk(sound[i].effect);

			sound[i].name[0] = '\0';
		}
	}

	soundIndex = 0;
}
