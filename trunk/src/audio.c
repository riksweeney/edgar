#include "audio.h"

void playSoundChunk(Mix_Chunk *, int);
int getDistance(int, int, int, int);
Mix_Chunk *loadSound(char *);

static int soundIndex = 0;

void playSound(char *name, int channel, int x, int y)
{
	int i, distance, volume;
	Mix_Chunk *chunk = NULL;
	
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
	
	distance = getDistance(player.x, x, player.y, y);
	
	volume = game.audioVolume;
	
	if (distance < SCREEN_WIDTH)
	{
		volume = game.audioVolume;
	}
	
	else if (distance < SCREEN_WIDTH * 2)
	{
		volume = game.audioVolume / 2;
	}
	
	else if (distance < SCREEN_WIDTH * 4)
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
	char path[MAX_LINE_LENGTH];
	Mix_Chunk *chunk;
	
	sprintf(path, "%ssound/%s", INSTALL_PATH, name);
	
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
	
	if (game.audio == ACTIVE)
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
