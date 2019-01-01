/*
Copyright (C) 2009-2019 Parallel Realities

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

static int32_t inputBuffer[MAX_INPUTS];
static int bufferID = 0;
static FILE *replayBuffer;
static int inputsRead = 0;

extern Game game;

void setReplayData(char *name, int loadedGame)
{
	char mapFile[6];
	double version;
	int32_t seed;

	printf("Setting replay file to %s\n", name);

	replayBuffer = fopen(name, "rb");

	if (replayBuffer == NULL)
	{
		showErrorAndExit("Failed to open replay data %s", name);
	}

	game.gameType = REPLAYING;

	fread(&version, sizeof(double), 1, replayBuffer);

	printf("This version : %0.2f Replay version %0.2f\n", VERSION, version);

	if (version != VERSION)
	{
		printf("This replay is from a different version and may not function correctly\n");
	}

	fread(&seed, sizeof(int32_t), 1, replayBuffer);

	seed = SWAP32(seed);

	printf("Setting seed %d\n", seed);

	setSeed(seed);

	fread(mapFile, 5, 1, replayBuffer);

	mapFile[5] = '\0';

	if (loadedGame == FALSE)
	{
		printf("Loading map %s\n", mapFile);

		loadMap(mapFile, TRUE);
	}
}

void setRecordData(char *name)
{
	int32_t seed;
	double version = VERSION;

	printf("Setting record file to %s\n", name);

	replayBuffer = fopen(name, "wb");

	if (replayBuffer == NULL)
	{
		showErrorAndExit("Failed to open replay data %s", name);
	}

	game.gameType = RECORDING;

	seed = time(NULL);

	seed = SWAP32(seed);

	fwrite(&version, sizeof(double), 1, replayBuffer);

	fwrite(&seed, sizeof(int32_t), 1, replayBuffer);

	printf("Setting seed %d\n", seed);

	setSeed(seed);
}

void setMapFile(char *name)
{
	printf("Setting map to %s\n", name);

	fwrite(name, 5, 1, replayBuffer);
}

void putBuffer(Input inp)
{
	int32_t input = 0;

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

	input = SWAP32(input);

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
	int32_t input;
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

	input = SWAP32(input);

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
	fwrite(inputBuffer, sizeof(int32_t), bufferID, replayBuffer);
}

static void loadBuffer()
{
	inputsRead = fread(inputBuffer, sizeof(int32_t), MAX_INPUTS, replayBuffer);

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
