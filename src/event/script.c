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

#include "../event/global_trigger.h"
#include "../event/map_trigger.h"
#include "../event/objective.h"
#include "../entity.h"
#include "script.h"
#include "../dialog.h"
#include "../player.h"
#include "../inventory.h"
#include "../audio/audio.h"
#include "../audio/music.h"
#include "../graphics/decoration.h"
#include "../graphics/animation.h"
#include "../game.h"
#include "../map.h"
#include "../system/properties.h"
#include "../system/pak.h"
#include "../custom_actions.h"
#include "../system/random.h"

extern Entity player, *self;
extern Game game;

static Script script;

void runScript(char *name)
{
	char filename[MAX_PATH_LENGTH], *line, *text, *savePtr;
	unsigned char *buffer;
	int i;

	savePtr = NULL;

	freeScript();

	script.lineCount = 0;

	snprintf(filename, sizeof(filename), "data/scripts/%s.dat", name);

	printf("Loading script file %s\n", filename);

	buffer = loadFileFromPak(filename);

	text = (char *)malloc((strlen((char *)buffer) + 1) * sizeof(char));

	if (text == NULL)
	{
		printf("Failed to allocate a whole %d bytes for script %s\n", (int)(sizeof(char *) * (strlen((char *)buffer) + 1)), filename);

		exit(1);
	}

	STRNCPY(text, (char *)buffer, strlen((char *)buffer) + 1);

	line = strtok_r((char *)text, "\n", &savePtr);

	while (line != NULL)
	{
		script.lineCount++;

		line = strtok_r(NULL, "\n", &savePtr);
	}

	free(text);

	script.text = (char **)malloc(sizeof(char *) * script.lineCount);

	if (script.text == NULL)
	{
		printf("Failed to allocate a whole %d bytes for script %s\n", (int)(sizeof(char *) * script.lineCount), filename);

		exit(1);
	}

	script.line = 0;

	i = 0;

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

		script.text[i] = (char *)malloc(strlen(line) + 1);

		if (script.text[i] == NULL)
		{
			printf("Failed to allocate %d bytes for script line %d\n", (int)(sizeof(char *) * script.lineCount), (i + 1));
		}

		STRNCPY(script.text[i], line, strlen(line) + 1);

		i++;

		line = strtok_r(NULL, "\n", &savePtr);
	}

	free(buffer);

	script.skipping = FALSE;

	playerWaitForDialog();

	readNextScriptLine();
}

