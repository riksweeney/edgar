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
static void upTouch(Entity *);
static void downTouch(Entity *);

Entity *addOneWayDoor(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a One Way Door");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &entityWait;

	switch (e->mental)
	{
		case 0:
			e->touch = &touch;
		break;

		case 1:
			e->touch = &upTouch;
		break;

		default:
			e->touch = &downTouch;
		break;
	}

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	checkToMap(self);
}

static void touch(Entity *other)
{
	if ((other->dirX < 0 && self->face == RIGHT) || (other->dirX > 0 && self->face == LEFT))
	{
		other->x -= other->dirX;

		other->dirX = 0;
	}

	else if (other->dirX == 0)
	{
		other->x += self->face == RIGHT ? self->speed : -self->speed;
	}
}

static void upTouch(Entity *other)
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

static void downTouch(Entity *other)
{
	int topBefore = other->y - other->dirY - 1;

	if (topBefore > self->y + self->h)
	{
		other->y = self->y + self->h;

		other->dirY = 0;
	}

	else
	{
		other->y += self->speed;

		other->dirY = 0;
	}
}
