#include "headers.h"

#include "entity.h"
#include "trigger.h"
#include "global_trigger.h"
#include "objective.h"
#include "target.h"
#include "map.h"
#include "player.h"
#include "inventory.h"
#include "compress.h"
#include "resources.h"
#include "load_save.h"

static char gameSavePath[MAX_PATH_LENGTH];

static void removeTemporaryData(void);

#ifndef WIN32
	void setupUserHomeDirectory()
	{
		char *userHome;
		char *name;
		char dir[MAX_PATH_LENGTH];
		struct passwd *pass;

		name = getlogin();

		strcpy(dir, "");

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

		printf("User Home = %s\n", userHome);

		sprintf(dir, "%s/.parallelrealities", userHome);

		if ((mkdir(dir, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) != 0) && (errno != EEXIST))
		{
			printf("Couldn't create required directory '%s'", dir);

			exit(1);
		}

		sprintf(dir, "%s/.parallelrealities/edgar", userHome);

		if ((mkdir(dir, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) != 0) && (errno != EEXIST))
		{
			printf("Couldn't create required directory '%s'", dir);

			exit(1);
		}

		sprintf(gameSavePath, "%s/.parallelrealities/edgar/", userHome);

		removeTemporaryData();
	}
#else
	void setupUserHomeDirectory()
	{
		strcpy(gameSavePath, "");

		removeTemporaryData();
	}
#endif

void loadGame(int slot)
{
	char itemName[MAX_MESSAGE_LENGTH], mapName[MAX_MESSAGE_LENGTH];
	char saveFile[MAX_PATH_LENGTH], line[MAX_LINE_LENGTH];
	FILE *read;

	freeGameResources();

	sprintf(saveFile, "%ssave%d", gameSavePath, slot);

	read = fopen(saveFile, "rb");

	printf("Reading save data\n");

	while (fgets(line, MAX_LINE_LENGTH, read) != NULL)
	{
		if (line[strlen(line) - 1] == '\n')
		{
			line[strlen(line) - 1] = '\0';
		}

		sscanf(line, "%s", itemName);

		if (strcmpignorecase("PLAYER_LOCATION", itemName) == 0)
		{
			sscanf(line, "%*s %s\n", itemName);

			printf("Loading save location %s\n", itemName);

			loadMap(itemName, FALSE);

			sprintf(mapName, "MAP_NAME %s", itemName);
		}

		else if (strcmpignorecase(line, mapName) == 0)
		{
			printf("Loading entities for map %s\n", mapName);

			loadResources(read);
		}
	}

	fclose(read);

	removeTemporaryData();

	saveTemporaryData();

	printf("Load completed\n");
}

void saveGame(int slot)
{
	char line[MAX_LINE_LENGTH], itemName[MAX_MESSAGE_LENGTH];
	char saveFile[MAX_PATH_LENGTH], tempfile[MAX_PATH_LENGTH];
	char *mapName = getMapName();
	int skipping = FALSE;
	FILE *read;
	FILE *write;

	printf("Saving game\n");

	sprintf(tempfile, "%stmpsave", gameSavePath);
	sprintf(saveFile, "%ssave%d", gameSavePath, slot);

	read = fopen(tempfile, "rb");

	write = fopen(saveFile, "wb");

	/* Save the player's position */

	printf("Copying persisting data\n");

	if (read != NULL)
	{
		while (fgets(line, MAX_LINE_LENGTH, read) != NULL)
		{
			if (line[strlen(line) - 1] == '\n')
			{
				line[strlen(line) - 1] = '\0';
			}

			if (skipping == FALSE)
			{
				sscanf(line, "%s", itemName);

				if (strcmpignorecase("PLAYER_DATA", line) == 0 || strcmpignorecase("PLAYER_INVENTORY", line) == 0 || strcmpignorecase("PLAYER_LOCATION", line) == 0)
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
		}

		fclose(read);
	}

	printf("Writing player location\n");

	fprintf(write, "PLAYER_LOCATION %s\n", mapName);

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
}

void saveTemporaryData()
{
	char line[MAX_LINE_LENGTH], itemName[MAX_MESSAGE_LENGTH];
	char tempFile[MAX_PATH_LENGTH], swapFile[MAX_PATH_LENGTH];
	char *mapName = getMapName();
	int skipping = FALSE;
	FILE *read;
	FILE *write;

	sprintf(tempFile, "%stmpsave", gameSavePath);
	sprintf(swapFile, "%sswap", gameSavePath);

	read = fopen(tempFile, "rb");

	write = fopen(swapFile, "wb");

	if (read != NULL)
	{
		while (fgets(line, MAX_LINE_LENGTH, read) != NULL)
		{
			if (line[strlen(line) - 1] == '\n')
			{
				line[strlen(line) - 1] = '\0';
			}

			if (skipping == FALSE)
			{
				sscanf(line, "%s", itemName);

				if (strcmpignorecase("PLAYER_DATA", line) == 0 || strcmpignorecase("PLAYER_INVENTORY", line) == 0 ||
					strcmpignorecase("GLOBAL_TRIGGER", line) == 0 || strcmpignorecase("OBJECTIVE", line) == 0)
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
		}

		fclose(read);

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
}

int hasPersistance(char *mapName)
{
	int val = FALSE;
	char line[MAX_LINE_LENGTH], itemName[MAX_MESSAGE_LENGTH], tempFile[MAX_PATH_LENGTH];
	FILE *read;

	sprintf(itemName, "MAP_NAME %s", mapName);

	sprintf(tempFile, "%stmpsave", gameSavePath);

	read = fopen(tempFile, "rb");

	if (read == NULL)
	{
		return val;
	}

	while (fgets(line, MAX_LINE_LENGTH, read) != NULL)
	{
		if (line[strlen(line) - 1] == '\n')
		{
			line[strlen(line) - 1] = '\0';
		}

		if (strcmpignorecase(line, itemName) == 0)
		{
			val = TRUE;
		}
	}

	fclose(read);

	return val;
}

void loadPersitanceData(char *mapName)
{
	char line[MAX_LINE_LENGTH], itemName[MAX_MESSAGE_LENGTH], tempFile[MAX_PATH_LENGTH];
	int found = FALSE;
	FILE *read;

	sprintf(itemName, "MAP_NAME %s", mapName);

	sprintf(tempFile, "%stmpsave", gameSavePath);

	read = fopen(tempFile, "rb");

	if (read == NULL)
	{
		perror("Persistance file wasn't found!");
	}

	while (fgets(line, MAX_LINE_LENGTH, read) != NULL)
	{
		if (line[strlen(line) - 1] == '\n')
		{
			line[strlen(line) - 1] = '\0';
		}

		if (strcmpignorecase(line, itemName) == 0)
		{
			found = TRUE;

			loadResources(read);
		}
	}

	fclose(read);

	if (found == FALSE)
	{
		printf("Failed to find persistance data!\n");

		exit(1);
	}
}

static void removeTemporaryData()
{
	FILE *fp;
	char tempFile[MAX_PATH_LENGTH];

	sprintf(tempFile, "%stmpsave", gameSavePath);

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
