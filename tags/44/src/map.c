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

#include "headers.h"

#include "map.h"
#include "system/resources.h"
#include "system/random.h"
#include "graphics/graphics.h"
#include "audio/audio.h"
#include "system/properties.h"
#include "entity.h"
#include "world/target.h"
#include "player.h"
#include "audio/music.h"
#include "game.h"
#include "event/trigger.h"
#include "weather.h"
#include "system/pak.h"
#include "geometry.h"
#include "system/error.h"

static Map map;
static SDL_Surface *mapImages[MAX_TILES];
static int lavaTile, waterTile, slimeTile;

extern Entity *self, player;
extern Game game;

static void loadMapTiles(char *);
static void loadMapBackground(char *name, int);
static void loadAmbience(char *);

static char *extensions[] = {"ogg", "mp3", "wav", NULL};

void loadMap(char *name, int loadEntityResources)
{
	int x, y;
	char itemName[MAX_MESSAGE_LENGTH], filename[MAX_LINE_LENGTH], *line, *token, *savePtr1, *savePtr2;
	unsigned char *buffer;

	savePtr1 = NULL;
	savePtr2 = NULL;

	lavaTile = LAVA_TILE_START;

	slimeTile = SLIME_TILE_START;

	waterTile = WATER_TILE_START + 1;

	snprintf(filename, sizeof(filename), "data/maps/%s.dat", name);

	buffer = loadFileFromPak(filename);

	/* Set the filename */

	STRNCPY(map.filename, name, sizeof(map.filename));

	/* Reset the weather */

	setWeather(NO_WEATHER);

	/* Reset the minimum and maximum scrolling */

	map.minX = MAX_MAP_X;
	map.minY = 0;

	map.maxX = map.maxY = 0;

	map.forceMinY = FALSE;

	/* Reset the clipping */

	map.darkMap = FALSE;

	/* Read the data from the file into the map */

	line = strtok_r((char *)buffer, "\n", &savePtr1);

	while (line != NULL)
	{
		sscanf(line, "%s", itemName);

		if (strcmpignorecase(itemName, "MAP_NAME") == 0)
		{
			sscanf(line, "%*s %[^$]s\n", itemName);

			STRNCPY(map.mapName, itemName, sizeof(map.mapName));
		}

		else if (strcmpignorecase(itemName, "TILESET") == 0)
		{
			/* Load the map tiles */

			sscanf(line, "%*s %s\n", itemName);

			loadMapTiles(itemName);

			STRNCPY(map.tilesetName, itemName, sizeof(map.tilesetName));
		}

		else if (strcmpignorecase(itemName, "BACKGROUND_SPEED") == 0)
		{
			/* Set the background speed */

			sscanf(line, "%*s %s\n", itemName);

			map.backgroundSpeed[0] = atof(itemName);
		}

		else if (strcmpignorecase(itemName, "WRAP_X") == 0)
		{
			/* Set the wrapping */

			sscanf(line, "%*s %s\n", itemName);

			map.wrapX[0] = (strcmpignorecase(itemName, "TRUE") == 0 ? TRUE : FALSE);
		}

		else if (strcmpignorecase(itemName, "WRAP_Y") == 0)
		{
			/* Set the wrapping */

			sscanf(line, "%*s %s\n", itemName);

			map.wrapY[0] = (strcmpignorecase(itemName, "TRUE") == 0 ? TRUE : FALSE);
		}

		else if (strcmpignorecase(itemName, "BACKGROUND_SPEED_2") == 0)
		{
			/* Set the background speed */

			sscanf(line, "%*s %s\n", itemName);

			map.backgroundSpeed[1] = atof(itemName);
		}

		else if (strcmpignorecase(itemName, "WRAP_X_2") == 0)
		{
			/* Set the wrapping */

			sscanf(line, "%*s %s\n", itemName);

			map.wrapX[1] = (strcmpignorecase(itemName, "TRUE") == 0 ? TRUE : FALSE);
		}

		else if (strcmpignorecase(itemName, "WRAP_Y_2") == 0)
		{
			/* Set the wrapping */

			sscanf(line, "%*s %s\n", itemName);

			map.wrapY[1] = (strcmpignorecase(itemName, "TRUE") == 0 ? TRUE : FALSE);
		}

		else if (strcmpignorecase(itemName, "DARK_MAP") == 0)
		{
			/* Set the darkness */

			sscanf(line, "%*s %s\n", itemName);

			map.darkMap = (strcmpignorecase(itemName, "TRUE") == 0 ? TRUE : FALSE);
		}

		else if (strcmpignorecase(itemName, "AMBIENCE") == 0)
		{
			/* Load the ambience */

			sscanf(line, "%*s %s\n", itemName);

			loadAmbience(itemName);

			STRNCPY(map.ambienceName, itemName, sizeof(map.ambienceName));
		}

		else if (strcmpignorecase(itemName, "MUSIC") == 0)
		{
			/* Load the music */

			x = sscanf(line, "%*s %s\n", itemName);

			if (x > 0)
			{
				STRNCPY(map.musicName, itemName, sizeof(map.musicName));

				loadMusic(map.musicName);
			}
		}

		else if (strcmpignorecase(itemName, "WEATHER") == 0)
		{
			/* Set the weather */

			sscanf(line, "%*s %s\n", itemName);

			x = getWeatherTypeByName(itemName);

			setWeather(x);
		}

		else if (strcmpignorecase(itemName, "MIN_Y") == 0)
		{
			/* The the map's minimum Y */

			sscanf(line, "%*s %s\n", itemName);

			map.forceMinY = (strcmpignorecase(itemName, "TRUE") == 0 ? TRUE : FALSE);

			if (map.forceMinY == TRUE)
			{
				map.minY = MAX_MAP_Y;
			}
		}

		else if (strcmpignorecase(itemName, "DATA") == 0)
		{
			line = strtok_r(NULL, "\n", &savePtr1);

			map.maxX = map.maxY = 0;

			for (y=0;y<MAX_MAP_Y;y++)
			{
				token = strtok_r(line, " ", &savePtr2);

				for (x=0;x<MAX_MAP_X;x++)
				{
					map.tile[y][x] = atoi(token);
					/*
					if (map.tile[y][x] != 0 && map.tile[y][x] <= 4)
					{
						map.tile[y][x] = 1 + (prand() % 4);
					}
					*/
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

						if (x < map.minX)
						{
							map.minX = x;
						}

						if (map.forceMinY == TRUE && y < map.minY)
						{
							map.minY = y;
						}
					}

					token = strtok_r(NULL, " ", &savePtr2);
				}

				if (y + 1 != MAX_MAP_Y)
				{
					line = strtok_r(NULL, "\n", &savePtr1);
				}
			}

			/*fgets(line, MAX_LINE_LENGTH, fp);*/
		}

		else if (loadEntityResources == TRUE && strcmpignorecase(itemName, "{") == 0)
		{
			loadResources(savePtr1);
		}

		else if (loadEntityResources == TRUE && strcmpignorecase(itemName, "}") == 0)
		{
			showErrorAndExit("Parse error: encountered closing } without matching {");
		}

		line = strtok_r(NULL, "\n", &savePtr1);
	}

	/* Set the maximum scroll position of the map */

	map.maxX = (map.maxX + 1) * TILE_SIZE;
	map.maxY = (map.maxY + 1) * TILE_SIZE;

	/* Set the minimum scroll position of the map */

	map.minX  = map.minX * TILE_SIZE;
	map.minY  = map.minY * TILE_SIZE;

	/* Set the start coordinates */

	map.startX = map.startY = 0;

	/* Set the thinkTime */

	map.thinkTime = 3;

	/* Close the file afterwards */

	free(buffer);

	setTransition(TRANSITION_IN, NULL);

	playMapMusic();

	resetCameraLimits();

	cameraSnapToTargetEntity();
}

