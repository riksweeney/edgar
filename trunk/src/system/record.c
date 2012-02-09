/*
Copyright (C) 2009-2012 Parallel Realities

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

#include "../headers.h"

#include "../map.h"
#include "error.h"
#include "random.h"

static void saveBuffer(void);
static void loadBuffer(void);

static int frame = 0;
static int inputBuffer[MAX_INPUTS];
static int bufferID = 0;
static FILE *replayBuffer;
static int inputsRead = 0;
static char screenshotPath[MAX_PATH_LENGTH];

extern Game game;

void setReplayData(char *name, int loadedGame)
{
	char mapFile[6];
	double version;
	long seed;
	int read;

	printf("Setting replay file to %s\n", name);

	replayBuffer = fopen(name, "rb");

	if (replayBuffer == NULL)
	{
		showErrorAndExit("Failed to open replay data %s", name);
	}

	game.gameType = REPLAYING;

	read = fread(&version, sizeof(double), 1, replayBuffer);

	printf("This version : %0.2f Replay version %0.2f\n", VERSION, version);

	if (version != VERSION)
	{
		printf("This replay is from a different version and may not function correctly\n");
	}

	read = fread(&seed, sizeof(long), 1, replayBuffer);

	printf("Setting seed %ld\n", seed);

	setSeed(seed);

	read = fread(mapFile, 5, 1, replayBuffer);

	mapFile[5] = '\0';

	if (loadedGame == FALSE)
	{
		printf("Loading map %s\n", mapFile);

		loadMap(mapFile, TRUE);
	}
}

void setRecordData(char *name)
{
	long seed;
	double version = VERSION;

	printf("Setting record file to %s\n", name);

	replayBuffer = fopen(name, "wb");

	if (replayBuffer == NULL)
	{
		showErrorAndExit("Failed to open replay data %s", name);
	}

	game.gameType = RECORDING;

	seed = time(NULL);

	fwrite(&version, sizeof(double), 1, replayBuffer);

	fwrite(&seed, sizeof(long), 1, replayBuffer);

	printf("Setting seed %ld\n", seed);

	setSeed(seed);
}

void setMapFile(char *name)
{
	printf("Setting map to %s\n", name);

	fwrite(name, 5, 1, replayBuffer);
}

void setScreenshotDir(char *name)
{
	STRNCPY(screenshotPath, name, sizeof(screenshotPath));

	printf("Set screenshot directory to %s\n", screenshotPath);
}

void takeScreenshot()
{
	char filename[MAX_PATH_LENGTH];

	if (strlen(screenshotPath) != 0)
	{
		snprintf(filename, sizeof(filename), "%s/edgar%06d.bmp", screenshotPath, frame);

		frame++;

		SDL_SaveBMP(game.screen, filename);
	}
}

void takeSingleScreenshot()
{
	SDL_SaveBMP(game.screen, "edgar.bmp");
}

void putBuffer(Input inp)
{
	int input = 0;

	if (inp.up == 1)        input |= 1;
	if (inp.down == 1)      input |= 2;
	if (inp.left == 1)      input |= 4;
	if (inp.right == 1)     input |= 8;
	if (inp.jump == 1)      input |= 16;
	if (inp.attack == 1)    input |= 32;
	if (inp.block == 1)     input |= 64;
	if (inp.activate == 1)  input |= 128;
	if (inp.interact == 1)  input |= 256;
	if (inp.previous == 1)  input |= 512;
	if (inp.next == 1)      input |= 1024;
	if (inp.inventory == 1) input |= 2048;
	if (inp.grabbing == 1)  input |= 4096;

	inputBuffer[bufferID] = input;

	bufferID++;

	if (bufferID == MAX_INPUTS)
	{
		saveBuffer();

		bufferID = 0;
	}
}

Input getBuffer()
{
	int input;
	Input inp;

	memset(&inp, 0, sizeof(Input));

	if (bufferID == 0)
	{
		loadBuffer();
	}

	if (inputsRead != MAX_INPUTS && bufferID == inputsRead)
	{
		printf("End of replay\n");

		exit(0);
	}

	input = inputBuffer[bufferID];

	if (input & 1)    inp.up = 1;
	if (input & 2)    inp.down = 1;
	if (input & 4)    inp.left = 1;
	if (input & 8)    inp.right = 1;
	if (input & 16)   inp.jump = 1;
	if (input & 32)   inp.attack = 1;
	if (input & 64)   inp.block = 1;
	if (input & 128)  inp.activate = 1;
	if (input & 256)  inp.interact = 1;
	if (input & 512)  inp.previous = 1;
	if (input & 1024) inp.next = 1;
	if (input & 2048) inp.inventory = 1;
	if (input & 4096) inp.grabbing = 1;

	bufferID++;

	if (bufferID == MAX_INPUTS)
	{
		bufferID = 0;
	}

	return inp;
}

static void saveBuffer()
{
	fwrite(inputBuffer, sizeof(int), bufferID, replayBuffer);
}

static void loadBuffer()
{
	inputsRead = fread(inputBuffer, sizeof(int), MAX_INPUTS, replayBuffer);

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
