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

#include "../enemy/enemies.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void entityWait(void);
static void touch(Entity *);

Entity *addSlimeContainer(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Slime Container");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &entityWait;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	e->thinkTime = 0;

	return e;
}

static void entityWait()
{
	Entity *e;

	if (self->health == 1)
	{
		if (self->active == FALSE)
		{
			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				if (self->mental < 15)
				{
					e = addEnemy("enemy/red_baby_slime", 0, 0);

					e->x = self->startX;
					e->y = self->startY;

					e->mental = 1;

					self->mental++;

					self->maxThinkTime++;
				}

				self->thinkTime = 60;
			}
		}

		else
		{
			self->thinkTime = 60;

			self->mental = 0;
		}
	}

	self->touch = self->active == FALSE ? &touch : NULL;

	setEntityAnimation(self, self->active == FALSE ? "STAND" : "WALK");
}

static void touch(Entity *other)
{
	pushEntity(other);
}
