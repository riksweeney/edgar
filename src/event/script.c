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

#include "../audio/audio.h"
#include "../audio/music.h"
#include "../credits.h"
#include "../custom_actions.h"
#include "../dialog.h"
#include "../entity.h"
#include "../event/global_trigger.h"
#include "../event/map_trigger.h"
#include "../event/objective.h"
#include "../game.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../inventory.h"
#include "../map.h"
#include "../medal.h"
#include "../menu/script_menu.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/load_save.h"
#include "../system/pak.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../weather.h"
#include "script.h"

extern Entity player, *self;
extern Game game;

static Script script;

static void setYes(void);
static void setNo(void);

void runScript(char *name)
{
	char filename[MAX_PATH_LENGTH], *line, *savePtr, **existingLines;
	unsigned char *buffer;
	int i, newCount;

	/* Don't fire if the player's health is 0 */

	if (player.health <= 0)
	{
		return;
	}

	newCount = 0;

	savePtr = NULL;

	snprintf(filename, sizeof(filename), "data/scripts/%s.dat", name);

	buffer = loadFileFromPak(filename);

	if (script.text != NULL)
	{
		newCount = countTokens((char *)buffer, "\n");

		existingLines = malloc(sizeof(char *) * script.lineCount);

		if (existingLines == NULL)
		{
			showErrorAndExit("Failed to allocate a whole %d bytes for script %s", (int)(sizeof(char *) * script.lineCount), filename);
		}

		for (i=0;i<script.lineCount;i++)
		{
			existingLines[i] = malloc(strlen(script.text[i]) + 1);

			if (existingLines[i] == NULL)
			{
				showErrorAndExit("Failed to allocate %d bytes for script line %d", (int)(sizeof(char *) * strlen(script.text[i]) + 1), (i + 1));
			}

			STRNCPY(existingLines[i], script.text[i], strlen(script.text[i]) + 1);

			free(script.text[i]);
		}

		free(script.text);

		script.text = NULL;

		script.text = malloc(sizeof(char *) * (script.lineCount + newCount));

		if (script.text == NULL)
		{
			showErrorAndExit("Failed to allocate a whole %d bytes for script %s", (int)(sizeof(char *) * (script.lineCount + newCount)), filename);
		}

		for (i=0;i<script.lineCount;i++)
		{
			script.text[i] = malloc(strlen(existingLines[i]) + 1);

			if (script.text[i] == NULL)
			{
				showErrorAndExit("Failed to allocate %d bytes for script line %d", (int)(sizeof(char *) * strlen(existingLines[i]) + 1), (i + 1));
			}

			STRNCPY(script.text[i], existingLines[i], strlen(existingLines[i]) + 1);

			free(existingLines[i]);
		}

		script.lineCount += newCount;

		free(existingLines);

		existingLines = NULL;
	}

	else
	{
		script.lineCount = countTokens((char *)buffer, "\n");

		script.text = malloc(sizeof(char *) * script.lineCount);

		if (script.text == NULL)
		{
			showErrorAndExit("Failed to allocate a whole %d bytes for script %s", (int)(sizeof(char *) * script.lineCount), filename);
		}

		script.line = 0;

		i = 0;
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

		script.text[i] = malloc(strlen(line) + 1);

		if (script.text[i] == NULL)
		{
			showErrorAndExit("Failed to allocate %d bytes for script line %d", (int)(sizeof(char *) * strlen(line) + 1), (i + 1));
		}

		STRNCPY(script.text[i], line, strlen(line) + 1);

		i++;

		line = strtok_r(NULL, "\n", &savePtr);
	}

	free(buffer);

	if (newCount == 0)
	{
		script.skipping = FALSE;

		script.currentDepth = 0;

		script.requiredDepth = 0;

		playerWaitForDialog();

		readNextScriptLine();
	}
}

