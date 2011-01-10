/*
Copyright (C) 2009-2011 Parallel Realities

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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../collisions.h"
#include "../entity.h"
#include "../system/error.h"

static void entityWait(void);
static void initialize(void);
static void touch(Entity *);

extern Entity *self;

Entity *addVanishingPlatform(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Vanishing Platform");
	}

	loadProperties(name, e);

	e->type = VANISHING_PLATFORM;

	e->action = &entityWait;

	e->touch = &touch;

	e->action = &initialize;

	e->fallout = NULL;

	e->draw = &drawLoopingAnimationToMap;

	e->x = x;
	e->y = y;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	if (self->active == TRUE)
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			self->health = self->health == 1 ? 0 : 1;

			if (self->health == 0)
			{
				self->flags |= NO_DRAW;

				self->touch = NULL;
			}

			else
			{
				self->flags &= ~NO_DRAW;

				self->touch = &touch;
			}

			self->thinkTime = self->maxThinkTime;
		}
	}
}

static void initialize()
{
	#if DEV == 1
	if (self->active == FALSE && strlen(self->requires) == 0)
	{
		showErrorAndExit("Vanishing Platform at %d %d is inactive and has no requires", (int)self->x, (int)self->y);
	}
	#endif

	self->touch = self->health == 0 ? NULL : &pushEntity;

	if (self->health == 0)
	{
		self->flags |= NO_DRAW;
	}

	self->action = &entityWait;
}

static void touch(Entity *other)
{
	Entity *temp;

	/* Test the horizontal movement */

	if (other->type == PROJECTILE)
	{
		temp = self;

		self = other;

		self->die();

		self = temp;
	}

	else if (other->dirY > 0)
	{
		/* Trying to move down */

		if (collision(other->x, other->y - other->dirY, other->w, other->h, self->x, self->y, self->w, self->h) == 0
		&& collision(other->x, other->y, other->w, other->h, self->x, self->y, self->w, self->h) == 1)
		{
			/* Place the player as close to the solid tile as possible */

			other->y = self->y;
			other->y -= other->h;

			other->standingOn = self;
			other->dirY = 0;
			other->flags |= ON_GROUND;
		}
	}
}
