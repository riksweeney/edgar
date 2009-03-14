#include "../headers.h"

#include "../event/global_trigger.h"
#include "../event/objective.h"
#include "../entity.h"
#include "script.h"
#include "../dialog.h"
#include "../player.h"
#include "../inventory.h"

extern Entity player;

static Script script;

void loadScript(char *name)
{
	char filename[MAX_PATH_LENGTH];
	int i;
	FILE *fp;

	freeScript();

	script.lineCount = 0;

	snprintf(filename, sizeof(filename), "data/scripts/%s.dat", name);

	printf("Loading script file %s\n", filename);

	fp = fopen(filename, "rb");

	if (fp == NULL)
	{
		printf("Failed to open script file %s\n", filename);

		exit(1);
	}

	while (fgets(filename, MAX_PATH_LENGTH, fp) != NULL)
	{
		script.lineCount++;
	}

	script.text = (char **)malloc(sizeof(char *) * script.lineCount);

	if (script.text == NULL)
	{
		printf("Could not allocate a whole %d bytes for script %s\n", sizeof(char *) * script.lineCount, filename);

		exit(1);
	}

	fseek(fp, 0L, SEEK_SET);

	script.line = 0;

	i = 0;

	while (fgets(filename, MAX_PATH_LENGTH, fp) != NULL)
	{
		if (filename[strlen(filename) - 1] == '\n')
		{
			filename[strlen(filename) - 1] = '\0';
		}

		script.text[i] = (char *)malloc(strlen(filename));

		if (script.text[i] == NULL)
		{
			printf("Could not allocate %d bytes for script line %d\n", sizeof(char *) * script.lineCount, (i + 1));
		}

		STRNCPY(script.text[i], filename, strlen(filename));

		i++;
	}

	fclose(fp);

	script.skipping = FALSE;

	playerWaitForDialog();
}

void readNextScriptLine()
{
	char *token, line[MAX_LINE_LENGTH];
	int readAgain = TRUE;
	Entity *e;

	while (readAgain == TRUE)
	{
		if (script.line == script.lineCount)
		{
			freeScript();

			freeDialogBox();

			playerResumeNormal();

			return;
		}

		STRNCPY(line, script.text[script.line], sizeof(line));

		token = strtok(line, " ");

		if (script.skipping == TRUE)
		{
			if (strcmpignorecase("END", token) == 0)
			{
				script.skipping = FALSE;
			}
		}

		else if (strcmpignorecase("TALK", token) == 0)
		{
			createDialogBoxFromScript(script.text[script.line]);

			readAgain = FALSE;
		}

		else if (strcmpignorecase("ADD", token) == 0)
		{
			token = strtok(NULL, " ");

			if (strcmpignorecase("ENTITY", token) == 0)
			{
				token = strtok(NULL, "\0");

				addEntityFromScript(token);
			}

			else if (strcmpignorecase("OBJECTIVE", token) == 0)
			{
				token = strtok(NULL, "\0");

				addObjectiveFromScript(token);
			}

			else if (strcmpignorecase("TRIGGER", token) == 0)
			{
				token = strtok(NULL, "\0");

				addGlobalTriggerFromScript(token);
			}
		}

		else if (strcmpignorecase("IF", token) == 0)
		{
			token = strtok(NULL, " ");

			if (strcmpignorecase(token, "EDGAR") == 0)
			{
				e = &player;
			}

			else
			{
				e = getEntityByObjectiveName(token);
			}

			if (e == NULL)
			{
				printf("IF command could not find Entity %s\n", token);

				exit(1);
			}

			token = strtok(NULL, " ");

			if (strcmpignorecase(token, "HEALTH") == 0)
			{
				token = strtok(NULL, " ");

				if (e->health != atoi(token))
				{
					script.skipping = TRUE;
				}
			}
		}

		else if (strcmpignorecase("SET", token) == 0)
		{
			token = strtok(NULL, " ");

			if (strcmpignorecase(token, "EDGAR") == 0)
			{
				e = &player;
			}

			else
			{
				e = getEntityByObjectiveName(token);
			}

			if (e == NULL)
			{
				printf("SET command could not find Entity %s\n", token);

				exit(1);
			}

			token = strtok(NULL, " ");

			if (strcmpignorecase(token, "HEALTH") == 0)
			{
				token = strtok(NULL, " ");

				e->health = atoi(token);
			}
		}

		else if (strcmpignorecase("REMOVE", token) == 0)
		{
			token = strtok(NULL, "\0");

			getInventoryItemFromScript(token);
		}

		else if (strcmpignorecase("WALK", token) == 0)
		{
			token = strtok(NULL, " ");

			if (strcmpignorecase(token, "EDGAR") == 0)
			{
				e = &player;
			}

			else
			{
				e = getEntityByObjectiveName(token);
			}

			if (e == NULL)
			{
				printf("WALK command could not find Entity %s\n", token);

				exit(1);
			}
		}

		script.line++;
	}
}

void freeScript()
{
	int i;

	if (script.text != NULL)
	{
		for (i=0;i<script.lineCount;i++)
		{
			free(script.text[i]);
		}

		free(script.text);

		script.text = NULL;
	}
}
