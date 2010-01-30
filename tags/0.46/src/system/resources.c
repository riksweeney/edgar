/*
Copyright (C) 2009-2010 Parallel Realities

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

extern Game game;
extern Entity player;

#include "../graphics/animation.h"
#include "../audio/audio.h"
#include "../map.h"
#include "../game.h"
#include "../graphics/sprites.h"
#include "../hud.h"
#include "../graphics/font.h"
#include "../player.h"
#include "../enemy/enemies.h"
#include "../world/lift.h"
#include "../world/target.h"
#include "../item/item.h"
#include "properties.h"
#include "../item/key_items.h"
#include "../world/spawner.h"
#include "../world/pressure_plate.h"
#include "../world/door.h"
#include "../world/weak_wall.h"
#include "../world/switch.h"
#include "../world/line_def.h"
#include "../audio/music.h"
#include "../entity.h"
#include "../world/level_exit.h"
#include "../graphics/decoration.h"
#include "../event/trigger.h"
#include "../event/script.h"
#include "../event/global_trigger.h"
#include "../event/map_trigger.h"
#include "../event/objective.h"
#include "../world/save_point.h"
#include "../inventory.h"
#include "../dialog.h"
#include "../event/script.h"
#include "../collisions.h"
#include "../menu/main_menu.h"
#include "../npc/npc.h"
#include "../world/action_point.h"
#include "../world/falling_platform.h"
#include "../world/trap_door.h"
#include "../world/conveyor_belt.h"
#include "../status_panel.h"
#include "../world/teleporter.h"
#include "../world/vanishing_platform.h"
#include "../world/anti_gravity_field.h"
#include "error.h"
#include "pak.h"

static char **key, **value;

void loadRequiredResources()
{
	/* Load the hud */

	initHud();

	/* Load the font */

	game.font = loadFont("font/vera.ttf", 14);
}

void freeLevelResources()
{
	/* Free the entities */

	freeEntities();

	/* Free the decorations */

	freeDecorations();

	/* Free the animations */

	freeAnimations();

	/* Free the sounds */

	freeSounds();

	/* Free music */

	freeMusic();

	/* Free the map data */

	freeMap();

	/* Free the sprites */

	freeSprites();

	/* Free the triggers */

	freeTriggers();

	/* Free the properties */

	freeProperties();

	/* Free the targets */

	freeTargets();

	/* Free the message queue */

	freeMessageQueue();
}

void freeGameResources()
{
	freeLevelResources();

	/* Free the Global Triggers */

	freeGlobalTriggers();

	/* Free the map triggers */

	freeMapTriggers();

	/* Free the Objectives */

	freeObjectives();

	/* Free the scripts */

	freeScript();

	/* Free the inventory */

	freeInventory();

	/* Free the player */

	freePlayer();

	/* Clear the boss meter */

	freeBossHealthBar();
}

void freeAllResources()
{
	freeLevelResources();

	/* Free the hud */

	freeHud();

	/* Free the dialog box */

	freeDialogBox();

	/* Free the script */

	freeScript();

	/* Free the game surfaces */

	freeGame();

	/* Free the font */

	closeFont(game.font);

	/* Clear the collision grid */

	freeCollisionGrid();

	/* Free the menus */

	freeMainMenu();

	/* Free the pak file */

	freePakFile();

	/* Free the status panel */

	freeStatusPanel();
}

