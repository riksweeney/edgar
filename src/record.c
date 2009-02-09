#include "headers.h"

#include "random.h"

static void saveBuffer(void);
static void loadBuffer(void);

static int frame = 0;
static Input inputBuffer[MAX_INPUTS];
static int bufferID = 0;
static FILE *replayBuffer;
static int inputsRead = 0;

extern Game game;

void setReplayData(char *name)
{
	long seed;
	
	printf("Setting record file to %s\n", name);
	
	replayBuffer = fopen(name, "rb");
	
	if (replayBuffer == NULL)
	{
		printf("Failed to open replay data %s\n", name);
		
		exit(1);
	}
	
	game.gameType = REPLAYING;
	
	fread(&seed, sizeof(long), 1, replayBuffer);
	
	printf("Setting seed %ld\n", seed);
	
	setSeed(seed);
}

void setRecordData(char *name)
{
	long seed;
	
	printf("Setting replay file to %s\n", name);
	
	replayBuffer = fopen(name, "wb");
	
	if (replayBuffer == NULL)
	{
		printf("Failed to open replay data %s\n", name);
		
		exit(1);
	}
	
	game.gameType = RECORDING;
	
	seed = time(NULL);
	
	fwrite(&seed, sizeof(long), 1, replayBuffer);
	
	printf("Setting seed %ld\n", seed);
	
	setSeed(seed);
}

void takeScreenshot()
{
	char filename[MAX_PATH_LENGTH];

	sprintf(filename, "/home/rik/temp/edgar%06d.bmp", frame);

	frame++;
	
	if (frame == 3600)
	{
		printf("Ending\n");
		
		exit(0);
	}

	SDL_SaveBMP(game.screen, filename);
}

void putBuffer(Input inp)
{
	inputBuffer[bufferID] = inp;
	
	bufferID++;
	
	if (bufferID == MAX_INPUTS)
	{
		saveBuffer();
		
		bufferID = 0;
	}
}

Input getBuffer()
{
	Input inp;
	
	if (bufferID == 0)
	{
		loadBuffer();
	}
	
	if (inputsRead != MAX_INPUTS && bufferID == inputsRead)
	{
		printf("End of replay\n");
		
		exit(0);
	}
	
	inp = inputBuffer[bufferID];
	
	bufferID++;
	
	if (bufferID == MAX_INPUTS)
	{
		bufferID = 0;
	}
	
	return inp;
}

static void saveBuffer()
{
	fwrite(inputBuffer, sizeof(Input), bufferID, replayBuffer);
}

static void loadBuffer()
{
	inputsRead = fread(inputBuffer, sizeof(Input), MAX_INPUTS, replayBuffer);
	
	if (inputsRead != MAX_INPUTS)
	{
		printf("Replay buffer not completely filled. Only read %d\n", inputsRead);
	}
}

void flushBuffer(int gameType)
{
	if (gameType == RECORDING)
	{
		saveBuffer();
	}
	
	if (replayBuffer != NULL)
	{
		fclose(replayBuffer);
	}
}
