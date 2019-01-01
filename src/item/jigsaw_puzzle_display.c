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

#include "../collisions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void activate(int);
static void entityWait(void);
static void init(void);
static void randomize(void);
static void swap(int *, int *);
static void addCursor(void);
static void cursorWait(void);
static void swapTiles(int, int, int);

Entity *addJigsawPuzzleDisplay(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Jigsaw Puzzle Display");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &init;
	e->activate = &activate;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	if (self->mental == -2)
	{
		self->mental = 1;

		init();
	}

	checkToMap(self);
}

static void activate(int val)
{
	if (self->mental != -3)
	{
		if (val == 5)
		{
			self->health = 1 - self->health;
		}

		else if (self->health == 0)
		{
			switch (val)
			{
				case 1:
					self->endY -= TILE_SIZE;
				break;

				case 2:
					self->endY += TILE_SIZE;
				break;

				case 3:
					self->endX -= TILE_SIZE;
				break;

				default:
					self->endX += TILE_SIZE;
				break;
			}

			if (self->endX < self->startX)
			{
				self->endX = self->startX;
			}

			else if (self->endX >= self->startX + self->w)
			{
				self->endX = self->startX + self->w - TILE_SIZE;
			}

			else if (self->endY < self->startY)
			{
				self->endY = self->startY;
			}

			else if (self->endY >= self->startY + self->h)
			{
				self->endY = self->startY + self->h - TILE_SIZE;
			}
		}

		else if (self->health == 1)
		{
			swapTiles(self->endX, self->endY, val);
		}
	}
}

static void init()
{
	int i;
	Entity *e, *prev;

	if (self->mental == 1 || self->mental == -3)
	{
		prev = self;

		for (i=0;i<9;i++)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add a Jigsaw Puzzle Block");
			}

			loadProperties("item/puzzle_piece", e);

			e->action = &doNothing;

			e->draw = &drawLoopingAnimationToMap;

			e->face = RIGHT;

			e->head = self;

			e->flags |= (FLY|DO_NOT_PERSIST);

			prev->target = e;

			prev = e;
		}

		randomize();
	}

	if (self->mental != -3)
	{
		addCursor();
	}

	self->action = &entityWait;
}

static void randomize()
{
	int i, *tiles, boardWidth, tileWidth, x, y, dir;
	Entity *e;

	tileWidth = self->target->w;
	boardWidth = self->w;

	tiles = malloc(9 * sizeof(int));

	if (tiles == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for the Jigsaw Puzzle display", (int)sizeof(int) * 9);
	}

	for (i=0;i<9;i++)
	{
		tiles[i] = i * tileWidth;
	}

	i = 0;

	for (e=self->target;e!=NULL;e=e->target)
	{
		e->targetX = self->x + (tiles[i] % boardWidth);
		e->targetY = self->y + (tiles[i] / boardWidth) * tileWidth;

		e->x = e->targetX;
		e->y = e->targetY;

		setEntityAnimationByID(e, i);

		e->health = i;

		i++;
	}

	if (self->mental != -3)
	{
		for (i=0;i<9;i++)
		{
			tiles[i] = i;
		}

		for (i=0;i<9;i++)
		{
			swap(&tiles[i], &tiles[prand() % 9]);
		}

		i = 0;

		self->mental = -10;

		for (i=0;i<10000;i++)
		{
			x = self->x + (TILE_SIZE * (prand() % 3));
			y = self->y + (TILE_SIZE * (prand() % 3));
			dir = 1 + (prand() % 4);

			swapTiles(x, y, dir);
		}

		self->endX = self->x;
		self->endY = self->y;

		self->mental = 1;
	}
}

static void swap(int *a, int *b)
{
	int temp;

	temp = *a;

	*a = *b;

	*b = temp;
}

static void addCursor()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Jigsaw Puzzle Cursor");
	}

	loadProperties("item/jigsaw_puzzle_cursor", e);

	e->x = self->x;
	e->y = self->y;

	e->action = &cursorWait;

	e->draw = &drawLoopingAnimationToMap;

	e->head = self;

	setEntityAnimation(e, "STAND");
}

static void cursorWait()
{
	self->x = self->head->endX;
	self->y = self->head->endY;

	if (self->head->mental == -3)
	{
		self->inUse = FALSE;
	}
}

static void swapTiles(int x, int y, int dir)
{
	int dirX, dirY, valid, i;
	Entity *e, *a, *b;

	dirX = 0;
	dirY = 0;

	switch (dir)
	{
		case 1:
			dirY = -TILE_SIZE;
		break;

		case 2:
			dirY = TILE_SIZE;
		break;

		case 3:
			dirX = -TILE_SIZE;
		break;

		default:
			dirX = TILE_SIZE;
		break;
	}

	a = NULL;
	b = NULL;

	for (e=self->target;e!=NULL;e=e->target)
	{
		if (e->targetX == x && e->targetY == y)
		{
			a = e;
		}

		else if (e->targetX == x + dirX && e->targetY == y + dirY)
		{
			b = e;
		}
	}

	if (a != NULL && b != NULL)
	{
		a->targetX += dirX;
		a->targetY += dirY;

		a->x = a->targetX;
		a->y = a->targetY;

		b->targetX -= dirX;
		b->targetY -= dirY;

		b->x = b->targetX;
		b->y = b->targetY;

		self->endX += dirX;
		self->endY += dirY;

		i = a->health;

		a->health = b->health;

		b->health = i;

		if (self->mental != -10)
		{
			i = 0;

			valid = TRUE;

			for (e=self->target;e!=NULL;e=e->target)
			{
				if (e->health != i)
				{
					valid = FALSE;
				}

				i++;
			}

			if (valid == TRUE)
			{
				self->mental = -3;
			}
		}
	}
}
