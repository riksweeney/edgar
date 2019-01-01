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
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void entityWait(void);
static void fallout(void);
static void falloutDie(void);

Entity *addIronBall(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Iron Ball");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &entityWait;
	e->draw = &drawLoopingAnimationToMap;
	e->fallout = &fallout;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	long onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if ((self->flags & ON_GROUND) || self->standingOn != NULL)
	{
		if (landedOnGround(onGround) == TRUE)
		{
			playSoundToMap("sound/enemy/red_grub/thud", -1, self->x, self->y, 0);

			self->dirX = self->endX == LEFT ? -self->speed : self->speed;
		}

		if (self->standingOn != NULL)
		{
			if (self->standingOn->dirX != 0)
			{
				self->endX = self->standingOn->dirX < 0 ? LEFT : RIGHT;
			}

			self->speed = self->standingOn->dirX == 0 ? self->speed : fabs(self->standingOn->dirX);
		}

		if (self->dirX == 0)
		{
			self->endX = self->endX == LEFT ? RIGHT : LEFT;
		}

		self->dirX = self->endX == LEFT ? -self->speed : self->speed;
	}

	else
	{
		self->dirX = 0;
	}
}

static void fallout()
{
	self->thinkTime = 300;

	self->flags |= DO_NOT_PERSIST;

	self->action = &falloutDie;
}

static void falloutDie()
{
	self->thinkTime--;

	if ((self->flags & ON_GROUND) || (self->thinkTime <= 0))
	{
		self->inUse = FALSE;
	}

	checkToMap(self);
}
