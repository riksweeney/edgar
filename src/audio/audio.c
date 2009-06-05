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

#include "audio.h"
#include "../geometry.h"
#include "../system/pak.h"

static Sound sound[MAX_SOUNDS];
extern Game game;
extern Entity player;

static void preCacheSound(char *);

static int soundIndex = 0;

void preCacheSounds(char *filename)
{
	char *line, *savePtr;
	unsigned char *buffer;

	if (game.audio == FALSE)
	{
		return;
	}

	buffer = loadFileFromPak(filename);

	line = strtok_r((char *)buffer, "\n", &savePtr);

	while (line != NULL)
	{
		if (line[strlen(line) - 1] == '\n')
		{
			line[strlen(line) - 1] = '\0';
		}

		if (line[strlen(line) - 1] == '\r')
		{
			line[strlen(line) - 1] = '\0';
		}

		preCacheSound(line);

		line = strtok_r(NULL, "\n", &savePtr);
	}

	free(buffer);
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

	chunk = loadSound(name);

	sound[soundIndex].effect = chunk;
	STRNCPY(sound[soundIndex].name, name, sizeof(sound[soundIndex].name));

	soundIndex++;
}

void playSound(char *name, int channelMin, int channelMax, int x, int y)
{
	int i, distance, volume, channel;
	Mix_Chunk *chunk = NULL;

	if (game.audio == FALSE || game.sfxDefaultVolume == 0)
	{
		return;
	}

	channel = channelMin;

	if (channelMin != channelMax)
	{
		for (i=channelMin;i<=channelMax;i++)
		{
			if (Mix_Playing(i) == FALSE)
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
		STRNCPY(sound[soundIndex].name, name, sizeof(sound[soundIndex].name));

		soundIndex++;
	}

	/* The further away the player is, the quieter the sound */

	distance = getDistance(player.x, player.y, x, y);

	volume = game.sfxDefaultVolume * VOLUME_STEPS;

	if (distance < SCREEN_WIDTH * 0.75)
	{
	}

	else if (distance < SCREEN_WIDTH * 1)
	{
		volume /= 2;
	}

	else if (distance < SCREEN_WIDTH * 1.25)
	{
		volume /= 4;
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

	if (game.audio == FALSE)
	{
		return NULL;
	}

	snprintf(path, sizeof(path), "%s", name);

	/* Load the sound specified by the filename */

	chunk = loadSoundFromPak(name);

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

	Mix_PlayChannel(channel, chunk, 0);
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

			sound[i].effect = NULL;
		}
	}

	soundIndex = 0;
}

int initAudio()
{
	/* Set the audio rate to 22050, default mix, 2 channels and a 1024 byte buffer */

	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) != 0)
	{
		printf("Could not open audio: %s\n", Mix_GetError());

		game.audio = FALSE;
	}

	else
	{
		game.audio = TRUE;
	}

	return game.audio;
}
