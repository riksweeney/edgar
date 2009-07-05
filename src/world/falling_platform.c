/*
Copyright (C) 2009 Parallel Realities

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

static void wait(void);
static void touch(Entity *);
static void initFall(void);
static void resetWait(void);
static void resetPlatform(void);
static void initialize(void);
static void fallout(void);

extern Entity *self;

Entity *addFallingPlatform(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("Couldn't get a free slot for a Falling Platform!\n");

		exit(1);
	}

	loadProperties(strcmpignorecase(name, "lift/falling_platform") == 0 ? "falling_platform/cave_falling_platform" : name, e);

	e->type = FALLING_PLATFORM;
	e->action = &wait;

	e->touch = &touch;

	e->action = &initialize;

	e->fallout = &fallout;

	e->draw = &drawLoopingAnimationToMap;

	e->x = e->startX = x;
	e->y = e->startY = y;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	if (self->maxThinkTime < 0)
	{
		if (self->health == 0)
		{
			resetPlatform();
		}

		else
		{
			self->dirY = self->speed;

			checkToMap(self);
		}

		self->health = 0;
	}
	
	else
	{
		self->dirY = 0;
	}
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

			if (self->maxThinkTime < 0)
			{
				self->health = 1;
			}

			else if (!(self->flags & ON_GROUND))
			{
				self->action = &initFall;
			}
		}
	}
}

static void initFall()
{
	self->thinkTime--;

	if (self->thinkTime > 0)
	{
		if (self->x == self->startX || (self->thinkTime % 4 == 0))
		{
			self->x = self->startX + (3 * (self->x < self->startX ? 1 : -1));
		}
		
		self->dirY = 0;
	}

	else
	{
		self->thinkTime = 0;

		self->x = self->startX;

		self->touch = NULL;

		checkToMap(self);

		if (self->flags & ON_GROUND)
		{
			self->thinkTime = 120;

			self->action = &resetWait;
		}
	}
}

static void resetWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &resetPlatform;
	}
}

static void resetPlatform()
{
	self->dirY = self->maxThinkTime < 0 ? -self->speed / 2 : -self->speed;

	checkToMap(self);

	if (self->y <= self->startY)
	{
		self->y = self->startY;

		self->thinkTime = self->maxThinkTime;

		self->touch = &touch;

		self->action = &wait;
	}
}

static void initialize()
{
	if (self->x != self->startX || self->y != self->startY)
	{
		self->action = &resetPlatform;
	}

	else
	{
		self->action = &wait;
	}

	self->health = 0;
}

static void fallout()
{

}
