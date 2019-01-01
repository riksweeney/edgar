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

#include "../audio/audio.h"
#include "../collisions.h"
#include "../entity.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void activate(int);
static void entityWait(void);
static void init(void);
static void randomize(void);
static void validate(void);
static void slideToTarget(void);

Entity *addSlidingPuzzleDisplay(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Sliding Puzzle Display");
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
	if (self->mental == -3)
	{
		validate();
	}

	checkToMap(self);
}

static void validate()
{
	int x, y, i, found, cols;
	Entity *e;

	cols = sqrt(self->health);

	x = self->x;
	y = self->y;

	i = 0;

	found = TRUE;

	for (e=self->target;e!=NULL;e=e->target)
	{
		if (i != 0 && (i % cols == 0))
		{
			x = self->x;

			y += e->h;
		}

		if (!(e->targetX == x && e->targetY == y))
		{
			found = FALSE;

			break;
		}

		i++;

		x += e->w;
	}

	if (found == TRUE)
	{
		self->mental = 1;
	}

	else
	{
		self->mental = 0;
	}
}

static void activate(int val)
{
	int dirX, dirY;
	Entity *e;

	dirX = 0;
	dirY = 0;

	switch (val)
	{
		case 1:
			dirY = -self->target->h;
		break;

		case 2:
			dirY = self->target->h;
		break;

		case 3:
			dirX = -self->target->w;
		break;

		case 4:
			dirX = self->target->w;
		break;
	}

	for (e=self->target;e!=NULL;e=e->target)
	{
		if (e->targetX + dirX == self->targetX && e->targetY + dirY == self->targetY)
		{
			e->targetX += dirX;
			e->targetY += dirY;

			self->targetX -= dirX;
			self->targetY -= dirY;

			if (self->mental == 0)
			{
				calculatePath(e->x, e->y, e->targetX, e->targetY, &e->dirX, &e->dirY);

				e->dirX *= 6;
				e->dirY *= 6;

				e->action = &slideToTarget;

				self->activate = NULL;

				playSoundToMap("sound/common/click", -1, self->x, self->y, 0);
			}

			else
			{
				e->x += dirX;
				e->y += dirY;
			}

			break;
		}
	}
}

static void init()
{
	int i;
	Entity *e, *prev;

	prev = self;

	for (i=0;i<self->health-1;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Sliding Puzzle Block");
		}

		loadProperties("item/sliding_puzzle_block", e);

		setEntityAnimationByID(e, i);

		e->health = i + 1;

		e->action = &entityWait;

		e->draw = &drawLoopingAnimationToMap;

		e->face = RIGHT;

		e->head = self;

		prev->target = e;

		prev = e;
	}

	if (self->mental == 0)
	{
		randomize();
	}

	self->action = &entityWait;
}

static void randomize()
{
	int i, *tiles, boardWidth, tileWidth;
	Entity *e;

	tileWidth = self->target->w;
	boardWidth = self->w;

	tiles = malloc(self->health * sizeof(int));

	if (tiles == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for the Sliding Puzzle display", (int)sizeof(int) * self->health);
	}

	for (i=0;i<self->health;i++)
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

		i++;
	}

	self->targetX = self->x + (tiles[i] % boardWidth);
	self->targetY = self->y + (tiles[i] / boardWidth) * tileWidth;

	/* Don't just randomly stuff the tiles anywhere, it could make it unsolvable */

	self->mental = -1;

	for (i=0;i<1000;i++)
	{
		self->activate(1 + (prand() % 4));
	}

	self->mental = 0;

	free(tiles);
}

static void slideToTarget()
{
	self->x += self->dirX;
	self->y += self->dirY;

	if (atTarget())
	{
		self->x = self->targetX;
		self->y = self->targetY;

		self->dirX = 0;
		self->dirY = 0;

		self->head->activate = &activate;

		self->head->mental = -3;

		self->action = &entityWait;
	}
}
