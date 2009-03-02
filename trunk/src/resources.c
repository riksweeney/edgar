#include "headers.h"

extern Game game;

#include "animation.h"
#include "audio.h"
#include "map.h"
#include "sprites.h"
#include "hud.h"
#include "font.h"
#include "player.h"
#include "enemies.h"
#include "lift.h"
#include "target.h"
#include "item.h"
#include "properties.h"
#include "key_items.h"
#include "spawner.h"
#include "pressure_plate.h"
#include "door.h"
#include "weak_wall.h"
#include "switch.h"
#include "line_def.h"
#include "music.h"
#include "entity.h"
#include "level_exit.h"
#include "decoration.h"
#include "trigger.h"
#include "save_point.h"

void loadRequiredResources()
{
	/* Load the hud */

	initHud();

	/* Load the font */

	game.font = loadFont("font/blackWolf.ttf", 16);
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

	/* Free the properties */

	freeProperties();

	/* Free the HUD messages */

	freeHudMessages();
}

void freeAllResources()
{
	freeLevelResources();

	/* Free the hud */

	freeHud();

	/* Free the font */

	closeFont(game.font);
}

void loadResources(FILE *fp)
{
	int i, startX, startY, type, name;
	char **key, **value, line[MAX_LINE_LENGTH];
	char *token;
	Entity *e;
	
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

		if (strcmpignorecase(line, "{") == 0)
		{
			i = 0;

			name = type = startX = startY = -1;

			e = NULL;
		}

		else if (strcmpignorecase(line, "}") == 0)
		{
			e = NULL;

			if (strcmpignorecase(value[type], "ITEM") == 0 || strcmpignorecase(value[type], "HEALTH") == 0 ||
				strcmpignorecase(value[type], "SHIELD") == 0 || strcmpignorecase(value[type], "WEAPON") == 0)
			{
				e = addPermanentItem(value[name], atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "player_start") == 0)
			{
				loadPlayer(atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "KEY_ITEM") == 0)
			{
				e = addKeyItem(value[name], atoi(value[startX]), atoi(value[startY]));
			}

			else if (strcmpignorecase(value[type], "ENEMY") == 0)
			{
				e = addEnemy(value[name], atoi(value[startX]), atoi(value[startY]));
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
			}

			for (i=0;i<MAX_PROPS_FILES;i++)
			{
				key[i][0] = '\0';
				
				value[i][0] = '\0';
			}
		}

		else
		{
			token = strtok(line, " ");

			strcpy(key[i], token);

			token = strtok(NULL, "\0");

			if (token != NULL)
			{
				strcpy(value[i], token);
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
	
	for (i=0;i<MAX_PROPS_FILES;i++)
	{
		free(key[i]);
		free(value[i]);
	}
	
	free(key);
	free(value);
}