int saveMap()
{
	int x, y;
	char filename[MAX_LINE_LENGTH];
	FILE *fp;

	self = &player;

	if (self->inUse == FALSE)
	{
		printf("No player start defined\n");

		return FALSE;
	}

	snprintf(filename, sizeof(filename), "data/maps/%s.dat", map.filename);

	printf("Saving map to %s\n", filename);

	fp = fopen(filename, "wb");

	/* If we can't open the map then exit */

	if (fp == NULL)
	{
		showErrorAndExit("Failed to open map %s", map.filename);
	}

	fprintf(fp, "MAP_NAME %s\n", map.mapName);
	fprintf(fp, "MUSIC %s\n", map.musicName);
	fprintf(fp, "TILESET %s\n", map.tilesetName);
	fprintf(fp, "AMBIENCE %s\n", map.ambienceName);
	fprintf(fp, "BACKGROUND_SPEED %0.1f\n", map.backgroundSpeed[0]);
	fprintf(fp, "WRAP_X %s\n", map.wrapX[0] == TRUE ? "TRUE" : "FALSE");
	fprintf(fp, "WRAP_Y %s\n", map.wrapY[0] == TRUE ? "TRUE" : "FALSE");
	fprintf(fp, "BACKGROUND_SPEED_2 %0.1f\n", map.backgroundSpeed[1]);
	fprintf(fp, "WRAP_X_2 %s\n", map.wrapX[1] == TRUE ? "TRUE" : "FALSE");
	fprintf(fp, "WRAP_Y_2 %s\n", map.wrapY[1] == TRUE ? "TRUE" : "FALSE");
	fprintf(fp, "WEATHER %s\n", getWeather());
	fprintf(fp, "MIN_Y %s\n", map.forceMinY == TRUE ? "TRUE" : "FALSE");
	fprintf(fp, "DARK_MAP %s\n", map.darkMap == TRUE ? "TRUE" : "FALSE");
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

	writePlayerMapStartToFile(fp);

	writeEntitiesToFile(fp);

	/* Now the targets */

	writeTargetsToFile(fp);

	/* And the triggers */

	writeTriggersToFile(fp);

	/* Close the file afterwards */

	fclose(fp);

	return TRUE;
}

