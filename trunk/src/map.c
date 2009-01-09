#include "map.h"

extern SDL_Surface *loadImage(char *);
extern void drawImage(SDL_Surface *, int, int);
extern void setPlayerLocation(int, int);
extern Mix_Chunk *loadSound(char *);
extern Entity *getPlayer(void);
extern void loadResource(char *);
extern void playSound(Mix_Chunk *, int);
extern long prand(void);

static void loadMapTiles(char *);
static void loadMapBackground(char *name);
static void loadAmbience(char *);

void freeMap(void);

void loadMap(char *name)
{
	int x, y;
	char itemName[255], line[MAX_LINE_LENGTH];
	FILE *fp;

	freeMap();

	fp = fopen(name, "rb");

	/* If we can't open the map then exit */

	if (fp == NULL)
	{
		printf("Failed to open map %s\n", name);

		exit(1);
	}

	/* Read the data from the file into the map */

	while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
	{
		sscanf(line, "%s", itemName);

		if (strcmpignorecase(itemName, "TILESET") == 0)
		{
			/* Load the map tiles */

			sscanf(line, "%*s %s\n", itemName);

			printf("Loading tiles from %s\n", itemName);

			loadMapTiles(itemName);

			strcpy(map.tilesetName, itemName);
		}

		else if (strcmpignorecase(itemName, "AMBIENCE") == 0)
		{
			/* Load the map tiles */

			sscanf(line, "%*s %s\n", itemName);

			printf("Loading ambience from %s\n", itemName);

			loadAmbience(itemName);

			strcpy(map.ambienceName, itemName);
		}

		else if (strcmpignorecase(itemName, "DATA") == 0)
		{
			map.maxX = map.maxY = 0;

			for (y=0;y<MAX_MAP_Y;y++)
			{
				for (x=0;x<MAX_MAP_X;x++)
				{
					fscanf(fp, "%d", &map.tile[y][x]);

					if (map.tile[y][x] > 0)
					{
						if (x > map.maxX)
						{
							map.maxX = x;
						}

						if (y > map.maxY)
						{
							map.maxY = y;
						}
					}
				}
			}
		}

		else
		{
			loadResource(line);
		}
	}

	/* Set the maximum scroll position of the map */

	map.maxX = (map.maxX + 1) * TILE_SIZE;
	map.maxY = (map.maxY + 1) * TILE_SIZE;

	/* Set the start coordinates */

	map.startX = map.startY = 0;

	/* Set the filename */

	strcpy(map.filename, name);

	/* Set the thinkTime */

	map.thinkTime = 120 + prand() % 1200;

	/* Close the file afterwards */

	fclose(fp);
}

