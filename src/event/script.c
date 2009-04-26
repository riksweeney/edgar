#include "../headers.h"

#include "../event/global_trigger.h"
#include "../event/objective.h"
#include "../entity.h"
#include "script.h"
#include "../dialog.h"
#include "../player.h"
#include "../inventory.h"
#include "../audio/audio.h"
#include "../decoration.h"
#include "../graphics/animation.h"
#include "../game.h"
#include "../map.h"

extern Entity player, *self;

static Script script;

void loadScript(char *name)
{
	char filename[MAX_PATH_LENGTH];
	int i;
	FILE *fp;

	freeScript();

	script.lineCount = 0;

	snprintf(filename, sizeof(filename), _("data/scripts/%s.dat"), name);

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
		
		if (filename[strlen(filename) - 1] == '\r')
		{
			filename[strlen(filename) - 1] = '\0';
		}

		script.text[i] = (char *)malloc(strlen(filename) + 1);

		if (script.text[i] == NULL)
		{
			printf("Could not allocate %d bytes for script line %d\n", sizeof(char *) * script.lineCount, (i + 1));
		}

		STRNCPY(script.text[i], filename, strlen(filename) + 1);

		i++;
	}

	fclose(fp);

	script.skipping = FALSE;

	playerWaitForDialog();
}

void readNextScriptLine()
{
	char *token, line[MAX_LINE_LENGTH], command[MAX_VALUE_LENGTH];
	int readAgain = TRUE;
	Entity *e, *e2;

	while (readAgain == TRUE)
	{
		if (script.line == script.lineCount)
		{
			freeScript();

			freeDialogBox();

			playerResumeNormal();

			return;
		}

		else if (script.thinkTime != 0)
		{
			script.thinkTime--;

			if (script.thinkTime != 0)
			{
				return;
			}
		}

		else if (script.counter != 0)
		{
			return;
		}

		STRNCPY(line, script.text[script.line], sizeof(line));

		token = strtok(line, " ");

		STRNCPY(command, token, sizeof(command));

		if (script.skipping == TRUE)
		{
			if (strcmpignorecase("END", command) == 0)
			{
				script.skipping = FALSE;
			}
		}

		else if (strcmpignorecase("TALK", command) == 0)
		{
			createDialogBoxFromScript(script.text[script.line]);

			readAgain = FALSE;
		}

		else if (strcmpignorecase("ADD", command) == 0)
		{
			token = strtok(NULL, " ");

			if (strcmpignorecase("ENTITY", token) == 0)
			{
				token = strtok(NULL, "\0");

				addEntityFromScript(token);
			}

			else if (strcmpignorecase("DECORATION", token) == 0)
			{
				token = strtok(NULL, "\0");

				addDecorationFromScript(token);
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

		else if (strcmpignorecase("IF", command) == 0)
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

				if (strcmpignorecase(token, "NOT_MAX") == 0)
				{
					if (e->health == e->maxHealth)
					{
						script.skipping = TRUE;
					}
				}

				else if (strcmpignorecase(token, "MAX") == 0)
				{
					if (e->health != e->maxHealth)
					{
						script.skipping = TRUE;
					}
				}

				else if (e->health != atoi(token))
				{
					script.skipping = TRUE;
				}
			}

			else
			{
				printf("Unknown IF command %s\n",token);

				exit(1);
			}
		}

		else if (strcmpignorecase("SET", command) == 0)
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

				if (strcmpignorecase(token, "MAX") == 0)
				{
					e->health = e->maxHealth;
				}

				else
				{
					e->health = atoi(token);
				}
			}

			else if (strcmpignorecase("FACE", token) == 0)
			{
				token = strtok(NULL, " ");

				if (strcmpignorecase(token, "LEFT") == 0)
				{
					e->face = LEFT;
				}

				else if (strcmpignorecase(token, "RIGHT") == 0)
				{
					e->face = RIGHT;
				}

				else
				{
					if (strcmpignorecase(token, "EDGAR") == 0)
					{
						e2 = &player;
					}

					else
					{
						e2 = getEntityByObjectiveName(token);
					}

					if (e2 == NULL)
					{
						printf("FACE command could not find Entity \"%s\"\n", token);

						exit(1);
					}

					e->face = (e->x < e2->x ? RIGHT : LEFT);
				}
				
				if (e == &player)
				{
					syncWeaponShieldToPlayer();
				}
			}

			else if (strcmpignorecase(token, "ANIMATION") == 0)
			{
				token = strtok(NULL, " ");

				setEntityAnimation(e, getAnimationTypeByName(token));
			}

			else
			{
				printf("Unknown SET command %s\n", token);

				exit(1);
			}
		}

		else if (strcmpignorecase("ACTIVATE", command) == 0)
		{
			token = strtok(NULL, "\0");

			activateEntitiesWithName(token, TRUE);
		}

		else if (strcmpignorecase("DEACTIVATE", command) == 0)
		{
			token = strtok(NULL, "\0");

			activateEntitiesWithName(token, FALSE);
		}

		else if (strcmpignorecase("LOAD_LEVEL", command) == 0)
		{
			token = strtok(NULL, "\0");

			setNextLevelFromScript(token);
		}

		else if (strcmpignorecase("REMOVE", command) == 0 || strcmpignorecase("HAS_ITEM", command) == 0)
		{
			getInventoryItemFromScript(script.text[script.line]);
		}

		else if (strcmpignorecase("WAIT", command) == 0)
		{
			freeDialogBox();

			token = strtok(NULL, "\0");

			script.thinkTime = atoi(token);
		}

		else if (strcmpignorecase("PLAY_SOUND", command) == 0)
		{
			token = strtok(NULL, "\0");

			playSound(token, OBJECT_CHANNEL_1, OBJECT_CHANNEL_2, player.x, player.y);
		}

		else if (strcmpignorecase("KILL", command) == 0)
		{
			token = strtok(NULL, "\0");

			e = getEntityByObjectiveName(token);

			if (e == NULL)
			{
				printf("KILL command could not find Entity %s\n", token);

				exit(1);
			}
			
			if (e->die != NULL)
			{
				self = e;
				
				self->die();
			}
			
			else
			{
				e->inUse = FALSE;
			}
		}

		else if (strcmpignorecase("FOLLOW", command) == 0)
		{
			freeDialogBox();

			token = strtok(NULL, "\0");

			if (strcmpignorecase("NONE", token) == 0)
			{
				e = NULL;
			}

			else if (strcmpignorecase(token, "EDGAR") == 0)
			{
				e = &player;
			}

			else
			{
				e = getEntityByObjectiveName(token);

				if (e == NULL)
				{
					printf("FOLLOW command could not find Entity %s\n", token);

					exit(1);
				}
			}

			centerMapOnEntity(e);
		}

		else if (strcmpignorecase("WALK_TO", command) == 0 || strcmpignorecase("WALK_TO_RELATIVE", command) == 0)
		{
			freeDialogBox();

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
				printf("WALK_TO command could not find Entity %s\n", token);

				exit(1);
			}

			token = strtok(NULL, "\0");

			printf("%s %s %s\n", command, e->objectiveName, token);

			if (strcmpignorecase("WALK_TO", command) == 0)
			{
				entityWalkTo(e, token);
			}

			else
			{
				entityWalkToRelative(e, token);
			}
		}

		script.line++;
	}
}

int scriptWaiting()
{
	return (script.thinkTime != 0 || script.counter != 0);
}

void setScriptCounter(int value)
{
	script.counter += value;

	if (script.counter == 0)
	{
		readNextScriptLine();
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