void doMap()
{
	map.thinkTime--;

	if (map.thinkTime <= 0)
	{
		lavaTile++;

		if (lavaTile > LAVA_TILE_END)
		{
			lavaTile = LAVA_TILE_START;
		}

		waterTile++;

		if (waterTile > WATER_TILE_END)
		{
			waterTile = WATER_TILE_START + 1;
		}

		slimeTile++;

		if (slimeTile > SLIME_TILE_END)
		{
			slimeTile = SLIME_TILE_START;
		}

		map.thinkTime = 3;
	}
	
	map.blendTime -= 3;
	
	if (map.blendTime < 0)
	{
		map.blendTime = 0;
	}
}

static void loadMapTiles(char *dir)
{
	int i;
	char filename[255];

	/* Load the blank tile for the editor */

	mapImages[0] = loadImage("gfx/map/0.png");

	for (i=1;i<MAX_TILES;i++)
	{
		snprintf(filename, sizeof(filename), "gfx/map/%s/%d.png", dir, i);

		if (existsInPak(filename) == FALSE)
		{
			continue;
		}

		mapImages[i] = loadImage(filename);
	}

	snprintf(filename, sizeof(filename), "gfx/map/%s/background.png", dir);

	loadMapBackground(filename, 0);

	snprintf(filename, sizeof(filename), "gfx/map/%s/background1.png", dir);

	loadMapBackground(filename, 1);
}