char *loadResources(char *buffer)
{
	int i, startX, startY, type, name, resourceType;
	char *token, *line, itemName[MAX_VALUE_LENGTH], *savePtr2, *savePtr;
	Entity *e;

	savePtr = NULL;

	resourceType = ENTITY_DATA;

	if (key == NULL || value == NULL)
	{
		key = (char **)malloc(sizeof(char *) * MAX_PROPS_FILES);
		value = (char **)malloc(sizeof(char *) * MAX_PROPS_FILES);

		if (key == NULL || value == NULL)
		{
			showErrorAndExit("Ran out of memory when loading properties");
		}

		for (i=0;i<MAX_PROPS_FILES;i++)
		{
			key[i] = (char *)malloc(MAX_VALUE_LENGTH);
			value[i] = (char *)malloc(MAX_VALUE_LENGTH);

			if (key[i] == NULL || value[i] == NULL)
			{
				showErrorAndExit("Ran out of memory when loading properties");
			}
		}
	}

	for (i=0;i<MAX_PROPS_FILES;i++)
	{
		key[i][0] = '\0';
		value[i][0] = '\0';
	}

	i = 0;

	name = type = startX = startY = -1;

	e = NULL;

	line = strtok_r(buffer, "\n", &savePtr);

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

		if (line[0] == '#' || line[0] == '\n')
		{
			line = strtok_r(NULL, "\n", &savePtr);

			continue;
		}

		sscanf(line, "%s", itemName);

		if (strcmpignorecase(itemName, "MAP_NAME") == 0)
		{
			printf("Encountered Map Data for %s. Returning\n", line);

			break;
		}

		else if (strcmpignorecase(itemName, "UPDATE_ENTITY") == 0 || strcmpignorecase(itemName, "REMOVE_ENTITY") == 0)
		{
			printf("Encountered Patch Instruction %s. Returning\n", line);

			break;
		}

		else if (strcmpignorecase(line, "PLAYER_INVENTORY") == 0)
		{
			resourceType = PLAYER_INVENTORY;
		}

		else if (strcmpignorecase(line, "ENTITY_DATA") == 0)
		{
			resourceType = ENTITY_DATA;
		}

		else if (strcmpignorecase(line, "{") == 0)
		{
			i = 0;

			name = type = startX = startY = -1;

			e = NULL;
		}

		else if (strcmpignorecase(line, "}") == 0)
		{
			e = NULL;

			if (strcmpignorecase(value[type], "ITEM") == 0 || strcmpignorecase(value[type], "SHIELD") == 0 || strcmpignorecase(value[type], "WEAPON") == 0)
			{
				e = addPermanentItem(value[name], atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "TEMP_ITEM") == 0 || strcmpignorecase(value[type], "HEALTH") == 0)
			{
				e = addTemporaryItem(value[name], atoi(value[startX]), atoi(value[startY]), LEFT, 0, 0);
			}

			else if (strcmpignorecase(value[type], "PLAYER") == 0)
			{
				e = loadPlayer(atoi(value[startX]), atoi(value[startY]), NULL);
			}

			else if (strcmpignorecase(value[type], "PLAYER_WEAPON") == 0)
			{
				setPlayerWeaponName(value[name]);
			}

			else if (strcmpignorecase(value[type], "PLAYER_SHIELD") == 0)
			{
				setPlayerShieldName(value[name]);
			}

			else if (strcmpignorecase(value[type], "KEY_ITEM") == 0)
			{
				e = addKeyItem(value[name], atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "ENEMY") == 0)
			{
				e = addEnemy(value[name], atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "NPC") == 0)
			{
				e = addNPC(value[name], atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "AUTO_LIFT") == 0 || strcmpignorecase(value[type], "MANUAL_LIFT") == 0)
			{
				e = addLift(value[name], atoi(value[startX]), atoi(value[startY]), getEntityTypeByName(value[type]));
			}

			else if (strcmpignorecase(value[type], "SPAWNER") == 0)
			{
				e = addSpawner(atoi(value[startX]), atoi(value[startY]), value[name]);
			}

			else if (strcmpignorecase(value[type], "TARGET") == 0)
			{
				addTarget(atoi(value[startX]), atoi(value[startY]), value[name]);
			}

			else if (strcmpignorecase(value[type], "PRESSURE_PLATE") == 0)
			{
				e = addPressurePlate(value[name], atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "TELEPORTER") == 0)
			{
				e = addTeleporter(value[name], atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "AUTO_DOOR") == 0 || strcmpignorecase(value[type], "MANUAL_DOOR") == 0)
			{
				e = addDoor(value[name], atoi(value[startX]), atoi(value[startY]), getEntityTypeByName(value[type]));
			}

			else if (strcmpignorecase(value[type], "WEAK_WALL") == 0)
			{
				e = addWeakWall(value[name], atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "TRAP_DOOR") == 0)
			{
				e = addTrapDoor(value[name], atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "CONVEYOR_BELT") == 0)
			{
				e = addConveyorBelt(value[name], atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "SWITCH") == 0)
			{
				e = addSwitch(value[name], atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "SCRIPT_LINE_DEF") == 0 || strcmpignorecase(value[type], "LINE_DEF") == 0)
			{
				e = addLineDef(value[type], value[name], atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "LEVEL_EXIT") == 0)
			{
				e = addLevelExit(value[name], atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "SAVE_POINT") == 0)
			{
				/* Save points don't spawn for replays */

				if (game.gameType == NORMAL)
				{
					e = addSavePoint(atoi(value[startX]), atoi(value[startY]));
				}
			}

			else if (strcmpignorecase(value[type], "TRIGGER") == 0)
			{
				addTriggerFromResource(key, value);
			}

			else if (strcmpignorecase(value[type], "GLOBAL_TRIGGER") == 0)
			{
				addGlobalTriggerFromResource(key, value);
			}

			else if (strcmpignorecase(value[type], "MAP_TRIGGER") == 0)
			{
				addMapTriggerFromResource(key, value);
			}

			else if (strcmpignorecase(value[type], "OBJECTIVE") == 0)
			{
				addObjectiveFromResource(key, value);
			}

			else if (strcmpignorecase(value[type], "ACTION_POINT") == 0)
			{
				e = addActionPoint(value[name], atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "FALLING_PLATFORM") == 0)
			{
				e = addFallingPlatform(atoi(value[startX]), atoi(value[startY]), value[name]);
			}

			else if (strcmpignorecase(value[type], "VANISHING_PLATFORM") == 0)
			{
				e = addVanishingPlatform(atoi(value[startX]), atoi(value[startY]), value[name]);
			}

			else if (strcmpignorecase(value[type], "ANTI_GRAVITY") == 0)
			{
				e = addAntiGravityField(atoi(value[startX]), atoi(value[startY]), value[name]);
			}

			else
			{
				showErrorAndExit("Unknown Entity type %s", value[type]);
			}

			if (e != NULL)
			{
				for (i=0;i<MAX_PROPS_FILES;i++)
				{
					if (strlen(key[i]) > 0)
					{
						setProperty(e, key[i], value[i]);
					}
				}

				if (resourceType == PLAYER_INVENTORY)
				{
					printf("Adding %s to inventory\n", e->name);

					addToInventory(e);
				}
			}

			for (i=0;i<MAX_PROPS_FILES;i++)
			{
				key[i][0] = '\0';

				value[i][0] = '\0';
			}

			i = 0;
		}

		else
		{
			token = strtok_r(line, " ", &savePtr2);

			STRNCPY(key[i], token, MAX_VALUE_LENGTH);

			token = strtok_r(NULL, "\0", &savePtr2);

			if (token != NULL)
			{
				STRNCPY(value[i], token, MAX_VALUE_LENGTH);
			}

			else
			{
				key[i][0] = '\0';
			}

			if (strcmpignorecase(key[i], "TYPE") == 0)
			{
				type = i;
			}

			else if (strcmpignorecase(key[i], "START_X") == 0)
			{
				startX = i;
			}

			else if (strcmpignorecase(key[i], "START_Y") == 0)
			{
				startY = i;
			}

			else if (strcmpignorecase(key[i], "NAME") == 0)
			{
				name = i;
			}

			i++;
		}

		line = strtok_r(NULL, "\n", &savePtr);
	}

	loadInventoryItems();

	return line;
}

