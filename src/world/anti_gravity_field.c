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
#include "../custom_actions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void entityWait(void);
static void touch(Entity *);

Entity *addAntiGravityField(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Anti Gravity Field");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = ANTI_GRAVITY;

	e->face = RIGHT;

	e->action = &entityWait;
	e->touch = &touch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	if (self->active == TRUE)
	{
		self->flags &= ~NO_DRAW;
	}

	else
	{
		self->flags |= NO_DRAW;
	}
}

static void touch(Entity *other)
{
	int bottomBefore;

	if (self->active == TRUE && !(other->flags & FLY))
	{
		if (other->dirY > 0)
		{
			bottomBefore = other->y + other->h - other->dirY - 1;

			if (bottomBefore < self->y)
			{
				/* Place the player as close to the solid tile as possible */

				other->y = self->y;
				other->y -= other->h;

				other->standingOn = self;
				other->dirY = 0;
				other->flags |= ON_GROUND;
			}

			else
			{
				setCustomAction(other, &antiGravity, 2, 0, 1);
			}
		}

		else
		{
			setCustomAction(other, &antiGravity, 2, 0, 1);
		}
	}
}
