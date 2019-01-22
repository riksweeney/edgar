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
#include "../collisions.h"
#include "../dialog.h"
#include "../enemy/enemies.h"
#include "../entity.h"
#include "../event/global_trigger.h"
#include "../event/map_trigger.h"
#include "../event/objective.h"
#include "../event/script.h"
#include "../event/trigger.h"
#include "../game.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../graphics/font.h"
#include "../graphics/sprites.h"
#include "../graphics/texture_cache.h"
#include "../hud.h"
#include "../inventory.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../map.h"
#include "../menu/menu.h"
#include "../player.h"
#include "../status_panel.h"
#include "../title.h"
#include "../world/action_point.h"
#include "../world/anti_gravity_field.h"
#include "../world/conveyor_belt.h"
#include "../world/door.h"
#include "../world/falling_platform.h"
#include "../world/level_exit.h"
#include "../world/lift.h"
#include "../world/line_def.h"
#include "../world/npc.h"
#include "../world/pressure_plate.h"
#include "../world/save_point.h"
#include "../world/spawner.h"
#include "../world/switch.h"
#include "../world/target.h"
#include "../world/teleporter.h"
#include "../world/trap_door.h"
#include "../world/vanishing_platform.h"
#include "../world/weak_wall.h"
#include "error.h"
#include "pak.h"
#include "properties.h"
#include "resources.h"

extern Game game;
extern Entity player;

static char **key, **value;

void loadRequiredResources()
{
	int code;

	/* Load the hud */

	initHud();

	/* Load the font */

	if (strlen(game.customFont) == 0)
	{
		code = getCharacterCodeForTestString();

		if (code >= 0x4E00)
		{
			#if DEV == 1
				printf("Code %d appears to be CJK. Using fallback font\n", code);
			#endif

			game.font = loadFont("font/DroidSansFallback.ttf", NORMAL_FONT_SIZE);

			game.largeFont = loadFont("font/DroidSansFallback.ttf", LARGE_FONT_SIZE);
		}

		else
		{
			game.font = loadFont("font/DejaVuSans.ttf", NORMAL_FONT_SIZE);

			game.largeFont = loadFont("font/DejaVuSans.ttf", LARGE_FONT_SIZE);
		}
	}

	else
	{
		game.font = loadCustomFont(game.customFont, game.fontSizeSmall);

		game.largeFont = loadCustomFont(game.customFont, game.fontSizeLarge);
	}
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

	if (game.overrideMusic == FALSE)
	{
		/* Free music */

		freeMusic();
	}

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

	/* Free the scripts */

	freeScript();
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

	/* Free the title screen */

	freeTitle();
}

