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

#include "../audio/music.h"
#include "../entity.h"
#include "../event/global_trigger.h"
#include "../event/map_trigger.h"
#include "../event/objective.h"
#include "../event/script.h"
#include "../event/trigger.h"
#include "../game.h"
#include "../graphics/font.h"
#include "../graphics/graphics.h"
#include "../hud.h"
#include "../input.h"
#include "../inventory.h"
#include "../map.h"
#include "../medal.h"
#include "../player.h"
#include "../world/target.h"
#include "compress.h"
#include "error.h"
#include "load_save.h"
#include "resources.h"

static char gameSavePath[MAX_PATH_LENGTH], tempFile[MAX_PATH_LENGTH], saveFileIndex[MAX_PATH_LENGTH], continueFile[MAX_PATH_LENGTH];
static int temporaryDataExists;

static void removeTemporaryData(void);
static void copyFile(char *, char *);
static void updateSaveFileIndex(int);
static void patchSaveGame(char *, double);
static void showPatchMessage(char *);

extern Game game;

#if UNIX == 1
	void setupUserHomeDirectory()
	{
		char *userHome;
		char dir[MAX_PATH_LENGTH];
		struct passwd *pass;

		pass = getpwuid(geteuid());

		if (pass == NULL)
		{
			printf("Couldn't determine the user home directory. Exiting.\n");

			exit(1);
		}

		userHome = pass->pw_dir;

		#if MACOS == 1
		SNPRINTF(dir, sizeof(dir), "%s/Library/Application Support", userHome);
		#else
		SNPRINTF(dir, sizeof(dir), "%s/.parallelrealities", userHome);
		#endif

		if (mkdir(dir, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) != 0 && errno != EEXIST)
		{
			printf("Couldn't create required directory '%s'\n", dir);

			exit(1);
		}

		#if MACOS == 1
		SNPRINTF(dir, sizeof(dir), "%s/Library/Application Support/Edgar", userHome);
		#else
		SNPRINTF(dir, sizeof(dir), "%s/.parallelrealities/edgar", userHome);
		#endif

		if (mkdir(dir, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) != 0 && errno != EEXIST)
		{
			printf("Couldn't create required directory '%s'\n", dir);

			exit(1);
		}

		#if MACOS == 1
		SNPRINTF(gameSavePath, sizeof(gameSavePath), "%s/Library/Application Support/Edgar/", userHome);
		#else
		SNPRINTF(gameSavePath, sizeof(gameSavePath), "%s/.parallelrealities/edgar/", userHome);
		#endif

		SNPRINTF(tempFile, sizeof(tempFile), "%stmpsave", gameSavePath);

		SNPRINTF(saveFileIndex, sizeof(saveFileIndex), "%ssaveheader", gameSavePath);

		SNPRINTF(continueFile, sizeof(continueFile), "%scontinuesave", gameSavePath);
	}
#elif _WIN32
	void setupUserHomeDirectory()
	{
        char *userHome;
        char dir[MAX_PATH_LENGTH];
        
        userHome = getenv("appdata");
        
        if (userHome == NULL)
        {
            userHome = "";
        }
        
        SNPRINTF(dir, sizeof(dir), "%s/parallelrealities", userHome);
        
        if (mkdir(dir) != 0 && errno != EEXIST)
        {
            printf("Couldn't create required directory '%s'\n", dir);

            exit(1);
        }
        
        SNPRINTF(dir, sizeof(dir), "%s/parallelrealities/edgar", userHome);
        
        if (mkdir(dir) != 0 && errno != EEXIST)
        {
            printf("Couldn't create required directory '%s'\n", dir);

            exit(1);
        }
        
        SNPRINTF(gameSavePath, sizeof(gameSavePath), "%s/parallelrealities/edgar/", userHome);
        
		SNPRINTF(tempFile, sizeof(tempFile), "%stmpsave", gameSavePath);

		SNPRINTF(saveFileIndex, sizeof(saveFileIndex), "%ssaveheader", gameSavePath);

		SNPRINTF(continueFile, sizeof(continueFile), "%scontinuesave", gameSavePath);
	}