void patchEntities(double versionFile, char *mapName)
{
	char patchFile[MAX_PATH_LENGTH], *line, *savePtr, itemName[MAX_VALUE_LENGTH];
	char key[MAX_VALUE_LENGTH], value[MAX_VALUE_LENGTH];
	int skipping = FALSE, x, y, read, found;
	unsigned char *buffer;
	Entity *e;
	Target *t;

	savePtr = NULL;

	snprintf(patchFile, sizeof(patchFile), "data/patch/%0.2f.dat", versionFile);

	printf("Looking for %s\n", patchFile);

	if (existsInPak(patchFile) == TRUE)
	{
		printf("Found %s in pakFile\n", patchFile);

		buffer = loadFileFromPak(patchFile);

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

			if (strcmpignorecase(itemName, "MAP_NAME") == 0)
			{
				sscanf(line, "%*s %s\n", itemName);

				skipping = strcmpignorecase(itemName, mapName) == 0 ? FALSE : TRUE;
			}

			else if (strcmpignorecase(itemName, "MODIFY_OBJECTIVE") == 0 && skipping == FALSE)
			{
				sscanf(line, "%*s \"%[^\"]\" \"%[^\"]\"", key, value);

				printf("Updating Objective %s\n", key);

				modifyObjective(key, value);
			}

			else if (strcmpignorecase(line, "ADD_ENTITY") == 0 && skipping == FALSE)
			{
				printf("Adding new Entities to %s\n", mapName);

				loadResources(savePtr);
			}

			else if (strcmpignorecase(itemName, "REMOVE_ENTITY") == 0 && skipping == FALSE)
			{
				read = sscanf(line, "%*s %s %d %d\n", itemName, &x, &y);

				found = FALSE;

				printf("Removing %s\n", itemName);

				e = getEntityByObjectiveName(itemName);

				if (e != NULL)
				{
					e->inUse = FALSE;

					found = TRUE;
				}

				if (found == FALSE)
				{
					t = getTargetByName(itemName);

					if (t != NULL)
					{
						t->active = FALSE;

						found = TRUE;
					}
				}

				if (found == FALSE && read == 3)
				{
					e = getEntityByStartXY(x, y);

					if (e != NULL)
					{
						e->inUse = FALSE;

						found = TRUE;
					}
				}
			}

			else if (strcmpignorecase(itemName, "UPDATE_ENTITY") == 0 && skipping == FALSE)
			{
				read = sscanf(line, "%*s %s %s %s\n", itemName, key, value);

				if (strcmpignorecase(itemName, "PLAYER") == 0)
				{
					e = &player;
				}

				else
				{
					e = getEntityByObjectiveName(itemName);
				}

				if (e != NULL)
				{
					setProperty(e, key, value);
				}
			}
			
			else if (strcmpignorecase(itemName, "UPDATE_ENTITY_BY_START") == 0 && skipping == FALSE)
			{
				read = sscanf(line, "%*s %d %d %s %s\n", &x, &y, key, value);
				
				if (strcmpignorecase(itemName, "PLAYER") == 0)
				{
					e = &player;
				}

				else
				{
					e = getEntityByStartXY(x, y);
				}
				
				if (e != NULL)
				{
					setProperty(e, key, value);
				}
			}

			line = strtok_r(NULL, "\n", &savePtr);
		}

		free(buffer);
	}
}
