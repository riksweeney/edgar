#include "headers.h"

#include "trigger.h"
#include "objective.h"
#include "entity.h"
#include "script.h"
#include "dialog.h"
#include "player.h"

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

	playerWaitForDialog();
}

void readNextScriptLine()
{
	char command[10];
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
		
		sscanf(script.text[script.line], "%s", command);
	
		if (strcmpignorecase("TALK", command) == 0)
		{
			createDialogBoxFromScript(script.text[script.line]);
			
			readAgain = FALSE;
		}
	
		else if (strcmpignorecase("ADD", command) == 0)
		{
			sscanf(script.text[script.line], "%*s %s", command);
	
			printf("Adding %s\n", command);
	
			if (strcmpignorecase("ENTITY", command) == 0)
			{
				addEntityFromScript(script.text[script.line]);
			}
	
			else if (strcmpignorecase("OBJECTIVE", command) == 0)
			{
				addObjectiveFromScript(script.text[script.line]);
			}
	
			else if (strcmpignorecase("TRIGGER", command) == 0)
			{
				addTriggerFromScript(script.text[script.line]);
			}
		}
	
		else if (strcmpignorecase("WALK", command) == 0)
		{
			sscanf(script.text[script.line], "%*s %s", command);
	
			if (strcmpignorecase(command, "EDGAR") == 0)
			{
				e = &player;
			}
	
			else
			{
				e = getEntityByObjectiveName(command);
			}
	
			if (e == NULL)
			{
				printf("WALK command could not find Entity %s\n", command);
	
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
