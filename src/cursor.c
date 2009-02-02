#include "headers.h"

#include "animation.h"
#include "entity.h"
#include "properties.h"
#include "map.h"
#include "player.h"
#include "collisions.h"
#include "target.h"
#include "graphics.h"
#include "status.h"

extern Cursor cursor;
extern Input input;
extern Entity *self;

static char *entityNames[] = {"edgar/edgar", "item/apple", "item/wooden_crate", "item/metal_crate",
							  "enemy/bat", "weapon/pickaxe",
							  "item/chicken_feed_bag", "enemy/chicken",
							  "item/chicken_trap", "lift/mine_lift", "lift/lift_target", "common/spawner",
							  "common/pressure_plate", "common/wooden_door", "wall/weak_mine_wall",
							  "item/iron_key", "common/switch",
							  NULL};
static int entityNamesLength = 0;
static int targetID = 1;

void initCursor()
{
	int i = 0;

	cursor.tileID = 0;
	cursor.entityType = 0;

	loadProperties(entityNames[0], &cursor.entity);

	cursor.entity.draw = &drawLoopingAnimationToMap;

	while (entityNames[i] != NULL)
	{
		i++;
	}

	entityNamesLength = i;
}

void doCursor()
{
	char name[30];

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

	if (input.add == 1)
	{
		if (cursor.type == TILES)
		{
			setTileAt((mapStartX() + cursor.x) / TILE_SIZE, (mapStartY() + cursor.y) / TILE_SIZE, cursor.tileID);
		}

		else
		{
			/* Entities can only be placed in blank spaces */

			if (isValidOnMap(&cursor.entity) == 1 && isSpaceEmpty(&cursor.entity) == NULL)
			{
				if (cursor.entityType == 0)
				{
					setPlayerLocation(mapStartX() + cursor.x, mapStartY() + cursor.y);
				}

				else
				{
					if (strcmpignorecase(cursor.entity.name, "lift/lift_target") == 0)
					{
						sprintf(name, "NEW_TARGET_%03d", targetID);

						addTarget(mapStartX() + cursor.x, mapStartY() + cursor.y, name);

						targetID++;
					}

					else
					{
						addEntity(cursor.entity, mapStartX() + cursor.x, mapStartY() + cursor.y);
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
			setTileAt((mapStartX() + cursor.x) / TILE_SIZE, (mapStartY() + cursor.y) / TILE_SIZE, BLANK_TILE);
		}

		else
		{
			self = isSpaceEmpty(&cursor.entity);

			if (self != NULL)
			{
				self->inUse = NOT_IN_USE;
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

				self->inUse = NOT_IN_USE;
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

			loadProperties(entityNames[cursor.entityType], &cursor.entity);
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

			loadProperties(entityNames[cursor.entityType], &cursor.entity);
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
		drawImage(tileImage(cursor.tileID), cursor.x, cursor.y);
		drawImage(tileImage(BLANK_TILE), cursor.x, cursor.y);
	}

	else
	{
		e = isSpaceEmpty(&cursor.entity);
		
		if (isValidOnMap(&cursor.entity) == 0 || e != NULL)
		{
			drawBox(cursor.x, cursor.y, cursor.entity.w, cursor.entity.h, 255, 0, 0);
		}

		cursor.entity.x = mapStartX() + cursor.x;
		cursor.entity.y = mapStartY() + cursor.y;

		self = &cursor.entity;

		self->draw();
	}
}
