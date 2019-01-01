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
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

static void entityWait(void);
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
		showErrorAndExit("No free slots to add a Falling Platform");
	}

	loadProperties(strcmpignorecase(name, "lift/falling_platform") == 0 ? "falling_platform/cave_falling_platform" : name, e);

	e->type = FALLING_PLATFORM;

	e->action = &entityWait;

	e->touch = &touch;

	e->action = &initialize;

	e->fallout = &fallout;

	e->draw = &drawLoopingAnimationToMap;

	e->x = e->startX = x;
	e->y = e->startY = y;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	self->face = RIGHT;

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

	else if (other->type == PLAYER && other->dirY > 0)
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
	int i;

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
			if (self->environment == AIR)
			{
				playSoundToMap("sound/enemy/red_grub/thud", -1, self->x, self->y, 0);

				for (i=0;i<20;i++)
				{
					addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
				}
			}

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

		self->action = &entityWait;
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
		self->action = &entityWait;
	}

	self->health = 0;
}

static void fallout()
{
	if (self->environment == AIR)
	{
		self->inUse = FALSE;
	}
}
