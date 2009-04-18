#include "headers.h"

#include "graphics/animation.h"
#include "entity.h"
#include "system/properties.h"
#include "map.h"
#include "player.h"
#include "collisions.h"
#include "world/target.h"
#include "graphics/graphics.h"
#include "status.h"

extern Cursor cursor;
extern Input input;
extern Entity *self;

static char entityNames[255][MAX_VALUE_LENGTH];
static int entityNamesLength = 0;
static int targetID = 1;

void initCursor(char *name)
{
	char line[MAX_LINE_LENGTH];
	FILE *fp;

	cursor.tileID = 0;
	cursor.entityType = 0;
	cursor.entity.weight = 0;

	loadProperties(entityNames[0], &cursor.entity);

	cursor.entity.active = TRUE;

	if (cursor.entity.weight == 0)
	{
		cursor.entity.weight = 1;
	}

	cursor.entity.draw = &drawLoopingAnimationToMap;

	snprintf(line, sizeof(line), "%sdata/cursor/%s.dat", INSTALL_PATH, name);

	fp = fopen(line, "rb");

	if (fp == NULL)
	{
		printf("Failed to open cursor data file %s\n", line);

		exit(1);
	}

	while (fgets(entityNames[entityNamesLength], MAX_VALUE_LENGTH, fp) != NULL)
	{
		if (entityNames[entityNamesLength][strlen(entityNames[entityNamesLength]) - 1] == '\n')
		{
			entityNames[entityNamesLength][strlen(entityNames[entityNamesLength]) - 1] = '\0';
		}

		entityNamesLength++;
	}

	fclose(fp);
}