#else
	void setupUserHomeDirectory()
	{
        STRNCPY(gameSavePath, "", sizeof(gameSavePath));
        STRNCPY(tempFile, "tmpsave", sizeof(tempFile));
        STRNCPY(saveFileIndex, "saveheader", sizeof(saveFileIndex));
        STRNCPY(continueFile, "continuesave", sizeof(continueFile));
	}
#endif

void startOnMap(char *mapName)
{
	removeTemporaryData();

	freeGameResources();

	initGame();

	loadMap(mapName, TRUE);

	cameraSnapToTargetEntity();
}

void newGame()
{
	removeTemporaryData();

	freeGameResources();

	initGame();

	stopMusic();

	game.overrideMusic = TRUE;

	loadMap("map04", TRUE);

	runScript("intro_part1");
}

void tutorial()
{
	removeTemporaryData();

	freeGameResources();

	initGame();

	loadMap("tutorial", TRUE);

	cameraSnapToTargetEntity();
}

void titleScreen()
{
	removeTemporaryData();

	freeGameResources();

	initGame();

	game.status = IN_TITLE;
}

int loadGame(int slot)
{
	char itemName[MAX_MESSAGE_LENGTH], mapName[MAX_MESSAGE_LENGTH], backup[MAX_PATH_LENGTH];
	char saveFile[MAX_PATH_LENGTH], *line, *savePtr, completion[5];
	double version = 0;
	float percentage, steps;
	unsigned char *buffer;
	int patchGame = FALSE, foundResources;
	FILE *fp;

	savePtr = NULL;

	SNPRINTF(saveFile, sizeof(saveFile), "%ssave%d", gameSavePath, slot);

	fp = fopen(saveFile, "rb");

	if (fp == NULL)
	{
		return FALSE;
	}

	fclose(fp);

	freeGameResources();

	initGame();

	buffer = decompressFile(saveFile);

	line = strtok_r((char *)buffer, "\n", &savePtr);

	foundResources = FALSE;

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

		if (strcmpignorecase("VERSION", itemName) == 0)
		{
			sscanf(line, "%*s %s\n", itemName);

			version = atof(itemName);

			if (version > VERSION)
			{
				printf("Save file version is newer than game version. This game might not work correctly.\n");
			}
		}

		else if (strcmpignorecase("PLAY_TIME", itemName) == 0)
		{
			sscanf(line, "%*s %ld\n", &game.playTime);
		}

		else if (strcmpignorecase("PLAYER_KILLS", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.kills);
		}

		else if (strcmpignorecase("BATS_DROWNED", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.batsDrowned);
		}

		else if (strcmpignorecase("TIMES_EATEN", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.timesEaten);
		}

		else if (strcmpignorecase("DISTANCE_TRAVELLED", itemName) == 0)
		{
			sscanf(line, "%*s %u\n", &game.distanceTravelled);
		}

		else if (strcmpignorecase("ATTACKS_BLOCKED", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.attacksBlocked);
		}

		else if (strcmpignorecase("SLIME_TIME", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.timeSpentAsSlime);
		}

		else if (strcmpignorecase("ARROWS_FIRED", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.arrowsFired);
		}

		else if (strcmpignorecase("SECRETS_FOUND", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.secretsFound);
		}

		else if (strcmpignorecase("CONTINUES", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.continues);
		}

		else if (strcmpignorecase("CHEATING", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.cheating);
		}

		else if (strcmpignorecase("PLAYER_LOCATION", itemName) == 0)
		{
			if (version < VERSION)
			{
				patchGame = TRUE;

				break;
			}

			sscanf(line, "%*s %s\n", itemName);

			loadMap(itemName, FALSE);

			SNPRINTF(mapName, sizeof(mapName), "MAP_NAME %s", itemName);
		}

		else if (strcmpignorecase(line, mapName) == 0)
		{
			foundResources = TRUE;

			loadResources(savePtr);
		}

		line = strtok_r(NULL, "\n", &savePtr);
	}

	if (patchGame == TRUE)
	{
		free(buffer);

		steps = (VERSION * 100) - (version * 100);

		steps = 100 / steps;

		percentage = 0;

		version += 0.01;

		/* Back up the original save file */

		SNPRINTF(backup, sizeof(backup), "%s.bak", saveFile);

		copyFile(saveFile, backup);

		showPatchMessage("0%");

		while (TRUE)
		{
			getInput(IN_TITLE);

			patchSaveGame(saveFile, version);

			version += 0.01;

			percentage += steps;

			SNPRINTF(completion, 5, "%d%%", (int)percentage);

			if ((int)(version * 100) > (int)(VERSION * 100))
			{
				break;
			}

			showPatchMessage(completion);
		}

		return loadGame(slot);
	}

	/* Fudge to make a game saved in the new Village map still load OK */

	if (foundResources == FALSE)
	{
		sscanf(mapName, "%*s %s\n", itemName);

		loadMap(itemName, TRUE);
	}

	free(buffer);

	copyFile(saveFile, tempFile);

	buffer = decompressFile(tempFile);

	free(buffer);

	cameraSnapToTargetEntity();

	freeMessageQueue();

	temporaryDataExists = TRUE;

	return TRUE;
}

