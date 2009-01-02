#include "audio.h"

Mix_Chunk *loadSound(char *);

void loadSoundToIndex(char *name, int index)
{
	if (sound[index].effect != NULL)
	{
		printf("Will not replace sound in index %d\n", index);
		
		exit(1);
	}
	
	sound[index].effect = loadSound(name);
}

Mix_Chunk *loadSound(char *name)
{
	char path[MAX_LINE_LENGTH] = INSTALL_PATH;
	Mix_Chunk *chunk;
	
	strcat(path, name);
	
	/* Load the sound specified by the filename */
	
	chunk = Mix_LoadWAV(path);

	if (chunk == NULL)
	{
		printf("Failed to load sound %s\n", path);
		
		exit(1);
	}
	
	return chunk;
}

void playSound(Mix_Chunk *chunk, int channel)
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
		}
	}
}