void saveMap()
{
	int x, y;
	FILE *fp;

	self = getPlayer();

	if (self->active == INACTIVE)
	{
		printf("No player start defined\n");

		return;
	}

	fp = fopen(map.filename, "wb");

	/* If we can't open the map then exit */

	if (fp == NULL)
	{
		printf("Failed to open map %s\n", map.filename);

		exit(1);
	}

	fprintf(fp, "TILESET %s\n", map.tilesetName);
	fprintf(fp, "AMBIENCE %s\n", map.ambienceName);
	fprintf(fp, "DATA\n");

	/* Write the data from the file into the map */

	for (y=0;y<MAX_MAP_Y;y++)
	{
		for (x=0;x<MAX_MAP_X;x++)
		{
			fprintf(fp, "%d ", map.tile[y][x]);
		}

		fprintf(fp, "\n");
	}

	/* Now write out all of the Entities */

	self = getPlayer();

	fprintf(fp, "player_start player_start %d %d %d %d\n", (int)self->x, (int)self->y, (int)self->endX, (int)self->endY);

	for (x=0;x<MAX_ENTITIES;x++)
	{
		self = &entity[x];

		if (self->active == ACTIVE)
		{
			if (self->type == WEAPON)
			{
				fprintf(fp, "WEAPON %s %d %d %d %d\n", self->name, (int)self->x, (int)self->y, (int)self->endX, (int)self->endY);
			}

			else if (self->type == ITEM)
			{
				fprintf(fp, "ITEM %s %d %d %d %d\n", self->name, (int)self->x, (int)self->y, (int)self->endX, (int)self->endY);
			}

			else if (self->type == KEY_ITEM)
			{
				fprintf(fp, "KEY_ITEM %s %d %d %d %d\n", self->name, (int)self->x, (int)self->y, (int)self->endX, (int)self->endY);
			}

			else if (self->type == LIFT)
			{
				fprintf(fp, "LIFT %s %d %d %d %d\n", self->name, (int)self->x, (int)self->y, (int)self->endX, (int)self->endY);
			}

			else if (self->type == HEALTH)
			{
				fprintf(fp, "HEALTH %s %d %d %d %d\n", self->name, (int)self->x, (int)self->y, (int)self->endX, (int)self->endY);
			}

			else if (self->type == SHIELD)
			{
				fprintf(fp, "SHIELD %s %d %d %d %d\n", self->name, (int)self->x, (int)self->y, (int)self->endX, (int)self->endY);
			}

			else if (self->type == ENEMY)
			{
				fprintf(fp, "ENEMY %s %d %d %d %d\n", self->name, (int)self->x, (int)self->y, (int)self->endX, (int)self->endY);
			}

			else
			{
				fprintf(fp, "UNKNOWN %s %d %d %d %d\n", self->name, (int)self->x, (int)self->y, (int)self->endX, (int)self->endY);
			}
		}
	}

	/* Close the file afterwards */

	fclose(fp);
}

void doMap()
{
	int sound = prand() % MAX_AMBIENT_SOUNDS;

	if (map.hasAmbience == 1)
	{
		map.thinkTime--;

		if (map.thinkTime <= 0)
		{
			while (map.ambience[sound] == NULL)
			{
				sound = prand() % MAX_AMBIENT_SOUNDS;
			}

			playSound(map.ambience[sound], -1);

			map.thinkTime = 120 + prand() % 1200;
		}
	}
}

static void loadMapTiles(char *dir)
{
	int i;
	char filename[255];
	FILE *fp;

	/* Load the blank tile for the editor */

	mapImages[0] = loadImage("gfx/map/0.png");

	for (i=1;i<MAX_TILES;i++)
	{
		sprintf(filename, "gfx/map/%s/%d.png", dir, i);

		fp = fopen(filename, "rb");

		if (fp == NULL)
		{
			continue;
		}

		fclose(fp);

		mapImages[i] = loadImage(filename);
	}

	sprintf(filename, "gfx/map/%s/background.png", dir);

	loadMapBackground(filename);
}

void drawMap()
{
	int x, y, mapX, x1, x2, mapY, y1, y2;

	mapX = map.startX / TILE_SIZE;
	x1 = (map.startX % TILE_SIZE) * -1;
	x2 = x1 + SCREEN_WIDTH + (x1 == 0 ? 0 : TILE_SIZE);

	mapY = map.startY / TILE_SIZE;
	y1 = (map.startY % TILE_SIZE) * -1;
	y2 = y1 + SCREEN_HEIGHT + (y1 == 0 ? 0 : TILE_SIZE);

	/* Draw the background */

	drawImage(map.background, 0, 0);

	/* Draw the map starting at the startX and startY */

	for (y=y1;y<y2;y+=TILE_SIZE)
	{
		mapX = map.startX / TILE_SIZE;

		for (x=x1;x<x2;x+=TILE_SIZE)
		{
			if (map.tile[mapY][mapX] != BLANK_TILE)
			{
				drawImage(mapImages[map.tile[mapY][mapX]], x, y);
			}

			mapX++;
		}

		mapY++;
	}
}