static void patchSaveGame(char *saveFile, double version)
{
	char location[MAX_VALUE_LENGTH], *line, *savePtr, itemName[MAX_MESSAGE_LENGTH], *returnedName, mapName[MAX_VALUE_LENGTH];
	unsigned char *buffer, *originalBuffer;
	int savedLocation = FALSE, saveMap;
	FILE *newSave;

	savePtr = NULL;

	freeGameResources();

	newSave = fopen(tempFile, "wb");

	buffer = decompressFile(saveFile);

	originalBuffer = malloc((strlen((char *)buffer) + 1) * sizeof(unsigned char));

	if (originalBuffer == NULL)
	{
		showErrorAndExit("Failed to allocate %d bytes to patch save game", (int)((strlen((char *)buffer) + 1) * sizeof(unsigned char)));
	}

	strcpy((char *)originalBuffer, (char *)buffer);

	line = strtok_r((char *)buffer, "\n", &savePtr);

	returnedName = NULL;

	mapName[0] = '\0';

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
			sscanf(line, "%*s %s\n", location);
		}

		else if (strcmpignorecase("MAP_NAME", itemName) == 0)
		{
			saveMap = TRUE;

			if (strlen(mapName) == 0 || strcmpignorecase(line, mapName) == 0)
			{
				sscanf(line, "%*s %s\n", itemName);

				returnedName = loadResources(savePtr);

				if (returnedName != NULL)
				{
					STRNCPY(mapName, returnedName, sizeof(mapName));
				}

				/* Rewind to start */

				free(buffer);

				buffer = malloc((strlen((char *)originalBuffer) + 1) * sizeof(unsigned char));

				if (buffer == NULL)
				{
					showErrorAndExit("Failed to allocate %d bytes to patch save game", (int)((strlen((char *)originalBuffer) + 1) * sizeof(unsigned char)));
				}

				strcpy((char *)buffer, (char *)originalBuffer);

				line = strtok_r((char *)buffer, "\n", &savePtr);

				/* Load up the patch file */

				saveMap = patchEntities(version, itemName);

				if (saveMap == FALSE)
				{
					SNPRINTF(itemName, sizeof(itemName), "%ld_old", game.playTime * SDL_GetTicks());
				}

				if (savedLocation == FALSE)
				{
					fprintf(newSave, "VERSION %0.2f\n", version);

					fprintf(newSave, "PLAY_TIME %ld\n", game.playTime);

					fprintf(newSave, "PLAYER_KILLS %d\n", game.kills);

					fprintf(newSave, "BATS_DROWNED %d\n", game.batsDrowned);

					fprintf(newSave, "TIMES_EATEN %d\n", game.timesEaten);

					fprintf(newSave, "DISTANCE_TRAVELLED %u\n", game.distanceTravelled);

					fprintf(newSave, "ATTACKS_BLOCKED %d\n", game.attacksBlocked);

					fprintf(newSave, "SLIME_TIME %d\n", game.timeSpentAsSlime);

					fprintf(newSave, "ARROWS_FIRED %d\n", game.arrowsFired);

					fprintf(newSave, "SECRETS_FOUND %d\n", game.secretsFound);

					fprintf(newSave, "CONTINUES %d\n", game.continues);

					fprintf(newSave, "CHEATING %d\n", game.cheating);

					fprintf(newSave, "PLAYER_LOCATION %s\n", location);

					savedLocation = TRUE;
				}

				fprintf(newSave, "MAP_NAME %s\n", itemName);

				if (strcmpignorecase(itemName, location) == 0)
				{
					fprintf(newSave, "PLAYER_DATA\n");

					writePlayerToFile(newSave);

					fprintf(newSave, "PLAYER_INVENTORY\n");

					writeInventoryToFile(newSave);
				}

				fprintf(newSave, "ENTITY_DATA\n");

				/* Now write out all of the Entities */

				writeEntitiesToFile(newSave);

				/* Now the targets */

				writeTargetsToFile(newSave);

				/* And the triggers */

				writeTriggersToFile(newSave);

				/* Add the global triggers */

				writeGlobalTriggersToFile(newSave);

				/* Add the map triggers */

				writeMapTriggersToFile(newSave);

				/* Add the objectives */

				writeObjectivesToFile(newSave);

				freeLevelResources();

				if (returnedName == NULL)
				{
					break;
				}
			}
		}

		line = strtok_r(NULL, "\n", &savePtr);
	}

	free(buffer);

	free(originalBuffer);

	fclose(newSave);

	#if DEV == 1
		copyFile(tempFile, "tmpdata");
	#endif

	copyFile(tempFile, saveFile);

	compressFile(saveFile);
}

