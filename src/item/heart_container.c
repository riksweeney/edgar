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
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void touch(Entity *);
static void entityWait(void);
static int draw(void);

Entity *addHeartContainer(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Heart Container");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &entityWait;
	e->touch = &touch;

	e->draw = &draw;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime < 90)
	{
		if (self->thinkTime % 3 == 0)
		{
			self->flags ^= NO_DRAW;
		}
	}

	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	if (other->type == PLAYER)
	{
		increasePlayerMaxHealth();

		self->inUse = FALSE;
	}
}

static int draw()
{
	Entity *e;

	if (drawLoopingAnimationToMap() == TRUE && (prand() % 10 == 0))
	{
		e = addBasicDecoration(self->x + prand() % self->w, self->y + prand() % 25, "decoration/particle");

		if (e != NULL)
		{
			e->thinkTime = 60 + prand() % 30;

			e->dirY = -0.3;
		}
	}

	return TRUE;
}