void centerEntityOnMap()
{
	if (map.targetEntity == NULL)
	{
		return;
	}

	map.startX = map.targetEntity->x - (SCREEN_WIDTH / 2);

	if (map.startX < 0)
	{
		map.startX = 0;
	}

	else if (map.startX + SCREEN_WIDTH >= map.maxX)
	{
		map.startX = map.maxX - SCREEN_WIDTH;

		if (map.startX < 0)
		{
			map.startX = 0;
		}
	}

	map.startY = map.targetEntity->y + map.targetEntity->h - (SCREEN_HEIGHT / 2);

	if (map.startY < 0)
	{
		map.startY = 0;
	}

	else if (map.startY + SCREEN_HEIGHT >= map.maxY)
	{
		map.startY = map.maxY - SCREEN_HEIGHT;

		if (map.startY < 0)
		{
			map.startY = 0;
		}
	}
}

static void loadMapBackground(char *name)
{
	/* Load the background image */

	map.background = loadImage(name);

	/* If we get back a NULL image, just exit */

	if (map.background == NULL)
	{
		exit(1);
	}
}

static void loadAmbience(char *dir)
{
	int i, j;
	char filename[MAX_LINE_LENGTH];
	static char *extensions[] = {"ogg", "mp3", "wav"};
	FILE *fp;

	map.hasAmbience = 0;

	for (i=0;i<MAX_AMBIENT_SOUNDS;i++)
	{
		for (j=0;extensions[j]!=NULL;j++)
		{
			sprintf(filename, "sound/ambience/%s/%d.%s", dir, i, extensions[j]);

			fp = fopen(filename, "rb");

			if (fp == NULL)
			{
				continue;
			}

			fclose(fp);

			map.ambience[i] =loadSound(filename);

			map.hasAmbience = 1;

			break;
		}
	}
}

void freeMap()
{
	int i;

	if (map.background != NULL)
	{
		SDL_FreeSurface(map.background);
	}

	/* Free the Map tiles */

	for (i=0;i<MAX_TILES;i++)
	{
		if (mapImages[i] != NULL)
		{
			SDL_FreeSurface(mapImages[i]);
		}
	}

	/* Free the sounds */

	for (i=0;i<MAX_AMBIENT_SOUNDS;i++)
	{
		if (map.ambience[i] != NULL)
		{
			Mix_FreeChunk(map.ambience[i]);
		}
	}
}

SDL_Surface *tileImage(int id)
{
	return mapImages[id];
}

SDL_Surface *mapImageAt(int x, int y)
{
	return mapImages[map.tile[y][x]];
}

int mapTileAt(int x, int y)
{
	return map.tile[y][x];
}

int maxMapX()
{
	return map.maxX;
}

int maxMapY()
{
	return map.maxY;
}

void setMaxMapX(int max)
{
	map.maxX = max;
}

void setMaxMapY(int max)
{
	map.maxY = max;
}

int mapStartX()
{
	return map.startX;
}

int mapStartY()
{
	return map.startY;
}

void mapStartXNext(int val)
{
	map.startX += val;

	if (map.startX < 0)
	{
		map.startX = 0;
	}

	else if (map.startX + SCREEN_WIDTH >= map.maxX)
	{
		map.startX = map.maxX - SCREEN_WIDTH;
	}
}

void mapStartYNext(int val)
{
	map.startY += val;

	if (map.startY < 0)
	{
		map.startY = 0;
	}

	else if (map.startY + SCREEN_HEIGHT >= map.maxY)
	{
		map.startY = map.maxY - SCREEN_HEIGHT;
	}
}

void setTileAt(int x, int y, int tileID)
{
	map.tile[y][x] = tileID;
}

int nextTile(int id)
{
	do
	{
		id++;

		if (id >= MAX_TILES)
		{
			id = 0;
		}
	}

	while (mapImages[id] == NULL);

	return id;
}

int prevTile(int id)
{
	do
	{
		id--;

		if (id < 0)
		{
			id = MAX_TILES - 1;
		}
	}

	while (mapImages[id] == NULL);

	return id;
}

void centerMapOnEntity(Entity *e)
{
	map.targetEntity = e;
}