void saveGame(int slot)
{
	char itemName[MAX_MESSAGE_LENGTH], *line, *savePtr;
	char saveFile[MAX_PATH_LENGTH];
	char *mapName = getMapFilename();
	char tempSaveFile[MAX_PATH_LENGTH];
	int i;
	unsigned char *buffer;
	int skipping = FALSE;
	FILE *read;
	FILE *write;

	savePtr = NULL;

	if (slot == -1)
	{
		for (i=1001;;i++)
		{
			SNPRINTF(saveFile, sizeof(saveFile), "%ssave%d", gameSavePath, i);

			read = fopen(saveFile, "rb");

			if (read == NULL)
			{
				break;
			}

			else
			{
				fclose(read);
			}
		}
	}

	else
	{
		/* Backup older save */

		SNPRINTF(saveFile, sizeof(saveFile), "%ssave%d", gameSavePath, slot);

		SNPRINTF(tempSaveFile, sizeof(tempSaveFile), "%stempsave%d", gameSavePath, slot);
	}

	write = fopen(tempSaveFile, "wb");

	fprintf(write, "VERSION %0.2f\n", VERSION);

	fprintf(write, "PLAY_TIME %ld\n", game.playTime);

	fprintf(write, "PLAYER_KILLS %d\n", game.kills);

	fprintf(write, "BATS_DROWNED %d\n", game.batsDrowned);

	fprintf(write, "TIMES_EATEN %d\n", game.timesEaten);

	fprintf(write, "DISTANCE_TRAVELLED %u\n", game.distanceTravelled);

	fprintf(write, "ATTACKS_BLOCKED %d\n", game.attacksBlocked);

	fprintf(write, "SLIME_TIME %d\n", game.timeSpentAsSlime);

	fprintf(write, "ARROWS_FIRED %d\n", game.arrowsFired);

	fprintf(write, "SECRETS_FOUND %d\n", game.secretsFound);

	fprintf(write, "CONTINUES %d\n", game.continues);

	fprintf(write, "CHEATING %d\n", game.cheating);

	fprintf(write, "PLAYER_LOCATION %s\n", mapName);

	read = fopen(tempFile, "rb");

	if (read == NULL)
	{
		if (temporaryDataExists == TRUE)
		{
			showErrorAndExit("SAVE GAME: Could not find persistance file: %s", strerror(errno));
		}
	}

	else
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

				if (strcmpignorecase("PLAYER_DATA", itemName) == 0 || strcmpignorecase("PLAYER_INVENTORY", itemName) == 0 ||
					strcmpignorecase("PLAYER_LOCATION", itemName) == 0 || strcmpignorecase("VERSION", itemName) == 0)
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

	fprintf(write, "MAP_NAME %s\n", mapName);

	fprintf(write, "PLAYER_DATA\n");

	writePlayerToFile(write);

	fprintf(write, "INVENTORY_INDEX %d\n", getInventoryIndex());

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

	/* Add the map triggers */

	writeMapTriggersToFile(write);

	/* Add the objectives */

	writeObjectivesToFile(write);

	/* Save the player data */

	fclose(write);

	#if DEV == 1
		copyFile(tempSaveFile, "savedata");
	#endif

	compressFile(tempSaveFile);

	copyFile(tempSaveFile, saveFile);

	remove(tempSaveFile);

	updateSaveFileIndex(slot);
}