void doCursor()
{
	char name[MAX_VALUE_LENGTH];
	int x, y;

	cursor.x = input.mouseX;
	cursor.y = input.mouseY;

	if (cursor.type == TILES || cursor.snapToGrid == 1)
	{
		cursor.x /= TILE_SIZE;
		cursor.y /= TILE_SIZE;

		cursor.x *= TILE_SIZE;
		cursor.y *= TILE_SIZE;
	}

	if (cursor.y >= SCREEN_HEIGHT - TILE_SIZE)
	{
		cursor.y = SCREEN_HEIGHT - TILE_SIZE * 2;
	}

	if (input.left == 1)
	{
		mapStartXNext(-TILE_SIZE);
	}

	else if (input.right == 1)
	{
		mapStartXNext(TILE_SIZE);
	}

	if (input.up == 1)
	{
		mapStartYNext(-TILE_SIZE);
	}

	else if (input.down == 1)
	{
		mapStartYNext(TILE_SIZE);
	}

	if (input.snap == 1)
	{
		cursor.snapToGrid = 1 - cursor.snapToGrid;

		input.snap = 0;
	}

	if (input.activate == 1)
	{
		cursor.entity.face = (cursor.entity.face == RIGHT ? LEFT : RIGHT);

		input.activate = 0;
	}
	
	if (input.block == 1)
	{
		if (cursor.type == TILES)
		{
			x = (getMapStartX() + cursor.x) / TILE_SIZE;
			y = (getMapStartY() + cursor.y) / TILE_SIZE;
			
			while (mapTileAt(x, y) == BLANK_TILE && x >= 0)
			{
				setTileAt(x, y, cursor.tileID);
				
				x--;
			}
			
			x = (getMapStartX() + cursor.x) / TILE_SIZE;
			
			x++;
			
			while (mapTileAt(x, y) == BLANK_TILE && x < MAX_MAP_X)
			{
				setTileAt(x, y, cursor.tileID);
				
				x++;
			}
		}
	}

	if (input.add == 1)
	{
		if (cursor.type == TILES)
		{
			setTileAt((getMapStartX() + cursor.x) / TILE_SIZE, (getMapStartY() + cursor.y) / TILE_SIZE, cursor.tileID);
		}

		else
		{
			/* Entities can only be placed in blank spaces */

			if (isValidOnMap(&cursor.entity) == 1 && isSpaceEmpty(&cursor.entity) == NULL)
			{
				if (cursor.entityType == 0)
				{
					setPlayerLocation(getMapStartX() + cursor.x, getMapStartY() + cursor.y);
				}

				else
				{
					if (strcmpignorecase(cursor.entity.name, "lift/lift_target") == 0)
					{
						snprintf(name, sizeof(name), "NEW_TARGET_%03d", targetID);

						addTarget(getMapStartX() + cursor.x, getMapStartY() + cursor.y, name);

						targetID++;
					}

					else
					{
						addEntity(cursor.entity, getMapStartX() + cursor.x, getMapStartY() + cursor.y);
					}
				}
			}

			input.add = 0;
		}
	}

	else if (input.remove == 1)
	{
		if (cursor.type == TILES)
		{
			setTileAt((getMapStartX() + cursor.x) / TILE_SIZE, (getMapStartY() + cursor.y) / TILE_SIZE, BLANK_TILE);
		}

		else
		{
			self = isSpaceEmpty(&cursor.entity);

			if (self != NULL)
			{
				self->inUse = FALSE;
			}
		}
	}

	if (input.cut == 1)
	{
		if (cursor.type != TILES)
		{
			self = isSpaceEmpty(&cursor.entity);

			if (self != NULL)
			{
				cursor.entity = *self;

				self->inUse = FALSE;
			}
		}

		input.cut = 0;
	}

	if (input.previous == 1)
	{
		if (cursor.type == TILES)
		{
			cursor.tileID = prevTile(cursor.tileID);
		}

		else
		{
			cursor.entityType--;

			if (cursor.entityType < 0)
			{
				cursor.entityType = entityNamesLength - 1;
			}

			memset(&cursor.entity, 0, sizeof(Entity));

			cursor.entity.draw = &drawLoopingAnimationToMap;

			cursor.entity.weight = 0;

			loadProperties(entityNames[cursor.entityType], &cursor.entity);

			cursor.entity.active = TRUE;

			if (cursor.entity.weight == 0)
			{
				cursor.entity.weight = 1;
			}
		}

		input.previous = 0;
	}

	else if (input.next == 1)
	{
		if (cursor.type == TILES)
		{
			cursor.tileID = nextTile(cursor.tileID);
		}

		else
		{
			cursor.entityType++;

			if (cursor.entityType >= entityNamesLength)
			{
				cursor.entityType = 0;
			}

			memset(&cursor.entity, 0, sizeof(Entity));

			cursor.entity.draw = &drawLoopingAnimationToMap;

			cursor.entity.weight = 0;

			loadProperties(entityNames[cursor.entityType], &cursor.entity);

			cursor.entity.active = TRUE;

			if (cursor.entity.weight == 0)
			{
				cursor.entity.weight = 1;
			}
		}

		input.next = 0;
	}

	if (input.save == 1)
	{
		saveMap();

		printf("Saved\n");

		input.save = 0;
	}
	/*
	if (input.load == 1)
	{
		loadMap(map.filename);

		printf("Loaded\n");

		input.load = 0;
	}
	*/
	if (input.left == 1 || input.right == 1 || input.up == 1 || input.down == 1)
	{
		SDL_Delay(30);
	}

	if (input.toggle == 1)
	{
		if (cursor.type == TILES)
		{
			cursor.type = ENTITIES;
		}

		else
		{
			cursor.type = TILES;
		}

		input.toggle = 0;
	}

	centerMapOnEntity(NULL);
}

void drawCursor()
{
	Entity *e;

	if (cursor.type == TILES)
	{
		drawImage(tileImage(cursor.tileID), cursor.x, cursor.y, FALSE);
		drawImage(tileImage(BLANK_TILE), cursor.x, cursor.y, FALSE);
	}

	else
	{
		e = isSpaceEmpty(&cursor.entity);

		if (isValidOnMap(&cursor.entity) == 0 || e != NULL)
		{
			drawBox(cursor.x, cursor.y, cursor.entity.w, cursor.entity.h, 255, 0, 0);
		}

		cursor.entity.x = getMapStartX() + cursor.x;
		cursor.entity.y = getMapStartY() + cursor.y;

		self = &cursor.entity;

		self->draw();
	}
}
