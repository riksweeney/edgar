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

#include "../entity.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"
#include "../event/objective.h"
#include "../world/target.h"
#include "../map.h"
#include "../player.h"
#include "../inventory.h"
#include "compress.h"
#include "resources.h"
#include "../game.h"
#include "load_save.h"
#include "../hud.h"
#include "pak.h"
#include "../input.h"

static char gameSavePath[MAX_PATH_LENGTH], tempFile[MAX_PATH_LENGTH];

static void removeTemporaryData(void);
static void copyFile(char *, char *);

extern Game game;

#ifndef WIN32
	void setupUserHomeDirectory()
	{
		char *userHome;
		char *name;
		char dir[MAX_PATH_LENGTH];
		struct passwd *pass;

		name = getlogin();

		STRNCPY(dir, "", sizeof(dir));

		if (name != NULL)
		{
			pass = getpwnam(name);
		}

		else
		{
			pass = getpwuid(geteuid());
		}

		if (pass == NULL)
		{
			printf("Couldn't determine the user home directory. Exiting.\n");
			exit(1);
		}

		userHome = pass->pw_dir;

		snprintf(dir, sizeof(dir), "%s/.parallelrealities", userHome);

		if ((mkdir(dir, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) != 0) && (errno != EEXIST))
		{
			printf("Couldn't create required directory '%s'", dir);

			exit(1);
		}

		snprintf(dir, sizeof(dir), "%s/.parallelrealities/edgar", userHome);

		if ((mkdir(dir, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) != 0) && (errno != EEXIST))
		{
			printf("Couldn't create required directory '%s'", dir);

			exit(1);
		}

		snprintf(gameSavePath, sizeof(gameSavePath), "%s/.parallelrealities/edgar/", userHome);

		snprintf(tempFile, sizeof(tempFile), "%stmpsave", gameSavePath);

		removeTemporaryData();
	}
#else
	void setupUserHomeDirectory()
	{
		STRNCPY(gameSavePath, "", sizeof(gameSavePath));
		STRNCPY(tempFile, "tmpsave", sizeof(gameSavePath));

		removeTemporaryData();
	}
#endif

void newGame()
{
	freeGameResources();
	
	loadMap("map01", TRUE);
	
	cameraSnapToTargetEntity();
}

int loadGame(int slot)
{
	char itemName[MAX_MESSAGE_LENGTH], mapName[MAX_MESSAGE_LENGTH];
	char saveFile[MAX_PATH_LENGTH], *line, *savePtr;
	int loadedMap = FALSE;
	unsigned char *buffer;
	FILE *fp;

	snprintf(saveFile, sizeof(saveFile), "%ssave%d", gameSavePath, slot);

	fp = fopen(saveFile, "rb");

	if (fp == NULL)
	{
		return FALSE;
	}

	fclose(fp);

	freeGameResources();

	printf("Loading save data from %s\n", saveFile);

	buffer = decompressFile(saveFile);

	printf("Reading save data\n");

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

		sscanf(line, "%s", itemName);

		if (strcmpignorecase("PLAYER_LOCATION", itemName) == 0)
		{
			if (loadedMap == TRUE)
			{
				printf("Already loaded Map data\n");
			}

			else
			{
				sscanf(line, "%*s %s\n", itemName);

				printf("Loading save location %s\n", itemName);

				loadMap(itemName, FALSE);

				snprintf(mapName, sizeof(mapName), "MAP_NAME %s", itemName);

				loadedMap = TRUE;
			}
		}

		else if (strcmpignorecase(line, mapName) == 0)
		{
			printf("Loading entities for map %s\n", mapName);

			loadResources(savePtr);
		}

		line = strtok_r(NULL, "\n", &savePtr);
	}

	free(buffer);

	copyFile(saveFile, tempFile);

	buffer = decompressFile(tempFile);

	free(buffer);

	cameraSnapToTargetEntity();
	
	freeMessageQueue();

	printf("Load completed\n");

	return TRUE;
}