static void updateSaveFileIndex(int slot)
{
	char **data = getSaveFileIndex();
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[MAX_VALUE_LENGTH], saveName[MAX_VALUE_LENGTH];
	FILE *fp;
	int i;

	if (slot == -1)
	{
		return;
	}

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, MAX_VALUE_LENGTH, "%H:%M %d %b %Y", timeinfo);

	SNPRINTF(saveName, MAX_VALUE_LENGTH, "%s - %s", _(getMapName()), buffer);

	STRNCPY(data[slot], saveName, MAX_VALUE_LENGTH);

	fp = fopen(saveFileIndex, "wb");

	if (fp == NULL)
	{
		perror("Could not update save data file. Game was still saved though.");

		return;
	}

	for (i=0;i<MAX_SAVE_SLOTS;i++)
	{
		fprintf(fp, "%s\n", data[i]);
	}

	fclose(fp);
}

int getMostRecentSave()
{
	char saveFile[MAX_PATH_LENGTH];
	struct stat fileInfo;
	int i, modTime, slot;
	FILE *fp;

	modTime = 0;

	slot = -1;

	for (i=0;i<MAX_SAVE_SLOTS;i++)
	{
		SNPRINTF(saveFile, sizeof(saveFile), "%ssave%d", gameSavePath, i);

		fp = fopen(saveFile, "rb");

		if (fp == NULL)
		{
			continue;
		}

		if (stat(saveFile, &fileInfo) != -1)
		{
			if (fileInfo.st_mtime > modTime)
			{
				modTime = fileInfo.st_mtime;

				slot = i;
			}
		}

		fclose(fp);
	}

	return slot;
}

void saveTemporaryData()
{
	char *line, itemName[MAX_MESSAGE_LENGTH], *savePtr;
	char swapFile[MAX_PATH_LENGTH];
	char *mapName = getMapFilename();
	unsigned char *buffer;
	int skipping = FALSE;
	FILE *read;
	FILE *write;

	savePtr = NULL;

	SNPRINTF(swapFile, sizeof(swapFile), "%sswap", gameSavePath);

	read = fopen(tempFile, "rb");

	write = fopen(swapFile, "wb");

	if (read == NULL)
	{
		if (temporaryDataExists == TRUE)
		{
			showErrorAndExit("SAVE TEMP DATA: Could not find persistance file: %s", strerror(errno));
		}
	}

	else
	{
		fclose(read);

		buffer = decompressFile(tempFile);

		if (strlen((char *)buffer) == 0)
		{
			showErrorAndExit("Something went wrong when decompressing the persistance file");
		}

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
					strcmpignorecase("PLAYER_LOCATION", line) == 0 || strcmpignorecase("MAP_TRIGGER", line) == 0)
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
			showErrorAndExit("Could not remove temporary file");
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
		showErrorAndExit("Could not rename temporary file");
	}

	#if DEV == 1
		copyFile(tempFile, "tmpdata");
	#endif

	compressFile(tempFile);

	temporaryDataExists = TRUE;
}