void readNextScriptLine()
{
	char *token, line[MAX_LINE_LENGTH], command[MAX_VALUE_LENGTH], *savePtr;
	int readAgain = TRUE;
	Entity *e, *e2;

	while (readAgain == TRUE)
	{
		if (script.counter != 0)
		{
			return;
		}

		else if (script.line == script.lineCount)
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

		STRNCPY(line, script.text[script.line], sizeof(line));

		token = strtok_r(line, " ", &savePtr);

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
			token = strtok_r(NULL, " ", &savePtr);

			if (strcmpignorecase("ENTITY", token) == 0)
			{
				token = strtok_r(NULL, "\0", &savePtr);

				addEntityFromScript(token);
			}

			else if (strcmpignorecase("DECORATION", token) == 0)
			{
				token = strtok_r(NULL, "\0", &savePtr);

				addDecorationFromScript(token);
			}

			else if (strcmpignorecase("OBJECTIVE", token) == 0)
			{
				token = strtok_r(NULL, "\0", &savePtr);

				addObjectiveFromScript(token);
			}

			else if (strcmpignorecase("TRIGGER", token) == 0)
			{
				token = strtok_r(NULL, "\0", &savePtr);

				addGlobalTriggerFromScript(token);
			}

			else if (strcmpignorecase("MAP_TRIGGER", token) == 0)
			{
				token = strtok_r(NULL, "\0", &savePtr);

				addMapTriggerFromScript(token);
			}

			else
			{
				printf("ADD command encountered unknown action %s\n", token);

				exit(1);
			}
		}

		else if (strcmpignorecase("IF", command) == 0)
		{
			token = strtok_r(NULL, " ", &savePtr);

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

			token = strtok_r(NULL, " ", &savePtr);

			if (strcmpignorecase(token, "HEALTH") == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

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

				else if (strcmpignorecase(token, "NOT") == 0)
				{
					token = strtok_r(NULL, " ", &savePtr);

					if (e->health == atoi(token))
					{
						script.skipping = TRUE;
					}
				}

				else if (e->health != atoi(token))
				{
					script.skipping = TRUE;
				}
			}

			else if (strcmpignorecase(token, "MAX_HEALTH") == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				if (e->maxHealth != atoi(token))
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
			token = strtok_r(NULL, " ", &savePtr);

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

			token = strtok_r(NULL, " ", &savePtr);

			if (strcmpignorecase(token, "HEALTH") == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				if (strcmpignorecase(token, "MAX") == 0)
				{
					e->health = e->maxHealth;
				}
				
				else if (strcmpignorecase(token, "RANDOM") == 0)
				{
					token = strtok_r(NULL, " ", &savePtr);
					
					e->health = (prand() % atoi(token)) + 1;
				}

				else
				{
					e->health = atoi(token);
				}
			}

			else if (strcmpignorecase("ADD_FLAG", token) == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				setFlags(e, token);
			}

			else if (strcmpignorecase("REMOVE_FLAG", token) == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				unsetFlags(e, token);
			}

			else if (strcmpignorecase("FACE", token) == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

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
				token = strtok_r(NULL, " ", &savePtr);

				setEntityAnimation(e, getAnimationTypeByName(token));
			}

			else if (strcmpignorecase(token, "SPEED") == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				e->speed = atof(token);
			}

			else if (strcmpignorecase(token, "DIR_X") == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				e->dirX = atoi(token);
			}

			else if (strcmpignorecase(token, "DIR_Y") == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				e->dirY = atoi(token);
			}

			else if (strcmpignorecase(token, "FRAME_SPEED") == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				e->frameSpeed = atof(token);
			}

			else if (strcmpignorecase(token, "RESUME_NORMAL_FUNCTION") == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				e->action = e->resumeNormalFunction;
			}

			else
			{
				printf("Unknown SET command %s\n", token);

				exit(1);
			}
		}

		else if (strcmpignorecase("ACTIVATE_REQUIRED", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			printf("Script is activating %s\n", token);

			activateEntitiesWithRequiredName(token, TRUE);
		}

		else if (strcmpignorecase("DEACTIVATE_REQUIRED", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			activateEntitiesWithRequiredName(token, FALSE);
		}

		else if (strcmpignorecase("ACTIVATE_OBJECTIVE", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			printf("Script is activating %s\n", token);

			activateEntitiesWithObjectiveName(token, TRUE);
		}

		else if (strcmpignorecase("DEACTIVATE_OBJECTIVE", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			activateEntitiesWithObjectiveName(token, FALSE);
		}

		else if (strcmpignorecase("LOAD_LEVEL", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			setNextLevelFromScript(token);
		}

		else if (strcmpignorecase("REMOVE", command) == 0 || strcmpignorecase("HAS_ITEM", command) == 0)
		{
			getInventoryItemFromScript(script.text[script.line]);
		}

		else if (strcmpignorecase("USE_ITEM", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			useInventoryItemFromScript(token);
		}

		else if (strcmpignorecase("WAIT", command) == 0)
		{
			freeDialogBox();

			token = strtok_r(NULL, "\0", &savePtr);

			script.thinkTime = atoi(token);
		}

		else if (strcmpignorecase("PLAY_SOUND", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			playSoundToMap(token, -1, player.x, player.y, 0);
		}

		else if (strcmpignorecase("MUSIC", command) == 0)
		{
			token = strtok_r(NULL, " ", &savePtr);

			if (strcmpignorecase("START", token) == 0)
			{
				playMusic();
			}

			else if (strcmpignorecase("STOP", token) == 0)
			{
				stopMusic();
			}

			else if (strcmpignorecase("FADE_DOWN", token) == 0)
			{
				token = strtok_r(NULL, "\0", &savePtr);

				fadeOutMusic(atoi(token));
			}

			else if (strcmpignorecase("FADE_UP", token) == 0)
			{
				token = strtok_r(NULL, "\0", &savePtr);

				fadeInMusic(atoi(token));
			}

			else if (strcmpignorecase("LOAD", token) == 0)
			{
				token = strtok_r(NULL, "\0", &savePtr);

				loadMusic(token);
			}
		}

		else if (strcmpignorecase("PLAY_BOSS_MUSIC", command) == 0)
		{
			playBossMusic();
		}

		else if (strcmpignorecase("SHAKE_SCREEN", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			if (strcmpignorecase("LIGHT", token) == 0)
			{
				game.shakeStrength = LIGHT;

				token = strtok_r(NULL, "\0", &savePtr);

				game.shakeThinkTime = atoi(token);
			}

			else if (strcmpignorecase("MEDIUM", token) == 0)
			{
				game.shakeStrength = MEDIUM;

				token = strtok_r(NULL, "\0", &savePtr);

				game.shakeThinkTime = atoi(token);
			}

			else if (strcmpignorecase("STRONG", token) == 0)
			{
				game.shakeStrength = STRONG;

				token = strtok_r(NULL, "\0", &savePtr);

				game.shakeThinkTime = atoi(token);
			}

			else if (strcmpignorecase("STOP", token) == 0)
			{
				game.shakeThinkTime = 1;
			}
		}

		else if (strcmpignorecase("KILL", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

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

			token = strtok_r(NULL, "\0", &savePtr);

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

		else if (strcmpignorecase("WALK_TO", command) == 0 || strcmpignorecase("WALK_TO_RELATIVE", command) == 0
			|| strcmpignorecase("REQUIRES_WALK_TO", command) == 0 || strcmpignorecase("REQUIRES_WALK_TO_RELATIVE", command) == 0)
		{
			freeDialogBox();

			token = strtok_r(NULL, " ", &savePtr);

			if (strcmpignorecase(token, "EDGAR") == 0)
			{
				e = &player;
			}

			else
			{
				e = (strcmpignorecase("WALK_TO", command) == 0 || strcmpignorecase("WALK_TO_RELATIVE", command) == 0) ? getEntityByObjectiveName(token) : getEntityByRequiredName(token);
			}

			if (e == NULL)
			{
				printf("WALK_TO command could not find Entity %s\n", token);

				exit(1);
			}

			token = strtok_r(NULL, "\0", &savePtr);

			printf("%s %s %s\n", command, e->objectiveName, token);

			if (strcmpignorecase("WALK_TO", command) == 0 || strcmpignorecase("REQUIRES_WALK_TO", command) == 0)
			{
				entityWalkTo(e, token);
			}

			else
			{
				entityWalkToRelative(e, token);
			}
		}

		else if (strcmpignorecase("CUSTOM_ACTION", command) == 0)
		{
			freeDialogBox();

			token = strtok_r(NULL, " ", &savePtr);

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
				printf("CUSTOM_ACTION command could not find Entity %s\n", token);

				exit(1);
			}

			token = strtok_r(NULL, "\0", &savePtr);

			addCustomActionFromScript(e, token);
		}

		else if (strcmpignorecase("JUMP", command) == 0)
		{
			freeDialogBox();

			token = strtok_r(NULL, " ", &savePtr);

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
				printf("JUMP command could not find Entity %s\n", token);

				exit(1);
			}

			e->dirY = -JUMP_HEIGHT;
		}

		else if (strcmpignorecase("WATCH", command) == 0)
		{
			freeDialogBox();

			token = strtok_r(NULL, " ", &savePtr);

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
				printf("WATCH command could not find Entity %s\n", token);

				exit(1);
			}

			token = strtok_r(NULL, " ", &savePtr);

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
				printf("WATCH command could not find Entity \"%s\"\n", token);

				exit(1);
			}

			e->target = e2;
		}

		else if (strcmpignorecase("LIMIT_CAMERA", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			limitCameraFromScript(token);
		}

		else if (strcmpignorecase("RESET_CAMERA", command) == 0)
		{
			resetCameraLimits();
		}

		else if (strcmpignorecase("CAMERA_SPEED", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			setCameraSpeed(atof(token));
		}

		else if (command[0] != '#')
		{
			printf("Skipping unknown script command %s\n", command);
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

	freeDialogBox();
}