void saveGame(int slot)
{
	char itemName[MAX_MESSAGE_LENGTH], *line, *savePtr;
	char saveFile[MAX_PATH_LENGTH];
	char *mapName = getMapName();
	unsigned char *buffer;
	int skipping = FALSE;
	FILE *read;
	FILE *write;

	printf("Saving game\n");

	snprintf(saveFile, sizeof(saveFile), "%ssave%d", gameSavePath, slot);

	read = fopen(tempFile, "rb");

	write = fopen(saveFile, "wb");
	
	fprintf(write, "VERSION %0.2f\n", VERSION);

	fprintf(write, "PLAYER_LOCATION %s\n", mapName);

	if (read != NULL)
	{
		printf("Copying persisting data\n");

		fclose(read);

		buffer = decompressFile(tempFile);

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

			if (skipping == FALSE)
			{
				sscanf(line, "%s", itemName);

				if (strcmpignorecase("PLAYER_DATA", itemName) == 0 || strcmpignorecase("PLAYER_INVENTORY", itemName) == 0 ||
					strcmpignorecase("PLAYER_LOCATION", itemName) == 0)
				{
					skipping = TRUE;
				}

				else if (strcmpignorecase("MAP_NAME", itemName) == 0)
				{
					sscanf(line, "%*s %s\n", itemName);

					if (strcmpignorecase(itemName, mapName) == 0)
					{
						skipping = TRUE;
					}

					else
					{
						fprintf(write, "%s\n", line);
					}
				}

				else
				{
					fprintf(write, "%s\n", line);
				}
			}

			else
			{
				sscanf(line, "%s", itemName);

				if (strcmpignorecase("MAP_NAME", itemName) == 0)
				{
					sscanf(line, "%*s %s\n", itemName);

					if (strcmpignorecase(itemName, mapName) != 0)
					{
						skipping = FALSE;

						fprintf(write, "%s\n", line);
					}
				}
			}

			line = strtok_r(NULL, "\n", &savePtr);
		}

		free(buffer);
	}

	/* Save the player's position */

	printf("Writing player location\n");

	fprintf(write, "MAP_NAME %s\n", mapName);

	printf("Writing player data\n");

	fprintf(write, "PLAYER_DATA\n");

	writePlayerToFile(write);

	printf("Writing player inventory\n");

	fprintf(write, "PLAYER_INVENTORY\n");

	writeInventoryToFile(write);

	fprintf(write, "ENTITY_DATA\n");

	/* Now write out all of the Entities */

	writeEntitiesToFile(write);

	/* Now the targets */

	writeTargetsToFile(write);

	/* And the triggers */

	writeTriggersToFile(write);

	/* Add the global triggers */

	writeGlobalTriggersToFile(write);

	/* Add the objectives */

	writeObjectivesToFile(write);

	/* Save the player data */

	fclose(write);

	#if DEV == 1
		copyFile(saveFile, "savedata");
	#endif

	compressFile(saveFile);
}

void saveTemporaryData()
{
	char *line, itemName[MAX_MESSAGE_LENGTH], *savePtr;
	char swapFile[MAX_PATH_LENGTH];
	char *mapName = getMapName();
	unsigned char *buffer;
	int skipping = FALSE;
	FILE *read;
	FILE *write;

	snprintf(swapFile, sizeof(swapFile), "%sswap", gameSavePath);

	read = fopen(tempFile, "rb");

	write = fopen(swapFile, "wb");

	if (read != NULL)
	{
		fclose(read);

		buffer = decompressFile(tempFile);

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

			if (skipping == FALSE)
			{
				sscanf(line, "%s", itemName);

				if (strcmpignorecase("PLAYER_DATA", line) == 0 || strcmpignorecase("PLAYER_INVENTORY", line) == 0 ||
					strcmpignorecase("GLOBAL_TRIGGER", line) == 0 || strcmpignorecase("OBJECTIVE", line) == 0 ||
					strcmpignorecase("PLAYER_LOCATION", line) == 0)
				{
					skipping = TRUE;
				}

				else if (strcmpignorecase("MAP_NAME", itemName) == 0)
				{
					sscanf(line, "%*s %s\n", itemName);

					if (strcmpignorecase(itemName, mapName) == 0)
					{
						skipping = TRUE;
					}

					else
					{
						fprintf(write, "%s\n", line);
					}
				}

				else
				{
					fprintf(write, "%s\n", line);
				}
			}

			else
			{
				sscanf(line, "%s", itemName);

				if (strcmpignorecase("MAP_NAME", itemName) == 0)
				{
					sscanf(line, "%*s %s\n", itemName);

					if (strcmpignorecase(itemName, mapName) != 0)
					{
						skipping = FALSE;

						fprintf(write, "%s\n", line);
					}
				}
			}

			line = strtok_r(NULL, "\n", &savePtr);
		}

		free(buffer);

		if (remove(tempFile) != 0)
		{
			perror("Could not remove temporary file");

			exit(1);
		}
	}

	fprintf(write, "MAP_NAME %s\n", mapName);

	fprintf(write, "ENTITY_DATA\n");

	/* Now write out all of the Entities */

	writeEntitiesToFile(write);

	/* Now the targets */

	writeTargetsToFile(write);

	/* And the triggers */

	writeTriggersToFile(write);

	fclose(write);

	if (rename(swapFile, tempFile) != 0)
	{
		perror("Could not rename temporary file");

		exit(1);
	}

	#if DEV == 1
		copyFile(tempFile, "tmpdata");
	#endif

	compressFile(tempFile);
}