void saveContinueData()
{
	char itemName[MAX_MESSAGE_LENGTH], *line, *savePtr;
	char saveFile[MAX_PATH_LENGTH];
	char *mapName = getMapFilename();
	unsigned char *buffer;
	int skipping = FALSE;
	FILE *read;
	FILE *write;

	savePtr = NULL;

	SNPRINTF(saveFile, sizeof(saveFile), "%scontinuesave", gameSavePath);

	write = fopen(saveFile, "wb");

	fprintf(write, "VERSION %0.2f\n", VERSION);

	fprintf(write, "PLAY_TIME %ld\n", game.playTime);

	fprintf(write, "PLAYER_KILLS %d\n", game.kills);

	fprintf(write, "BATS_DROWNED %d\n", game.batsDrowned);

	fprintf(write, "TIMES_EATEN %d\n", game.timesEaten);

	fprintf(write, "DISTANCE_TRAVELLED %u\n", game.distanceTravelled);

	fprintf(write, "ATTACKS_BLOCKED %d\n", game.attacksBlocked);

	fprintf(write, "SLIME_TIME %d\n", game.timeSpentAsSlime);

	fprintf(write, "ARROWS_FIRED %d\n", game.arrowsFired);

	fprintf(write, "SECRETS_FOUND %d\n", game.secretsFound);

	fprintf(write, "CONTINUES %d\n", game.continues);

	fprintf(write, "CHEATING %d\n", game.cheating);

	fprintf(write, "PLAYER_LOCATION %s\n", mapName);

	read = fopen(tempFile, "rb");

	if (read == NULL)
	{
		if (temporaryDataExists == TRUE)
		{
			showErrorAndExit("SAVE CONTINUE DATA: Could not find persistance file: %s", strerror(errno));
		}
	}

	else
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

				if (strcmpignorecase("PLAYER_DATA", itemName) == 0 || strcmpignorecase("PLAYER_INVENTORY", itemName) == 0 ||
					strcmpignorecase("PLAYER_LOCATION", itemName) == 0 || strcmpignorecase("VERSION", itemName) == 0)
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

	fprintf(write, "MAP_NAME %s\n", mapName);

	fprintf(write, "PLAYER_DATA\n");

	writePlayerToFile(write);

	fprintf(write, "INVENTORY_INDEX %d\n", getInventoryIndex());

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

	/* Add the map triggers */

	writeMapTriggersToFile(write);

	/* Add the objectives */

	writeObjectivesToFile(write);

	/* Save the player data */

	fclose(write);

	#if DEV == 1
		copyFile(saveFile, "continuedata");
	#endif

	compressFile(saveFile);
}

int loadContinueData()
{
	char itemName[MAX_MESSAGE_LENGTH], mapName[MAX_MESSAGE_LENGTH];
	char saveFile[MAX_PATH_LENGTH], *line, *savePtr;
	unsigned char *buffer;
	FILE *fp;

	savePtr = NULL;

	SNPRINTF(saveFile, sizeof(saveFile), "%scontinuesave", gameSavePath);

	fp = fopen(saveFile, "rb");

	if (fp == NULL)
	{
		return FALSE;
	}

	fclose(fp);

	freeGameResources();

	initGame();

	game.canContinue = TRUE;

	buffer = decompressFile(saveFile);

	if (strlen((char *)buffer) == 0)
	{
		showErrorAndExit("Something went wrong when decompressing the continue file");
	}

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

		if (strcmpignorecase("PLAY_TIME", itemName) == 0)
		{
			sscanf(line, "%*s %ld\n", &game.playTime);
		}

		else if (strcmpignorecase("PLAYER_KILLS", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.kills);
		}

		else if (strcmpignorecase("BATS_DROWNED", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.batsDrowned);
		}

		else if (strcmpignorecase("TIMES_EATEN", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.timesEaten);
		}

		else if (strcmpignorecase("DISTANCE_TRAVELLED", itemName) == 0)
		{
			sscanf(line, "%*s %u\n", &game.distanceTravelled);
		}

		else if (strcmpignorecase("ATTACKS_BLOCKED", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.attacksBlocked);
		}

		else if (strcmpignorecase("SLIME_TIME", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.timeSpentAsSlime);
		}

		else if (strcmpignorecase("ARROWS_FIRED", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.arrowsFired);
		}

		else if (strcmpignorecase("SECRETS_FOUND", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.secretsFound);
		}

		else if (strcmpignorecase("CONTINUES", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.continues);
		}

		else if (strcmpignorecase("CHEATING", itemName) == 0)
		{
			sscanf(line, "%*s %d\n", &game.cheating);
		}

		else if (strcmpignorecase("PLAYER_LOCATION", itemName) == 0)
		{
			sscanf(line, "%*s %s\n", itemName);

			loadMap(itemName, FALSE);

			SNPRINTF(mapName, sizeof(mapName), "MAP_NAME %s", itemName);
		}

		else if (strcmpignorecase(line, mapName) == 0)
		{
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

	return TRUE;
}

int hasPersistance(char *mapName)
{
	int val = FALSE;
	char *line, itemName[MAX_MESSAGE_LENGTH], *savePtr;
	unsigned char *buffer;
	FILE *read;

	savePtr = NULL;

	SNPRINTF(itemName, sizeof(itemName), "MAP_NAME %s", mapName);

	read = fopen(tempFile, "rb");

	if (read == NULL)
	{
		if (temporaryDataExists == TRUE)
		{
			showErrorAndExit("HAS PERSISTANCE: Could not find persistance file: %s", strerror(errno));
		}

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

			break;
		}

		line = strtok_r(NULL, "\n", &savePtr);
	}

	free(buffer);

	return val;
}

