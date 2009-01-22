#include "resources.h"

extern void loadMapTiles(void);
extern void loadMapBackground(char *);
extern void loadMap(char *);
extern void loadBasicWeapons(void);
extern TTF_Font *loadFont(char *, int);
extern void freeSprites(void);
extern void freeMap(void);
extern void freeAnimations(void);
extern void freeSounds(void);
extern void loadPlayer(int, int);
extern void initHud(void);
extern void freeHud(void);

extern Entity *addKeyItem(char *, int, int);
extern Entity *addPermanentItem(char *, int, int);
extern Entity *addLift(char *, int, int);
extern Entity *addEnemy(char *, int, int);

extern void setProperty(Entity *, char *, char *);

void loadRequiredResources()
{
	/* Load the map */

	loadMap(INSTALL_PATH"data/maps/map02.dat");

	/* Load the hud */

	initHud();

	/* Load the font */

	game.font = loadFont("font/blackWolf.ttf", 16);
}

void freeRequiredResources()
{
	/* Free the animations */

	freeAnimations();

	/* Free the sounds */

	freeSounds();

	/* Free the map data */

	freeMap();

	/* Free the sprites */

	freeSprites();

	/* Free the hud */

	freeHud();
}

void loadResources(FILE *fp)
{
	int i, startX, startY, type, name;
	char key[MAX_PROPS_FILES][30], value[MAX_PROPS_FILES][MAX_LINE_LENGTH], line[MAX_LINE_LENGTH];
	char *token;
	Entity *e;

	i = 0;

	name = type = startX = startY = -1;

	e = NULL;

	while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
	{
		line[strlen(line) - 1] = '\0';

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
			if (type == -1 || startX == -1 || startY == -1 || name == -1)
			{
				printf("Property is missing basic values:\n");

				for (i=0;i<MAX_PROPS_FILES;i++)
				{
					printf("%s = %s\n", key[i], value[i]);
				}

				exit(1);
			}

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

			else if (strcmpignorecase(value[type], "LIFT") == 0)
			{
				e = addLift(value[name], atoi(value[startX]), atoi(value[startY]));
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
			
			memset(key, 0, sizeof(key));
			
			memset(value, 0, sizeof(value));
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
}