void freeAllResources()
{
	int i;

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

	closeFont(game.largeFont);

	/* Clear the collision grid */

	freeCollisionGrid();

	/* Free the menus */

	freeMenus();

	/* Free the pak file */

	freePakFile();

	/* Free the texture cache */

	freeTextureCache();

	if (key != NULL)
	{
		for (i=0;i<MAX_PROPS_FILES;i++)
		{
			free(key[i]);
		}
		
		free(key);

		key = NULL;
	}

	if (value != NULL)
	{
		for (i=0;i<MAX_PROPS_FILES;i++)
		{
			free(value[i]);
		}
		
		free(value);

		value = NULL;
	}
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
		key = malloc(sizeof(char *) * MAX_PROPS_FILES);
		value = malloc(sizeof(char *) * MAX_PROPS_FILES);

		if (key == NULL || value == NULL)
		{
			showErrorAndExit("Ran out of memory when loading properties");
		}

		for (i=0;i<MAX_PROPS_FILES;i++)
		{
			key[i] = malloc(MAX_VALUE_LENGTH);
			value[i] = malloc(MAX_VALUE_LENGTH);

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

		else if (line[0] == ' ')
		{
			printf("WARNING: Line starts with a space\n");

			#if DEV == 1
				exit(0);
			#endif

			line = strtok_r(NULL, "\n", &savePtr);

			continue;
		}

		sscanf(line, "%s", itemName);

		if (strcmpignorecase(itemName, "MAP_NAME") == 0)
		{
			break;
		}

		else if (strcmpignorecase(itemName, "UPDATE_ENTITY") == 0 || strcmpignorecase(itemName, "REMOVE_ENTITY") == 0)
		{
			break;
		}

		else if (strcmpignorecase(line, "PLAYER_INVENTORY") == 0)
		{
			resourceType = PLAYER_INVENTORY;
		}

		else if (strstr(line, "INVENTORY_INDEX") != NULL)
		{
			sscanf(line, "%*s %d", &startX);

			setInventoryIndex(startX);
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
			e = addEntityFromResource(value[type], value[name], startX == -1 ? 0 : atoi(value[startX]), startY == -1 ? 0 : atoi(value[startY]));

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

Entity *addEntityFromResource(char *type, char *name, int startX, int startY)
{
	Entity *e = NULL;

	if (strcmpignorecase(type, "ITEM") == 0 || strcmpignorecase(type, "SHIELD") == 0 || strcmpignorecase(type, "WEAPON") == 0)
	{
		e = addPermanentItem(name, startX, startY);
	}

	else if (strcmpignorecase(type, "TEMP_ITEM") == 0 || strcmpignorecase(type, "HEALTH") == 0)
	{
		e = addTemporaryItem(name, startX, startY, LEFT, 0, 0);
	}

	else if (strcmpignorecase(type, "PLAYER") == 0)
	{
		e = loadPlayer(startX, startY, NULL);
	}

	else if (strcmpignorecase(type, "PLAYER_WEAPON") == 0)
	{
		setPlayerWeaponName(name);
	}

	else if (strcmpignorecase(type, "PLAYER_SHIELD") == 0)
	{
		setPlayerShieldName(name);
	}

	else if (strcmpignorecase(type, "KEY_ITEM") == 0)
	{
		e = addKeyItem(name, startX, startY);
	}

	else if (strcmpignorecase(type, "ENEMY") == 0)
	{
		e = addEnemy(name, startX, startY);
	}

	else if (strcmpignorecase(type, "NPC") == 0)
	{
		e = addNPC(name, startX, startY);
	}

	else if (strcmpignorecase(type, "AUTO_LIFT") == 0 || strcmpignorecase(type, "MANUAL_LIFT") == 0)
	{
		e = addLift(name, startX, startY, getEntityTypeByName(type));
	}

	else if (strcmpignorecase(type, "SPAWNER") == 0)
	{
		e = addSpawner(startX, startY, name);
	}

	else if (strcmpignorecase(type, "TARGET") == 0)
	{
		addTarget(startX, startY, name);
	}

	else if (strcmpignorecase(type, "PRESSURE_PLATE") == 0)
	{
		e = addPressurePlate(name, startX, startY);
	}

	else if (strcmpignorecase(type, "TELEPORTER") == 0)
	{
		e = addTeleporter(name, startX, startY);
	}

	else if (strcmpignorecase(type, "AUTO_DOOR") == 0 || strcmpignorecase(type, "MANUAL_DOOR") == 0)
	{
		e = addDoor(name, startX, startY, getEntityTypeByName(type));
	}

	else if (strcmpignorecase(type, "WEAK_WALL") == 0)
	{
		e = addWeakWall(name, startX, startY);
	}

	else if (strcmpignorecase(type, "TRAP_DOOR") == 0)
	{
		e = addTrapDoor(name, startX, startY);
	}

	else if (strcmpignorecase(type, "CONVEYOR_BELT") == 0)
	{
		e = addConveyorBelt(name, startX, startY);
	}

	else if (strcmpignorecase(type, "SWITCH") == 0)
	{
		e = addSwitch(name, startX, startY);
	}

	else if (strcmpignorecase(type, "SCRIPT_LINE_DEF") == 0 || strcmpignorecase(type, "LINE_DEF") == 0)
	{
		e = addLineDef(type, name, startX, startY);
	}

	else if (strcmpignorecase(type, "LEVEL_EXIT") == 0)
	{
		e = addLevelExit(name, startX, startY);
	}

	else if (strcmpignorecase(type, "SAVE_POINT") == 0)
	{
		/* Save points don't spawn for replays */

		if (game.gameType == NORMAL)
		{
			e = addSavePoint(startX, startY);
		}
	}

	else if (strcmpignorecase(type, "TRIGGER") == 0)
	{
		addTriggerFromResource(key, value);
	}

	else if (strcmpignorecase(type, "GLOBAL_TRIGGER") == 0)
	{
		addGlobalTriggerFromResource(key, value);
	}

	else if (strcmpignorecase(type, "MAP_TRIGGER") == 0)
	{
		addMapTriggerFromResource(key, value);
	}

	else if (strcmpignorecase(type, "OBJECTIVE") == 0)
	{
		addObjectiveFromResource(key, value);
	}

	else if (strcmpignorecase(type, "ACTION_POINT") == 0)
	{
		e = addActionPoint(name, startX, startY);
	}

	else if (strcmpignorecase(type, "FALLING_PLATFORM") == 0)
	{
		e = addFallingPlatform(startX, startY, name);
	}

	else if (strcmpignorecase(type, "VANISHING_PLATFORM") == 0)
	{
		e = addVanishingPlatform(startX, startY, name);
	}

	else if (strcmpignorecase(type, "ANTI_GRAVITY") == 0)
	{
		e = addAntiGravityField(startX, startY, name);
	}

	else
	{
		showErrorAndExit("Unknown Entity type %s", type);
	}

	return e;
}

int patchEntities(double versionFile, char *mapName)
{
	char patchFile[MAX_PATH_LENGTH], *line, *savePtr, itemName[MAX_VALUE_LENGTH];
	char key[MAX_VALUE_LENGTH], value[MAX_VALUE_LENGTH];
	int skipping = FALSE, x, y, read, found, saveMap;
	unsigned char *buffer;
	Entity *e;
	EntityList *el, *entities;
	Target *t;

	savePtr = NULL;

	snprintf(patchFile, sizeof(patchFile), "data/patch/%0.2f.dat", versionFile);

	saveMap = TRUE;

	if (existsInPak(patchFile) == TRUE)
	{
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

				modifyObjective(key, value);
			}

			else if (strcmpignorecase(itemName, "REMOVE_OBJECTIVE") == 0 && skipping == FALSE)
			{
				sscanf(line, "%*s \"%[^\"]\"", key);

				removeObjective(key);
			}

			else if (strcmpignorecase(itemName, "REMOVE_TRIGGER") == 0 && skipping == FALSE)
			{
				sscanf(line, "%*s \"%[^\"]\"", key);

				removeGlobalTrigger(key);

				removeTrigger(key);
			}

			else if (strcmpignorecase(line, "ADD_ENTITY") == 0 && skipping == FALSE)
			{
				loadResources(savePtr);
			}

			else if (strcmpignorecase(itemName, "REMOVE_ENTITY") == 0 && skipping == FALSE)
			{
				read = sscanf(line, "%*s %s %d %d", itemName, &x, &y);

				found = FALSE;

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
				read = sscanf(line, "%*s %s %s %s", itemName, key, value);

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
					if (strcmpignorecase(value, "NULL") == 0)
					{
						STRNCPY(value, "", sizeof(value));
					}

					setProperty(e, key, value);
				}
			}

			else if (strcmpignorecase(itemName, "UPDATE_ENTITY_BY_START") == 0 && skipping == FALSE)
			{
				read = sscanf(line, "%*s %d %d %s %[^\n]s", &x, &y, key, value);

				e = getEntityByStartXY(x, y);

				if (e != NULL)
				{
					setProperty(e, key, value);
				}
			}

			else if (strcmpignorecase(itemName, "UPDATE_ENTITY_BY_XY") == 0 && skipping == FALSE)
			{
				read = sscanf(line, "%*s %d %d %s %[^\n]s", &x, &y, key, value);

				e = getEntityByXY(x, y);

				if (e != NULL)
				{
					setProperty(e, key, value);
				}
			}

			else if (strcmpignorecase(itemName, "TRANSLATE_ENTITIES") == 0 && skipping == FALSE)
			{
				read = sscanf(line, "%*s %d %d", &x, &y);

				entities = getEntities();

				player.x -= x;
				player.y -= y;

				for (el=entities->next;el!=NULL;el=el->next)
				{
					e = el->entity;

					e->x -= x;
					e->y -= y;

					if (e->startX - x > 0)
					{
						e->startX -= x;
					}

					if (e->startY - y > 0)
					{
						e->startY -= y;
					}

					if (e->endX - x > 0)
					{
						e->endX -= x;
					}

					if (e->endY - y > 0)
					{
						e->endY -= y;
					}
				}

				t = getTargets();

				for (x=0;x<MAX_TARGETS;x++)
				{
					if (t[x].active == TRUE)
					{
						if (t[x].x - x > 0)
						{
							t[x].x -= x;
						}

						if (t[x].y - y > 0)
						{
							t[x].y -= y;
						}
					}
				}
			}

			else if (strcmpignorecase(itemName, "RENAME_MAP") == 0 && skipping == FALSE)
			{
				saveMap = FALSE;
			}

			line = strtok_r(NULL, "\n", &savePtr);
		}

		free(buffer);
	}

	return saveMap;
}