int bossExists(char *bossName)
{
	int val = FALSE;
	char *line, *savePtr;
	unsigned char *buffer;
	FILE *read;

	savePtr = NULL;

	read = fopen(tempFile, "rb");

	if (read == NULL)
	{
		if (temporaryDataExists == TRUE)
		{
			showErrorAndExit("BOSS EXISTS: Could not find persistance file: %s", strerror(errno));
		}

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

		if (strstr(line, bossName) != NULL)
		{
			val = TRUE;

			break;
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

	savePtr = NULL;

	SNPRINTF(itemName, sizeof(itemName), "MAP_NAME %s", mapName);

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

			loadResources(savePtr);
		}

		line = strtok_r(NULL, "\n", &savePtr);
	}

	free(buffer);

	if (found == FALSE)
	{
		showErrorAndExit("Failed to find persistance data!");
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
			showErrorAndExit("Could not remove temporary file: %s", strerror(errno));
		}
	}

	fp = fopen(continueFile, "rb");

	if (fp != NULL)
	{
		fclose(fp);

		if (remove(continueFile) != 0)
		{
			showErrorAndExit("Could not remove continue file: %s", strerror(errno));
		}
	}

	temporaryDataExists = FALSE;
}

void loadConfig()
{
	FILE *fp;
	char settingsFile[MAX_PATH_LENGTH], *line, *savePtr;
	unsigned char *buffer;
	long length;

	resetGameSettings();

	game.hasConfig = FALSE;

	SNPRINTF(settingsFile, sizeof(settingsFile), "%sconfig", gameSavePath);

	resetControls(FALSE);

	fp = fopen(settingsFile, "rb");

	if (fp == NULL)
	{
		game.firstRun = TRUE;

		return;
	}

	game.firstRun = FALSE;

	fseek(fp, 0L, SEEK_END);

	length = ftell(fp);

	buffer = malloc((length + 1) * sizeof(unsigned char));

	if (buffer == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %ld bytes for config file...", length);

		exit(1);
	}

	fseek(fp, 0L, SEEK_SET);

	fread(buffer, length, 1, fp);

	buffer[length] = '\0';

	fclose(fp);

	savePtr = NULL;

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

	SNPRINTF(settingsFile, sizeof(settingsFile), "%sconfig", gameSavePath);

	fp = fopen(settingsFile, "wb");

	if (fp == NULL)
	{
		showErrorAndExit("Could not save settings: %s", strerror(errno));

		exit(1);
	}

	writeControlsToFile(fp);

	writeGameSettingsToFile(fp);

	fclose(fp);
}

static void copyFile(char *src, char *dest)
{
	int c;
	FILE *sourceFile, *destFile;

	sourceFile = fopen(src, "rb");

	if (sourceFile == NULL)
	{
		showErrorAndExit("Could not open %s for reading: %s", src, strerror(errno));
	}

	destFile = fopen(dest, "wb");

	if (destFile == NULL)
	{
		showErrorAndExit("Could not open %s for writing: %s", dest, strerror(errno));
	}

	while ((c = fgetc(sourceFile)) != EOF)
	{
		fputc(c, destFile);
	}

	fclose(sourceFile);

	fclose(destFile);
}

char *getGameSavePath()
{
	return gameSavePath;
}