int hasPersistance(char *mapName)
{
	int val = FALSE;
	char *line, itemName[MAX_MESSAGE_LENGTH], *savePtr;
	unsigned char *buffer;
	FILE *read;

	snprintf(itemName, sizeof(itemName), "MAP_NAME %s", mapName);

	read = fopen(tempFile, "rb");

	if (read == NULL)
	{
		printf("No persistance data found\n");

		return val;
	}

	fclose(read);

	buffer = decompressFile(tempFile);

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

		if (strcmpignorecase(line, itemName) == 0)
		{
			val = TRUE;
		}

		line = strtok_r(NULL, "\n", &savePtr);
	}

	free(buffer);

	return val;
}

void loadPersitanceData(char *mapName)
{
	char *line, itemName[MAX_MESSAGE_LENGTH], *savePtr;
	int found = FALSE;
	unsigned char *buffer;

	snprintf(itemName, sizeof(itemName), "MAP_NAME %s", mapName);

	buffer = decompressFile(tempFile);

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

		if (strcmpignorecase(line, itemName) == 0)
		{
			found = TRUE;

			printf("Loading peristance data for: %s\n", itemName);

			loadResources(savePtr);
		}

		line = strtok_r(NULL, "\n", &savePtr);
	}

	free(buffer);

	if (found == FALSE)
	{
		printf("Failed to find persistance data!\n");

		exit(1);
	}
}

static void removeTemporaryData()
{
	FILE *fp;

	fp = fopen(tempFile, "rb");

	if (fp != NULL)
	{
		fclose(fp);

		if (remove(tempFile) != 0)
		{
			perror("Could not remove temporary file");

			exit(1);
		}
	}
}

void loadConfig()
{
	FILE *fp;
	char settingsFile[MAX_PATH_LENGTH], *line, *savePtr;
	unsigned char *buffer;
	long length;

	game.hasConfig = FALSE;

	snprintf(settingsFile, sizeof(settingsFile), "%sconfig", gameSavePath);

	resetControls(FALSE);

	fp = fopen(settingsFile, "rb");

	if (fp == NULL)
	{
		resetGameSettings();

		return;
	}

	fseek(fp, 0L, SEEK_END);

	length = ftell(fp);

	buffer = (unsigned char *)malloc((length + 1) * sizeof(unsigned char));

	if (buffer == NULL)
	{
		printf("Could not allocate a whole %ld bytes for config file...\n", length);

		exit(1);
	}

	fseek(fp, 0L, SEEK_SET);

	fread(buffer, length, 1, fp);

	buffer[length] = '\0';

	fclose(fp);

	line = strtok_r((char *)buffer, "\n", &savePtr);

	while (line != NULL)
	{
		if (strcmpignorecase(line, "CONTROLS") == 0)
		{
			readControlsFromFile(savePtr);
		}

		else if (strcmpignorecase(line, "GAME_SETTINGS") == 0)
		{
			readGameSettingsFromFile(savePtr);
		}

		line = strtok_r(NULL, "\n", &savePtr);
	}

	game.hasConfig = TRUE;

	free(buffer);
}

void saveConfig()
{
	FILE *fp;
	char settingsFile[MAX_PATH_LENGTH];

	snprintf(settingsFile, sizeof(settingsFile), "%sconfig", gameSavePath);

	fp = fopen(settingsFile, "wb");

	if (fp == NULL)
	{
		perror("Could not save settings");

		exit(1);
	}

	writeControlsToFile(fp);

	writeGameSettingsToFile(fp);

	fclose(fp);
}

static void copyFile(char *src, char *dest)
{
	char c;
	FILE *sourceFile, *destFile;

	sourceFile = fopen(src, "rb");

	destFile = fopen(dest, "wb");

	while (!feof(sourceFile))
	{
		c = fgetc(sourceFile);

		fputc(c, destFile);
	}

	fclose(sourceFile);

	fclose(destFile);
}
