#include "../headers.h"

extern Game game;

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
#include "../decoration.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"
#include "../event/objective.h"
#include "../world/save_point.h"
#include "../inventory.h"
#include "../dialog.h"
#include "../event/script.h"
#include "../collisions.h"
#include "../menu/main_menu.h"
#include "../npc/npc.h"

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
}

void freeGameResources()
{
	freeLevelResources();

	/* Free the Global Triggers */

	freeGlobalTriggers();

	/* Free the Objectives */

	freeObjectives();
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
}

void loadResources(FILE *fp)
{
	int i, startX, startY, type, name, resourceType;
	char *token, line[MAX_LINE_LENGTH], itemName[MAX_VALUE_LENGTH];
	Entity *e;

	resourceType = ENTITY_DATA;

	if (key == NULL || value == NULL)
	{
		key = (char **)malloc(sizeof(char *) * MAX_PROPS_FILES);
		value = (char **)malloc(sizeof(char *) * MAX_PROPS_FILES);

		if (key == NULL || value == NULL)
		{
			printf("Ran out of memory when loading properties\n");

			exit(1);
		}

		for (i=0;i<MAX_PROPS_FILES;i++)
		{
			key[i] = (char *)malloc(MAX_VALUE_LENGTH);
			value[i] = (char *)malloc(MAX_VALUE_LENGTH);

			if (key[i] == NULL || value[i] == NULL)
			{
				printf("Ran out of memory when loading properties\n");

				exit(1);
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

	while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
	{
		if (line[strlen(line) - 1] == '\n')
		{
			line[strlen(line) - 1] = '\0';
		}

		if (strlen(line) == 0 || line[0] == '#')
		{
			continue;
		}

		sscanf(line, "%s", itemName);

		if (strcmpignorecase(itemName, "MAP_NAME") == 0)
		{
			printf("Encountered Map Data for %s. Returning\n", line);

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
				e = loadPlayer(atoi(value[startX]), atoi(value[startY]));
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

			else if (strcmpignorecase(value[type], "AUTO_DOOR") == 0 || strcmpignorecase(value[type], "MANUAL_DOOR") == 0)
			{
				e = addDoor(value[name], atoi(value[startX]), atoi(value[startY]), getEntityTypeByName(value[type]));
			}

			else if (strcmpignorecase(value[type], "WEAK_WALL") == 0)
			{
				e = addWeakWall(value[name], atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "SWITCH") == 0)
			{
				e = addSwitch(value[name], atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "LINE_DEF") == 0)
			{
				e = addLineDef(value[name], atoi(value[startX]), atoi(value[startY]));
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

			else if (strcmpignorecase(value[type], "OBJECTIVE") == 0)
			{
				addObjectiveFromResource(key, value);
			}

			else
			{
				printf("Unknown Entity type %s\n", value[type]);
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
			token = strtok(line, " ");

			STRNCPY(key[i], token, MAX_VALUE_LENGTH);

			token = strtok(NULL, "\0");

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
	}

	loadInventoryItems();
}
