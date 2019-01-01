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
#include "../graphics/decoration.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void move(void);
static int draw(void);

Entity *addBuzzSaw(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Buzz Saw");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &move;
	e->draw = &draw;
	e->touch = &entityTouch;

	e->type = ENEMY;

	e->face = RIGHT;

	setEntityAnimation(e, "STAND");

	return e;
}

static void move()
{
	if (self->dirX == 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->dirX = self->health == 0 ? -self->speed : self->speed;

			self->thinkTime = self->maxThinkTime;

			self->health = 1 - self->health;
		}
	}

	if (self->mental == 0 && isAtEdge(self) == TRUE)
	{
		self->dirX = 0;
	}

	else if (self->mental == 1)
	{
		if (self->dirX < 0 && self->x <= self->startX)
		{
			self->x = self->startX;

			self->dirX = 0;

			self->health = 1;
		}

		else if (self->dirX > 0 && self->x >= self->endX)
		{
			self->x = self->endX;

			self->dirX = 0;

			self->health = 0;
		}
	}

	checkToMap(self);
}

static int draw()
{
	Entity *e;

	if (drawLoopingAnimationToMap() == TRUE)
	{
		e = addPixelDecoration(self->x, self->y + self->h);

		if (e != NULL)
		{
			e->dirX = prand() % 20;
			e->dirY = -prand() % 20;

			if (prand() % 2 == 0)
			{
				e->dirX *= -1;
			}

			e->dirX /= 10;
			e->dirY /= 10;

			e->thinkTime = 20 + (prand() % 30);

			e->health = 220;

			e->maxHealth = 220;

			e->mental = 0;
		}

		e = addPixelDecoration(self->x + self->w, self->y + self->h);

		if (e != NULL)
		{
			e->dirX = prand() % 20;
			e->dirY = -prand() % 20;

			if (prand() % 2 == 0)
			{
				e->dirX *= -1;
			}

			e->dirX /= 10;
			e->dirY /= 10;

			e->thinkTime = 20 + (prand() % 30);

			e->health = 220;

			e->maxHealth = 220;

			e->mental = 0;
		}
	}

	return TRUE;
}