void readNextScriptLine()
{
	char *token, line[MAX_LINE_LENGTH], command[MAX_VALUE_LENGTH], *savePtr, *token2;
	int readAgain = TRUE, val;
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

			freeDialogBox();
		}

		STRNCPY(line, script.text[script.line], sizeof(line));

		if (strlen(line) == 0)
		{
			script.line++;

			continue;
		}

		token = strtok_r(line, " ", &savePtr);

		STRNCPY(command, token, sizeof(command));

		if (script.skipping == TRUE)
		{
			if (strcmpignorecase("IF", command) == 0)
			{
				script.currentDepth++;
			}

			else if (strcmpignorecase("END", command) == 0)
			{
				script.currentDepth--;

				if (script.currentDepth == script.requiredDepth)
				{
					script.skipping = FALSE;

					if (script.currentDepth < 0)
					{
						showErrorAndExit("Script error, unmatched END");
					}
				}
			}
		}

		else if (strcmpignorecase("TALK", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			createDialogBoxFromScript(token);

			readAgain = FALSE;
		}

		else if (strcmpignorecase("AUTO_TALK", command) == 0)
		{
			token = strtok_r(NULL, " ", &savePtr);

			token2 = strtok_r(NULL, "\0", &savePtr);

			createDialogBoxFromScript(token2);

			val = atoi(token);

			script.thinkTime = val;
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
				showErrorAndExit("ADD command encountered unknown action %s", token);
			}
		}

		else if (strcmpignorecase("IF", command) == 0)
		{
			token = strtok_r(NULL, " ", &savePtr);

			if (strcmpignorecase(token, "HAS_VISITED") == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				if (hasPersistance(token) == FALSE)
				{
					script.skipping = TRUE;
				}
			}

			else if (strcmpignorecase(token, "HAS_NOT_VISITED") == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				if (hasPersistance(token) == TRUE)
				{
					script.skipping = TRUE;
				}
			}

			else if (strcmpignorecase(token, "CONFIRM_YES") == 0)
			{
				if (script.yesNoResult == FALSE)
				{
					script.skipping = TRUE;
				}
			}

			else if (strcmpignorecase(token, "CONFIRM_NO") == 0)
			{
				if (script.yesNoResult == TRUE)
				{
					script.skipping = TRUE;
				}
			}

			else if (strcmpignorecase(token, "CHEATING") == 0)
			{
				if (game.cheating == FALSE)
				{
					script.skipping = TRUE;
				}
			}

			else if (strcmpignorecase(token, "NOT_CHEATING") == 0)
			{
				if (game.cheating == TRUE)
				{
					script.skipping = TRUE;
				}
			}

			else if (strcmpignorecase(token, "EXISTS") == 0)
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
					script.skipping = TRUE;
				}
			}

			else if (strcmpignorecase(token, "NOT_EXISTS") == 0)
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

				if (e != NULL)
				{
					script.skipping = TRUE;
				}
			}

			else if (strcmpignorecase(token, "EQUALS") == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				token2 = strtok_r(NULL, " ", &savePtr);

				val = -1;

				if (strcmpignorecase(token, "[CONTINUE_COUNT]") == 0)
				{
					val = game.continues;
				}

				if (val != atoi(token2))
				{
					script.skipping = TRUE;
				}
			}

			else if (strcmpignorecase(token, "GREATER_THAN") == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				token2 = strtok_r(NULL, " ", &savePtr);

				val = -1;

				if (strcmpignorecase(token, "[CONTINUE_COUNT]") == 0)
				{
					val = game.continues;
				}

				if (val <= atoi(token2))
				{
					script.skipping = TRUE;
				}
			}

			else if (strcmpignorecase(token, "LESS_THAN") == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				token2 = strtok_r(NULL, " ", &savePtr);

				val = -1;

				if (strcmpignorecase(token, "[CONTINUE_COUNT]") == 0)
				{
					val = game.continues;
				}

				if (val > atoi(token2))
				{
					script.skipping = TRUE;
				}
			}

			else
			{
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
					showErrorAndExit("IF command could not find Entity %s", token);
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

				else if (strcmpignorecase(token, "MENTAL") == 0)
				{
					token = strtok_r(NULL, " ", &savePtr);

					if (strcmpignorecase(token, "NOT") == 0)
					{
						token = strtok_r(NULL, " ", &savePtr);

						if (e->mental == atoi(token))
						{
							script.skipping = TRUE;
						}
					}

					else if (e->mental != atoi(token))
					{
						script.skipping = TRUE;
					}
				}

				else if (strcmpignorecase(token, "ACTIVE") == 0)
				{
					token = strtok_r(NULL, " ", &savePtr);

					if (e->active != TRUE)
					{
						script.skipping = TRUE;
					}
				}

				else if (strcmpignorecase(token, "NOT_ACTIVE") == 0)
				{
					token = strtok_r(NULL, " ", &savePtr);

					if (e->active != FALSE)
					{
						script.skipping = TRUE;
					}
				}

				else
				{
					showErrorAndExit("Unknown IF command %s",token);
				}
			}

			script.currentDepth++;

			if (script.skipping == FALSE)
			{
				script.requiredDepth++;
			}
		}

		else if (strcmpignorecase("WHILE", command) == 0 || strcmpignorecase("WHILE_NOT", command) == 0)
		{
			token = strtok_r(NULL, " ", &savePtr);

			if (strcmpignorecase(token, "EXISTS") == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				e = getEntityByObjectiveName(token);

				if (e != NULL)
				{
					freeDialogBox();

					script.thinkTime = 15;

					script.line--;
				}
			}

			else if (strcmpignorecase(token, "NOT_EXISTS") == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				e = getEntityByObjectiveName(token);

				if (e == NULL)
				{
					freeDialogBox();

					script.thinkTime = 15;

					script.line--;
				}
			}

			else if (strcmpignorecase(token, "CAMERA_NOT_AT_MINIMUM") == 0)
			{
				if (cameraAtMinimum() == FALSE)
				{
					freeDialogBox();

					script.thinkTime = 15;

					script.line--;
				}
			}

			else
			{
				e = getEntityByObjectiveName(token);

				if (e == NULL)
				{
					showErrorAndExit("WHILE command could not find Entity %s", token);
				}

				token = strtok_r(NULL, " ", &savePtr);

				if (strcmpignorecase(token, "HEALTH") == 0)
				{
					token = strtok_r(NULL, " ", &savePtr);

					val = atoi(token);

					if ((strcmpignorecase("WHILE", command) == 0 && e->health == val) ||
						(strcmpignorecase("WHILE_NOT", command) == 0 && e->health != val))
					{
						freeDialogBox();

						script.thinkTime = 15;

						script.line--;
					}
				}

				else if (strcmpignorecase(token, "MENTAL") == 0)
				{
					token = strtok_r(NULL, " ", &savePtr);

					val = atoi(token);

					if ((strcmpignorecase("WHILE", command) == 0 && e->mental == val) ||
						(strcmpignorecase("WHILE_NOT", command) == 0 && e->mental != val))
					{
						freeDialogBox();

						script.thinkTime = 15;

						script.line--;
					}
				}
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
				showErrorAndExit("SET command could not find Entity %s", token);
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

			else if (strcmpignorecase("PROPERTY", token) == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				token2 = strtok_r(NULL, "\0", &savePtr);

				setProperty(e, token, token2);
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
						showErrorAndExit("FACE command could not find Entity \"%s\"", token);
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

				setEntityAnimation(e, token);

				if (e == &player)
				{
					syncWeaponShieldToPlayer();
				}
			}

			else if (strcmpignorecase(token, "PROPERTIES") == 0)
			{
				token = strtok_r(NULL, " ", &savePtr);

				loadProperties(token, e);

				if (e == &player)
				{
					syncWeaponShieldToPlayer();
				}
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
				showErrorAndExit("Unknown SET command %s", token);
			}
		}

		else if (strcmpignorecase("ACTIVATE_REQUIRED", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			activateEntitiesWithRequiredName(token, TRUE);
		}

		else if (strcmpignorecase(token, "ATTACK") == 0)
		{
			scriptAttack();
		}

		else if (strcmpignorecase("DEACTIVATE_REQUIRED", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			activateEntitiesWithRequiredName(token, FALSE);
		}

		else if (strcmpignorecase("ACTIVATE_OBJECTIVE", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			activateEntitiesWithObjectiveName(token, TRUE);
		}

		else if (strcmpignorecase("DEACTIVATE_OBJECTIVE", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			activateEntitiesWithObjectiveName(token, FALSE);
		}

		else if (strcmpignorecase("ACTIVATE_OBJECTIVE_WITH_VALUE", command) == 0)
		{
			token = strtok_r(NULL, " ", &savePtr);

			token2 = strtok_r(NULL, " ", &savePtr);

			val = atoi(token2);

			activateEntitiesValueWithObjectiveName(token, val);
		}

		else if (strcmpignorecase("EQUIP_WEAPON", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			setWeaponFromScript(token);
		}

		else if (strcmpignorecase("EQUIP_SHIELD", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			setShieldFromScript(token);
		}

		else if (strcmpignorecase("UNEQUIP_WEAPON", command) == 0)
		{
			unsetWeapon();
		}

		else if (strcmpignorecase("UNEQUIP_SHIELD", command) == 0)
		{
			unsetShield();
		}

		else if (strcmpignorecase("WEATHER", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			setWeather(getWeatherTypeByName(token));
		}

		else if (strcmpignorecase(token, "SHOW_CONFIRM") == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			freeDialogBox();

			setScriptCounter(1);

			script.menu = initScriptMenu(token, &setYes, &setNo);

			script.draw = &drawScriptMenu;

			playerWaitForConfirm();
		}

		else if (strcmpignorecase("LOAD_LEVEL", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			setNextLevelFromScript(token);
		}

		else if (strcmpignorecase("SHOW_CREDITS", command) == 0)
		{
			showEndCredits();
		}

		else if (strcmpignorecase("REMOVE", command) == 0 || strcmpignorecase("HAS_ITEM", command) == 0)
		{
			getInventoryItemFromScript(script.text[script.line]);
		}

		else if (strcmpignorecase("ADD_TO_INVENTORY", command) == 0)
		{
			token = strtok_r(NULL, " ", &savePtr);

			token2 = strtok_r(NULL, "\0", &savePtr);

			scriptAddToInventory(token, token2 == NULL ? FALSE : TRUE);
		}

		else if (strcmpignorecase("HAS_OBJECTIVE", command) == 0)
		{
			getObjectiveFromScript(script.text[script.line]);
		}

		else if (strcmpignorecase("USE_ITEM", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			useInventoryItemFromScript(token);
		}

		else if (strcmpignorecase("BECOME_SLIME", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			becomeJumpingSlime(atoi(token));
		}

		else if (strcmpignorecase("FIRE_TRIGGER", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			fireGlobalTrigger(token);
		}

		else if (strcmpignorecase("WAIT", command) == 0)
		{
			freeDialogBox();

			token = strtok_r(NULL, "\0", &savePtr);

			script.thinkTime = atoi(token);
		}

		else if (strcmpignorecase("REMOVE_ALL_SPAWNED_IN", command) == 0)
		{
			removeAllSpawnedIn();
		}

		else if (strcmpignorecase("DISABLE_SPAWNERS", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			disableSpawners(atoi(token));
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
				playMapMusic();
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

				playLoadedMusic();
			}

			else if (strcmpignorecase("PLAY_BOSS_MUSIC", token) == 0)
			{
				playDefaultBossMusic();

				playLoadedMusic();
			}

			else if (strcmpignorecase("OVERRIDE", token) == 0)
			{
				game.overrideMusic = TRUE;
			}

			else if (strcmpignorecase("REMOVE_OVERRIDE", token) == 0)
			{
				game.overrideMusic = FALSE;
			}
		}

		else if (strcmpignorecase("ADD_MEDAL", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			addMedal(token);
		}

		else if (strcmpignorecase("ADD_SECRET", command) == 0)
		{
			increaseSecretsFound();
		}

		else if (strcmpignorecase("FLASH_SCREEN", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			fadeFromColour(255, 255, 255, atoi(token));
		}

		else if (strcmpignorecase("PLAY_BOSS_MUSIC", command) == 0)
		{
			playDefaultBossMusic();
		}

		else if (strcmpignorecase("SHAKE_SCREEN", command) == 0)
		{
			token = strtok_r(NULL, " ", &savePtr);

			if (strcmpignorecase("LIGHT", token) == 0)
			{
				token = strtok_r(NULL, "\0", &savePtr);

				shakeScreen(LIGHT, atoi(token));
			}

			else if (strcmpignorecase("MEDIUM", token) == 0)
			{
				token = strtok_r(NULL, "\0", &savePtr);

				shakeScreen(MEDIUM, atoi(token));
			}

			else if (strcmpignorecase("STRONG", token) == 0)
			{
				token = strtok_r(NULL, "\0", &savePtr);

				shakeScreen(STRONG, atoi(token));
			}

			else if (strcmpignorecase("STOP", token) == 0)
			{
				shakeScreen(LIGHT, 0);
			}
		}

		else if (strcmpignorecase("KILL", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

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
				showErrorAndExit("KILL command could not find Entity %s", token);
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

		else if (strcmpignorecase("RESET", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

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
				showErrorAndExit("RESET command could not find Entity %s", token);
			}

			if (e->fallout != NULL)
			{
				self = e;

				self->fallout();

				self->thinkTime = 0;
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
					showErrorAndExit("FOLLOW command could not find Entity %s", token);
				}
			}

			centerMapOnEntity(e);
		}

		else if (strcmpignorecase("WALK_TO", command) == 0 || strcmpignorecase("WALK_TO_RELATIVE", command) == 0
			|| strcmpignorecase("REQUIRES_WALK_TO", command) == 0 || strcmpignorecase("REQUIRES_WALK_TO_RELATIVE", command) == 0
			|| strcmpignorecase("WALK_TO_ENTITY", command) == 0)
		{
			freeDialogBox();

			token = strtok_r(NULL, " ", &savePtr);

			if (strcmpignorecase(token, "EDGAR") == 0)
			{
				e = &player;
			}

			else
			{
				e = (strcmpignorecase("WALK_TO", command) == 0 || strcmpignorecase("WALK_TO_RELATIVE", command) == 0
				|| strcmpignorecase("WALK_TO_ENTITY", command) == 0) ? getEntityByObjectiveName(token) : getEntityByRequiredName(token);
			}

			if (e == NULL)
			{
				showErrorAndExit("WALK_TO command could not find Entity %s", token);
			}

			token = strtok_r(NULL, "\0", &savePtr);

			if (strcmpignorecase("WALK_TO", command) == 0 || strcmpignorecase("REQUIRES_WALK_TO", command) == 0)
			{
				entityWalkTo(e, token);
			}

			else if (strcmpignorecase("WALK_TO_ENTITY", command) == 0)
			{
				entityWalkToEntity(e, token);
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
				showErrorAndExit("CUSTOM_ACTION command could not find Entity %s", token);
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
				showErrorAndExit("JUMP command could not find Entity %s", token);
			}

			e->dirY = -JUMP_HEIGHT;
		}

		else if (strcmpignorecase("TELEPORT", command) == 0)
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
				showErrorAndExit("TELEPORT command could not find Entity %s", token);
			}

			token = strtok_r(NULL, "\0", &savePtr);

			teleportEntityFromScript(e, token);
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
				showErrorAndExit("WATCH command could not find Entity %s", token);
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
				showErrorAndExit("WATCH command could not find Entity \"%s\"", token);
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

		else if (strcmpignorecase("CAMERA_SNAP", command) == 0)
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

			centerMapOnEntity(e);

			cameraSnapToTargetEntity();
		}

		else if (strcmpignorecase("CAMERA_SPEED", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			setCameraSpeed(atof(token));
		}

		else if (strcmpignorecase("CAMERA_START", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			setCameraPositionFromScript(token);
		}

		else if (strcmpignorecase("RUN_SCRIPT", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			runScript(token);
		}

		else if (strcmpignorecase("HUD", command) == 0)
		{
			token = strtok_r(NULL, "\0", &savePtr);

			if (strcmpignorecase("OFF", token) == 0)
			{
				game.showHUD = FALSE;
			}

			else
			{
				game.showHUD = TRUE;
			}
		}

		else if (command[0] != '#')
		{
			#if DEV == 1
			printf("Skipping unknown script command %s\n", command);
			#endif
		}

		script.line++;
	}
}

int scriptRunning()
{
	return script.lineCount != 0;
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

		script.counter = 0;

		script.thinkTime = 0;

		script.yesNoResult = 0;

		script.lineCount = 0;
	}

	freeScriptMenu();

	script.draw = NULL;

	freeDialogBox();
}

void scriptDrawMenu()
{
	if (script.draw != NULL)
	{
		script.draw();
	}
}

static void setYes()
{
	script.yesNoResult = TRUE;

	setScriptCounter(-1);

	playerWaitForDialog();

	script.draw = NULL;

	freeScriptMenu();
}

static void setNo()
{
	script.yesNoResult = FALSE;

	setScriptCounter(-1);

	playerWaitForDialog();

	script.draw = NULL;

	freeScriptMenu();
}

void doScriptMenu()
{
	script.menu->action();
}