char **getSaveFileIndex()
{
	int i;
	FILE *fp;
	char **entries, save0[MAX_PATH_LENGTH];

	entries = malloc(sizeof(char *) * MAX_SAVE_SLOTS);

	if (entries == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for save data", (int)sizeof(char *) * MAX_SAVE_SLOTS);

		exit(1);
	}

	for (i=0;i<MAX_SAVE_SLOTS;i++)
	{
		entries[i] = malloc(sizeof(char) * MAX_PATH_LENGTH);

		if (entries[i] == NULL)
		{
			showErrorAndExit("Failed to allocate a whole %d bytes for save data", (int)sizeof(char) * MAX_PATH_LENGTH);

			exit(1);
		}

		entries[i][0] = '\0';
	}

	fp = fopen(saveFileIndex, "rb");

	if (fp == NULL)
	{
		/* Check for a save game from version 0.1 */

		SNPRINTF(save0, sizeof(save0), "%ssave0", gameSavePath);

		fp = fopen(save0, "rb");

		if (fp != NULL)
		{
			fclose(fp);

			STRNCPY(entries[0], "Saved Game", MAX_PATH_LENGTH);
		}

		return entries;
	}

	for (i=0;i<MAX_SAVE_SLOTS;i++)
	{
		if (fgets(entries[i], MAX_PATH_LENGTH, fp) == NULL)
		{
			entries[i][0] = '\0';
		}

		else
		{
			entries[i][strlen(entries[i]) - 1] = '\0';
		}
	}

	fclose(fp);

	return entries;
}

static void showPatchMessage(char *message)
{
	int y;
	SDL_Rect dest;
	Texture *title, *completion;
	SDL_Surface *text;

	text = generateTextSurface(_("Patching your save game. Please wait..."), game.font, 220, 220, 220, 0, 0, 0);
	
	title = convertSurfaceToTexture(text, TRUE);

	text = generateTextSurface(message, game.font, 220, 220, 220, 0, 0, 0);
	
	completion = convertSurfaceToTexture(text, TRUE);

	if (title == NULL || completion == NULL)
	{
		return;
	}

	clearScreen(0, 0, 0);

	y = (SCREEN_HEIGHT - title->h) / 2;

	dest.x = (SCREEN_WIDTH - title->w) / 2;
	dest.y = y;
	dest.w = title->w;
	dest.h = title->h;
	
	SDL_RenderCopy(game.renderer, title->texture, NULL, &dest);

	y += title->h + 15;

	dest.x = (SCREEN_WIDTH - completion->w) / 2;
	dest.y = y;
	dest.w = completion->w;
	dest.h = completion->h;
	
	SDL_RenderCopy(game.renderer, completion->texture, NULL, &dest);

	destroyTexture(title);

	destroyTexture(completion);

	/* Swap the buffers */

	SDL_RenderPresent(game.renderer);
}

void loadObtainedMedals()
{
	char medalFile[MAX_PATH_LENGTH], *line, *savePtr;
	unsigned char *buffer;
	FILE *fp;

	savePtr = NULL;

	SNPRINTF(medalFile, MAX_PATH_LENGTH, "%smedals", gameSavePath);

	fp = fopen(medalFile, "rb");

	if (fp == NULL)
	{
		return;
	}

	fclose(fp);

	buffer = decompressFile(medalFile);

	line = strtok_r((char *)buffer, "\n", &savePtr);

	while (line != NULL)
	{
		setObtainedMedal(line);

		line = strtok_r(NULL, "\n", &savePtr);
	}

	free(buffer);
}

void saveObtainedMedals()
{
	int i, medalCount;
	char medalFile[MAX_PATH_LENGTH];
	Medal *medal;
	FILE *fp;

	SNPRINTF(medalFile, MAX_PATH_LENGTH, "%smedals", gameSavePath);

	fp = fopen(medalFile, "wb");

	if (fp == NULL)
	{
		showErrorAndExit("Could not save Medal data: %s", strerror(errno));
	}

	medalCount = getMedalCount();

	medal = getMedals();

	for (i=0;i<medalCount;i++)
	{
		if (medal[i].obtained == TRUE)
		{
			fprintf(fp, "%s\n", medal[i].code);
		}
	}

	fclose(fp);

	#if DEV == 1
		copyFile(medalFile, "medaldata");
	#endif

	compressFile(medalFile);
}
