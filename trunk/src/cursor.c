/*
Copyright (C) 2009-2012 Parallel Realities

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

#include "headers.h"

#include "collisions.h"
#include "entity.h"
#include "graphics/animation.h"
#include "graphics/graphics.h"
#include "map.h"
#include "player.h"
#include "status_panel.h"
#include "system/pak.h"
#include "system/properties.h"
#include "world/target.h"

extern Cursor cursor;
extern Input input;
extern Entity *self;
extern Game game;

static char entityNames[255][MAX_VALUE_LENGTH];
static int entityNamesLength = 0;
static int targetID = 1;

void initCursor(char *name)
{
	char line[MAX_LINE_LENGTH], *token, *savePtr;
	unsigned char *buffer;

	savePtr = NULL;

	cursor.tileID = 0;
	cursor.entityType = 0;
	cursor.entity.weight = 0;
	cursor.entity.originalWeight = 0;

	cursor.entity.inUse = TRUE;

	loadProperties(entityNames[0], &cursor.entity);

	cursor.entity.active = TRUE;

	cursor.entity.alpha = 255;

	if (cursor.entity.weight == 0)
	{
		cursor.entity.weight = 1;

		cursor.entity.originalWeight = 1;
	}

	cursor.entity.draw = &drawLoopingAnimationToMap;

	snprintf(line, sizeof(line), "data/cursor/%s.dat", name);

	buffer = loadFileFromPak(line);

	token = strtok_r((char *)buffer, "\n", &savePtr);

	while (token != NULL)
	{
		STRNCPY(entityNames[entityNamesLength], token, MAX_VALUE_LENGTH);

		if (entityNames[entityNamesLength][strlen(entityNames[entityNamesLength]) - 1] == '\n')
		{
			entityNames[entityNamesLength][strlen(entityNames[entityNamesLength]) - 1] = '\0';
		}

		if (entityNames[entityNamesLength][strlen(entityNames[entityNamesLength]) - 1] == '\r')
		{
			entityNames[entityNamesLength][strlen(entityNames[entityNamesLength]) - 1] = '\0';
		}

		entityNamesLength++;

		token = strtok_r(NULL, "\n", &savePtr);
	}

	free(buffer);
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
						cursor.entity.startX = getMapStartX() + cursor.x;
						cursor.entity.startY = getMapStartY() + cursor.y;

						cursor.entity.endX = getMapStartX() + cursor.x;
						cursor.entity.endY = getMapStartY() + cursor.y;

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

			cursor.entity.originalWeight = 0;

			cursor.entity.inUse = TRUE;

			loadProperties(entityNames[cursor.entityType], &cursor.entity);

			cursor.entity.active = TRUE;

			cursor.entity.alpha = 255;

			if (cursor.entity.weight == 0)
			{
				cursor.entity.weight = 1;

				cursor.entity.originalWeight = 1;
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

			cursor.entity.originalWeight = 1;

			cursor.entity.inUse = TRUE;

			loadProperties(entityNames[cursor.entityType], &cursor.entity);

			cursor.entity.active = TRUE;

			cursor.entity.alpha = 255;

			if (cursor.entity.weight == 0)
			{
				cursor.entity.weight = 1;

				cursor.entity.originalWeight = 1;
			}
		}

		input.next = 0;
	}

	if (input.save == 1)
	{
		if (saveMap() == TRUE)
		{
			setStatusPanelMessage("Saved");
		}

		else
		{
			setStatusPanelMessage("Saving Failed");
		}

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
		drawImage(tileImage(cursor.tileID), cursor.x, cursor.y, FALSE, 255);

		drawImage(tileImage(BLANK_TILE), cursor.x, cursor.y, FALSE, 255);
	}

	else
	{
		e = isSpaceEmpty(&cursor.entity);

		if (isValidOnMap(&cursor.entity) == 0 || e != NULL)
		{
			drawBox(game.screen, cursor.x, cursor.y, cursor.entity.w, cursor.entity.h, 255, 0, 0);
		}

		if (e != NULL)
		{
			setStatusPanelMessage("%s : %d %d", (strlen(e->objectiveName) == 0 ? e->name : e->objectiveName), (int)e->x, (int)e->y);
		}

		else
		{
			setStatusPanelMessage("");
		}

		cursor.entity.x = getMapStartX() + cursor.x;
		cursor.entity.y = getMapStartY() + cursor.y;

		self = &cursor.entity;

		self->draw();
	}
}
