#include "headers.h"

#include "entity.h"
#include "trigger.h"
#include "target.h"
#include "map.h"
#include "player.h"
#include "inventory.h"
#include "compress.h"

static char gameSavePath[MAX_PATH_LENGTH];

#ifdef UNIX
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
}
#else
void setupUserHomeDirectory()
{
	strcpy(gameSavePath, "");
}
#endif

void saveGame()
{
	char line[MAX_LINE_LENGTH], itemName[MAX_MESSAGE_LENGTH];
	char tmpsaveucpx[MAX_LINE_LENGTH], tmpsave[MAX_LINE_LENGTH], tmpsavetmp[MAX_LINE_LENGTH];
	char *mapName = getMapName();
	int skipping = FALSE;
	FILE *read;
	FILE *write;
	
	sprintf(tmpsaveucpx, "%stmpsaveucpx", gameSavePath);
	sprintf(tmpsave, "%stmpsave", gameSavePath);
	sprintf(tmpsavetmp, "%stmpsave.tmp", gameSavePath);
	
	read = fopen(tmpsave, "rb");
	
	write = fopen(tmpsaveucpx, "wb");
	
	if (read != NULL)
	{
		if (decompressFile(read, write) != Z_OK)
		{
			printf("Failed to decompress temporary file\n");
			
			exit(1);
		}
		
		fclose(read);
		
		if (remove(tmpsave) != 0)
		{
			perror("Could not delete temporary file");
			
			exit(1);
		}
	}
	
	fclose(write);
	
	read = fopen(tmpsaveucpx, "rb");
	
	write = fopen(tmpsavetmp, "wb");
	
	/* Save the player's position */
	
	fprintf(write, "PLAYER DATA\n");
	
	writePlayerToFile(write);
	
	fprintf(write, "PLAYER INVENTORY\n");
	
	writeInventoryToFile(write);
	
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
				
				if (strcmpignorecase("PLAYER DATA", line) == 0 || strcmpignorecase("PLAYER INVENTORY", line) == 0)
				{
					skipping = TRUE;
				}
				
				else if (strcmpignorecase("MAP", itemName) == 0)
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
				
				if (strcmpignorecase("MAP", itemName) == 0)
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
		
		if (remove(tmpsaveucpx) != 0)
		{
			perror("Could not delete temporary file");
			
			exit(1);
		}
	}
	
	fprintf(write, "MAP %s\n", mapName);
	
	/* Now write out all of the Entities */

	writeEntitiesToFile(write);

	/* Now the targets */

	writeTargetsToFile(write);

	/* And the triggers */

	writeTriggersToFile(write);
	
	fclose(write);
	
	read = fopen(tmpsavetmp, "rb");
	
	write = fopen(tmpsave, "wb");
	
	compressFile(read, write);
	
	fclose(read);
	fclose(write);
	
	if (remove(tmpsavetmp) != 0)
	{
		perror("Could not delete temporary file");
		
		exit(1);
	}
}

void saveTemporaryData()
{
	char line[MAX_LINE_LENGTH], itemName[MAX_MESSAGE_LENGTH];
	char tmpsaveucpx[MAX_LINE_LENGTH], tmpsave[MAX_LINE_LENGTH], tmpsavetmp[MAX_LINE_LENGTH];
	char *mapName = getMapName();
	int skipping = FALSE;
	FILE *read;
	FILE *write;
	
	sprintf(tmpsaveucpx, "%stmpsaveucpx", gameSavePath);
	sprintf(tmpsave, "%stmpsave", gameSavePath);
	sprintf(tmpsavetmp, "%stmpsave.tmp", gameSavePath);
	
	read = fopen(tmpsave, "rb");
	
	write = fopen(tmpsaveucpx, "wb");
	
	if (read != NULL)
	{
		if (decompressFile(read, write) != Z_OK)
		{
			printf("Failed to decompress temporary file\n");
			
			exit(1);
		}
		
		fclose(read);
		
		if (remove(tmpsave) != 0)
		{
			perror("Could not delete temporary file");
			
			exit(1);
		}
	}
	
	fclose(write);
	
	read = fopen(tmpsaveucpx, "rb");
	
	write = fopen(tmpsavetmp, "wb");
	
	/* Save the player's position */
	
	fprintf(write, "PLAYER DATA\n");
	
	writePlayerToFile(write);
	
	fprintf(write, "PLAYER INVENTORY\n");
	
	writeInventoryToFile(write);
	
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
				
				if (strcmpignorecase("PLAYER DATA", line) == 0 || strcmpignorecase("PLAYER INVENTORY", line) == 0)
				{
					skipping = TRUE;
				}
				
				else if (strcmpignorecase("MAP", itemName) == 0)
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
				
				if (strcmpignorecase("MAP", itemName) == 0)
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
		
		if (remove(tmpsaveucpx) != 0)
		{
			perror("Could not delete temporary file");
			
			exit(1);
		}
	}
	
	fprintf(write, "MAP %s\n", mapName);
	
	/* Now write out all of the Entities */

	writeEntitiesToFile(write);

	/* Now the targets */

	writeTargetsToFile(write);

	/* And the triggers */

	writeTriggersToFile(write);
	
	fclose(write);
	
	read = fopen(tmpsavetmp, "rb");
	
	write = fopen(tmpsave, "wb");
	
	compressFile(read, write);
	
	fclose(read);
	fclose(write);
	
	if (remove(tmpsavetmp) != 0)
	{
		perror("Could not delete temporary file");
		
		exit(1);
	}
}