void drawMap(int depth)
{
	int x, y, mapX, x1, x2, mapY, y1, y2, tileID;

	/* Draw the background */

	if (depth <= 0)
	{
		map.backgroundStartX[0] = map.startX * map.backgroundSpeed[0];
		map.backgroundStartY[0] = map.startY * map.backgroundSpeed[0];

		if (map.backgroundStartX[0] + SCREEN_WIDTH > map.background[0]->w && map.wrapX[0] == FALSE)
		{
			map.backgroundStartX[0] = map.background[0]->w - SCREEN_WIDTH;
		}

		if (map.backgroundStartY[0] + SCREEN_HEIGHT > map.background[0]->h && map.wrapY[0] == FALSE)
		{
			map.backgroundStartY[0] = map.background[0]->h - SCREEN_HEIGHT;
		}

		if (map.wrapX == FALSE && map.wrapY == FALSE)
		{
			drawClippedImage(map.background[0], map.backgroundStartX[0], map.backgroundStartY[0], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		}

		else if (map.wrapX[0] == TRUE && map.wrapY[0] == FALSE)
		{
			x = map.backgroundStartX[0] % map.background[0]->w;

			drawClippedImage(map.background[0], x, map.backgroundStartY[0], 0, 0, map.background[0]->w - x, SCREEN_HEIGHT);

			drawClippedImage(map.background[0], 0, map.backgroundStartY[0], map.background[0]->w - x, 0, x, SCREEN_HEIGHT);
		}

		else if (map.wrapX[0] == FALSE && map.wrapY[0] == TRUE)
		{
			y = map.backgroundStartY[0] % map.background[0]->h;

			drawClippedImage(map.background[0], map.backgroundStartX[0], y, 0, 0, SCREEN_WIDTH, map.background[0]->h - y);

			drawClippedImage(map.background[0], map.backgroundStartX[0], 0, 0, map.background[0]->h - y, SCREEN_WIDTH, y);
		}

		else
		{
			x = map.backgroundStartX[0] % map.background[0]->w;
			y = map.backgroundStartY[0] % map.background[0]->h;

			drawClippedImage(map.background[0], x, y, 0, 0, map.background[0]->w - x, map.background[0]->h - y);

			drawClippedImage(map.background[0], 0, y, map.background[0]->w - x, 0, x, map.background[0]->h - y);

			drawClippedImage(map.background[0], x, 0, 0, map.background[0]->h - y, map.background[0]->w - x, y);

			drawClippedImage(map.background[0], 0, 0, map.background[0]->w - x, map.background[0]->h - y, x, y);
		}

		if (map.background[1] != NULL)
		{
			map.backgroundStartX[1] = map.startX * map.backgroundSpeed[1];
			map.backgroundStartY[1] = map.startY * map.backgroundSpeed[1];

			if (map.backgroundStartX[1] + SCREEN_WIDTH > map.background[1]->w && map.wrapX[1] == FALSE)
			{
				map.backgroundStartX[1] = map.background[1]->w - SCREEN_WIDTH;
			}

			if (map.backgroundStartY[1] + SCREEN_HEIGHT > map.background[1]->h && map.wrapY[1] == FALSE)
			{
				map.backgroundStartY[1] = map.background[1]->h - SCREEN_HEIGHT;
			}

			if (map.wrapX[1] == FALSE && map.wrapY[1] == FALSE)
			{
				drawClippedImage(map.background[1], map.backgroundStartX[1], map.backgroundStartY[1], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			}

			else if (map.wrapX[1] == TRUE && map.wrapY[1] == FALSE)
			{
				x = map.backgroundStartX[1] % map.background[1]->w;

				drawClippedImage(map.background[1], x, map.backgroundStartY[1], 0, 0, map.background[1]->w - x, SCREEN_HEIGHT);

				drawClippedImage(map.background[1], 0, map.backgroundStartY[1], map.background[1]->w - x, 0, x, SCREEN_HEIGHT);
			}

			else if (map.wrapX[1] == FALSE && map.wrapY[1] == TRUE)
			{
				y = map.backgroundStartY[1] % map.background[1]->h;

				drawClippedImage(map.background[1], map.backgroundStartX[1], y, 0, 0, SCREEN_WIDTH, map.background[1]->h - y);

				drawClippedImage(map.background[1], map.backgroundStartX[1], 0, 0, map.background[1]->h - y, SCREEN_WIDTH, y);
			}

			else
			{
				x = map.backgroundStartX[1] % map.background[1]->w;
				y = map.backgroundStartY[1] % map.background[1]->h;

				drawClippedImage(map.background[1], x, y, 0, 0, map.background[1]->w - x, map.background[1]->h - y);

				drawClippedImage(map.background[1], 0, y, map.background[1]->w - x, 0, x, map.background[1]->h - y);

				drawClippedImage(map.background[1], x, 0, 0, map.background[1]->h - y, map.background[1]->w - x, y);

				drawClippedImage(map.background[1], 0, 0, map.background[1]->w - x, map.background[1]->h - y, x, y);
			}
		}
	}

	mapX = map.startX / TILE_SIZE;
	x1 = (map.startX % TILE_SIZE) * -1;
	x2 = x1 + SCREEN_WIDTH + (x1 == 0 ? 0 : TILE_SIZE);

	mapY = map.startY / TILE_SIZE;
	y1 = (map.startY % TILE_SIZE) * -1;
	y2 = y1 + SCREEN_HEIGHT + (y1 == 0 ? 0 : TILE_SIZE);

	/* Draw the map starting at the startX and startY */

	for (y=y1;y<y2;y+=TILE_SIZE)
	{
		mapX = map.startX / TILE_SIZE;

		for (x=x1;x<x2;x+=TILE_SIZE)
		{
			switch (map.tile[mapY][mapX])
			{
				case LAVA_TILE_START:
					tileID = lavaTile;
				break;

				case SLIME_TILE_START:
					tileID = slimeTile;
				break;

				case WATER_TILE_START + 1:
					tileID = waterTile;
				break;

				default:
					tileID = map.tile[mapY][mapX];
				break;
			}

			if (tileID == BLANK_TILE)
			{
				mapX++;

				continue;
			}
			/*
			if (mapImages[tileID] == NULL)
			{
				printf("Tile %d is NULL\n", tileID);

				exit(0);
			}
			*/

			switch (depth)
			{
				case 0:
					if (tileID >= BACKGROUND_TILE_START && tileID <= BACKGROUND_TILE_END)
					{
						drawImage(mapImages[tileID], x, y, FALSE, 255);
					}
				break;

				case 1:
					if (tileID < BACKGROUND_TILE_START)
					{
						drawImage(mapImages[tileID], x, y, FALSE, 255);
					}
				break;

				case 2:
					if (tileID == SLIME_TILE_BLEND)
					{
						drawImage(mapImages[WATER_TILE_START], x, y, FALSE, 128);
						drawImage(mapImages[slimeTile], x, y, FALSE, map.blendTime);
						
						if (map.blendTime == 0)
						{
							map.tile[mapY][mapX] = WATER_TILE_START;
						}
					}
					
					else if (tileID >= FOREGROUND_TILE_START)
					{
						drawImage(mapImages[tileID], x, y, FALSE, tileID >= WATER_TILE_START && tileID <= WATER_TILE_END ? 128 : 255);
					}
				break;

				default:
					drawImage(mapImages[tileID], x, y, FALSE, 255);
				break;
			}

			mapX++;
		}

		mapY++;
	}
}

void centerEntityOnMap()
{
	float speed;

	if (map.targetEntity == NULL)
	{
		return;
	}

	if (map.targetEntity->standingOn != NULL && fabs(map.targetEntity->standingOn->speed) > fabs(map.targetEntity->speed))
	{
		speed = map.targetEntity->standingOn->speed;

		if (speed < fabs(map.targetEntity->dirX))
		{
			speed = fabs(map.targetEntity->dirX);
		}

		if (speed < fabs(map.targetEntity->standingOn->dirY))
		{
			speed = fabs(map.targetEntity->standingOn->dirY);
		}
	}

	else
	{
		speed = map.targetEntity->originalSpeed > map.targetEntity->speed ? map.targetEntity->originalSpeed : map.targetEntity->speed;

		if (speed < fabs(map.targetEntity->dirX))
		{
			speed = fabs(map.targetEntity->dirX);
		}

		if (speed < fabs(map.targetEntity->dirY))
		{
			speed = fabs(map.targetEntity->dirY);
		}
	}

	if (map.cameraSpeed != -1)
	{
		speed = map.cameraSpeed;
	}

	map.startX = map.targetEntity->x - (SCREEN_WIDTH / 2);

	if (map.startX < (map.minX != map.cameraMinX ? map.cameraMinX : map.minX))
	{
		map.startX = (map.minX != map.cameraMinX ? map.cameraMinX : map.minX);
	}

	else if (map.startX + SCREEN_WIDTH >= (map.maxX != map.cameraMaxX ? map.cameraMaxX : map.maxX))
	{
		map.startX = (map.maxX != map.cameraMaxX ? map.cameraMaxX : map.maxX) - SCREEN_WIDTH;

		if (map.startX < (map.minX != map.cameraMinX ? map.cameraMinX : map.minX))
		{
			map.startX = (map.minX != map.cameraMinX ? map.cameraMinX : map.minX);
		}
	}

	map.startY = map.targetEntity->y + map.targetEntity->h - SCREEN_HEIGHT / 1.5;

	if (map.startY < (map.minY != map.cameraMinY ? map.cameraMinY : map.minY))
	{
		map.startY = (map.minY != map.cameraMinY ? map.cameraMinY : map.minY);
	}

	else if (map.startY + SCREEN_HEIGHT >= (map.maxY != map.cameraMaxY ? map.cameraMaxY : map.maxY))
	{
		map.startY = (map.maxY != map.cameraMaxY ? map.cameraMaxY : map.maxY) - SCREEN_HEIGHT;

		if (map.startY < (map.minY != map.cameraMinY ? map.cameraMinY : map.minY))
		{
			map.startY = (map.minY != map.cameraMinY ? map.cameraMinY : map.minY);
		}
	}

	if (abs(map.cameraX - map.startX) > speed)
	{
		map.cameraX += map.cameraX < map.startX ? speed : -speed;
	}

	else
	{
		map.cameraX = map.startX;
	}

	if (abs(map.cameraY - map.startY) > speed)
	{
		if (map.cameraY < map.startY)
		{
			map.cameraY += (map.targetEntity->dirY > speed ? map.targetEntity->dirY : speed);
		}

		else
		{
			map.cameraY += (map.targetEntity->dirY < -speed ? map.targetEntity->dirY : -speed);
		}
	}

	else
	{
		map.cameraY = map.startY;
	}

	map.startX = map.cameraX;
	map.startY = map.cameraY;
}

static void loadMapBackground(char *name, int index)
{
	/* Load the background image */

	if (index == 0)
	{
		map.background[index] = loadImage(name);
	}

	else
	{
		if (existsInPak(name) == FALSE)
		{
			return;
		}

		map.background[index] = loadImage(name);
	}
}

static void loadAmbience(char *dir)
{
	int i, j;
	char filename[MAX_PATH_LENGTH];

	map.hasAmbience = FALSE;

	if (game.audio == FALSE)
	{
		return;
	}

	for (i=0;i<MAX_AMBIENT_SOUNDS;i++)
	{
		for (j=0;extensions[j]!=NULL;j++)
		{
			snprintf(filename, sizeof(filename), "ambience/%s/%d.%s", dir, i, extensions[j]);

			if (existsInPak(filename) == FALSE)
			{
				continue;
			}

			map.ambience[i] = loadSound(filename);

			map.hasAmbience = TRUE;

			break;
		}
	}
}

void freeMap()
{
	int i;

	if (map.background[0] != NULL)
	{
		SDL_FreeSurface(map.background[0]);

		map.background[0] = NULL;
	}

	if (map.background[1] != NULL)
	{
		SDL_FreeSurface(map.background[1]);

		map.background[1] = NULL;
	}

	/* Free the Map tiles */

	for (i=0;i<MAX_TILES;i++)
	{
		if (mapImages[i] != NULL)
		{
			SDL_FreeSurface(mapImages[i]);

			mapImages[i] = NULL;
		}
	}

	/* Free the sounds */

	for (i=0;i<MAX_AMBIENT_SOUNDS;i++)
	{
		if (map.ambience[i] != NULL)
		{
			Mix_FreeChunk(map.ambience[i]);

			map.ambience[i] = NULL;
		}
	}

	memset(&map, 0, sizeof(Map));
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
	if (x < 0 || x >= MAX_MAP_X || y < 0 || y >= MAX_MAP_Y)
	{
		return BLANK_TILE;
	}

	return map.tile[y][x];
}

int getMaxMapX()
{
	return map.maxX;
}

int getMaxMapY()
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

void setMinMapX(int min)
{
	map.minX = min;
}

void setMinMapY(int min)
{
	map.minY = min;
}

int getMinMapX()
{
	return map.minX;
}

int getMinMapY()
{
	return map.minY;
}

int getMapStartX()
{
	return map.startX;
}

int getMapStartY()
{
	return map.startY;
}

void setMapStartX(int startX)
{
	map.startX = startX;

	if (map.startX < 0)
	{
		map.startX = 0;
	}

	else if (map.startX + SCREEN_WIDTH >= map.maxX)
	{
		map.startX = map.maxX - SCREEN_WIDTH;
	}
}

void setMapStartY(int startY)
{
	map.startY = startY;

	if (map.startY < 0)
	{
		map.startY = 0;
	}

	else if (map.startY + SCREEN_HEIGHT >= map.maxY)
	{
		map.startY = map.maxY - SCREEN_HEIGHT;
	}
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

void cameraSnapToTargetEntity()
{
	if (map.targetEntity == NULL)
	{
		return;
	}

	map.startX = map.targetEntity->x - (SCREEN_WIDTH / 2);

	if (map.startX < map.minX)
	{
		map.startX = map.minX;
	}

	else if (map.startX + SCREEN_WIDTH >= map.maxX)
	{
		map.startX = map.maxX - SCREEN_WIDTH;

		if (map.startX < map.minX)
		{
			map.startX = map.minX;
		}
	}

	map.startY = map.targetEntity->y + map.targetEntity->h - SCREEN_HEIGHT / 1.5;

	if (map.startY < map.minY)
	{
		map.startY = map.minY;
	}

	else if (map.startY + SCREEN_HEIGHT >= map.maxY)
	{
		map.startY = map.maxY - SCREEN_HEIGHT;

		if (map.startY < map.minY)
		{
			map.startY = map.minY;
		}
	}

	map.cameraX = map.startX;
	map.cameraY = map.startY;
}

void setCameraPosition(int x, int y)
{
	map.cameraX = x;
	map.cameraY = y;
}

void limitCamera(int minX, int minY, int maxX, int maxY)
{
	map.cameraMinX = minX;
	map.cameraMinY = minY;

	map.cameraMaxX = maxX;
	map.cameraMaxY = maxY;
}

void limitCameraFromScript(char *line)
{
	sscanf(line, "%d %d %d %d", &map.cameraMinX, &map.cameraMinY, &map.cameraMaxX, &map.cameraMaxY);
}

void resetCameraLimits()
{
	map.cameraMinX = map.minX;
	map.cameraMinY = map.minY;

	map.cameraMaxX = map.maxX;
	map.cameraMaxY = map.maxY;
}

int cameraAtMinimum()
{
	return (map.cameraMinX == map.startX && map.cameraMinY == map.startY);
}

void setCameraSpeed(float speed)
{
	map.cameraSpeed = speed;
}

void centerMapOnEntity(Entity *e)
{
	map.targetEntity = e;

	map.cameraSpeed = -1;
}

char *getMapFilename()
{
	return map.filename;
}

char *getMapMusic()
{
	return map.musicName;
}

char *getMapName()
{
	return map.mapName;
}

int getDistanceFromCamera(int x, int y)
{
	return getDistance(map.cameraX + SCREEN_WIDTH / 2, map.cameraY + SCREEN_HEIGHT / 2, x, y);
}

int outOfBounds(Entity *e)
{
	if (e->x < map.startX || e->x > map.startX + SCREEN_WIDTH || e->y < map.startY || e->y > map.startY + SCREEN_HEIGHT)
	{
		return TRUE;
	}

	return FALSE;
}

int mapIsDark()
{
	return map.darkMap;
}

void resetBlendTime()
{
	map.blendTime = 255;
}
