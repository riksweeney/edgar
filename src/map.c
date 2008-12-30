#include "map.h"

extern SDL_Surface *loadImage(char *);
extern void drawImage(SDL_Surface *, int, int);
extern void setPlayerLocation(int, int);
extern void loadMapTiles(char *);

void loadMapTiles(char *);

void loadMap(char *name)
{
	int x, y;
	char itemName[255], line[MAX_LINE_LENGTH];
	FILE *fp;

	fp = fopen(name, "rb");

	/* If we can't open the map then exit */

	if (fp == NULL)
	{
		printf("Failed to open map %s\n", name);

		exit(1);
	}

	/* Read the data from the file into the map */
	
	while (!feof(fp))
	{
		fgets(line, MAX_LINE_LENGTH, fp);
		
		sscanf(line, "%s", itemName);
		
		if (strcmp(itemName, "TILESET") == 0)
		{
			/* Load the map tiles */
			
			sscanf(line, "%*s %s\n", itemName);
			
			printf("Loading tiles from %s\n", itemName);
			
			loadMapTiles(itemName);
		}
		
		else if (strcmp(itemName, "DATA") == 0)
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
		
		/* Set the maximum scroll position of the map */
		
		map.maxX = (map.maxX + 1) * TILE_SIZE;
		map.maxY = (map.maxY + 1) * TILE_SIZE;
		
		/* Set the start coordinates */
		
		map.startX = map.startY = 0;
		
		/* Set the filename */
		
		map.filename = name;
	}

	/* Close the file afterwards */

	fclose(fp);
}

void saveMap()
{
	int x, y;
	FILE *fp;

	fp = fopen(map.filename, "wb");

	/* If we can't open the map then exit */

	if (fp == NULL)
	{
		printf("Failed to open map %s\n", map.filename);

		exit(1);
	}

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
	
	for (x=0;x<MAX_ENTITIES;x++)
	{
		if (entity[x].active == ACTIVE)
		{
			switch (entity[x].type)
			{
				case SPANNER:
					fprintf(fp, "SPANNER ");
				break;
				
				default:
					fprintf(fp, "UNKNOWN ");
				break;
			}
			
			fprintf(fp, "%d %d\n", (int)entity[x].x / TILE_SIZE, (int)entity[x].y / TILE_SIZE);
		}
	}

	/* Close the file afterwards */

	fclose(fp);
}

void loadMapTiles(char *dir)
{
	int i;
	char filename[255];
	FILE *fp;
	
	for (i=0;i<MAX_TILES;i++)
	{
		sprintf(filename, "gfx/map/%s/%d.png", dir, i);
		
		fp = fopen(filename, "rb");
		
		if (fp == NULL)
		{
			continue;
		}
		
		fclose(fp);
		
		mapImages[i] = loadImage(filename);
		
		if (mapImages[i] == NULL)
		{
			exit(1);
		}
	}
}

void freeMapTiles()
{
	int i;
	
	for (i=0;i<MAX_TILES;i++)
	{
		if (mapImages[i] != NULL)
		{
			SDL_FreeSurface(mapImages[i]);
		}
	}
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

void centerEntityOnMap(Entity *e)
{
	map.startX = e->x - (SCREEN_WIDTH / 2);
	
	if (map.startX < 0)
	{
		map.startX = 0;
	}
	
	else if (map.startX + SCREEN_WIDTH >= map.maxX)
	{
		map.startX = map.maxX - SCREEN_WIDTH;
	}
	
	map.startY = e->y + e->h - (SCREEN_HEIGHT / 2);
	
	if (map.startY < 0)
	{
		map.startY = 0;
	}
	
	else if (map.startY + SCREEN_HEIGHT >= map.maxY)
	{
		map.startY = map.maxY - SCREEN_HEIGHT;
	}
}

void loadMapBackground(char *name)
{
	/* Load the background image */
	
	map.background = loadImage(name);
	
	/* If we get back a NULL image, just exit */
	
	if (map.background == NULL)
	{
		exit(1);
	}
}

void freeMap()
{
	if (map.background != NULL)
	{
		SDL_FreeSurface(map.background);
	}
	
	/* Free the Map tiles */
	
	freeMapTiles();
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

int mapStartX()
{
	return map.startX;
}

int mapStartY()
{
	return map.startY;
}

