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
#include "../enemy/enemies.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self, player;

static void followPlayer(void);
static void die(void);
static void activate(int);

Entity *addGolemRockDropper(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Golem Rock Dropper");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &followPlayer;

	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->activate = &activate;

	setEntityAnimation(e, "STAND");

	return e;
}

static void followPlayer()
{
	Entity *e;

	if (self->active == TRUE)
	{
		self->targetX = player.x - self->w / 2 + player.w / 2;

		/* Position above the player */

		if (abs(self->x - self->targetX) <= abs(self->dirX))
		{
			self->x = self->targetX;

			self->dirX = 0;
		}

		else
		{
			self->dirX = self->targetX < self->x ? -player.speed * 3 : player.speed * 3;
		}

		if (self->health > 0)
		{
			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				e = addEnemy("common/large_rock", self->x, self->y);

				e->x += (self->w - e->w) / 2;
				e->y += (self->h - e->h) / 2;

				self->health--;

				self->thinkTime = self->maxThinkTime;
			}
		}
	}

	checkToMap(self);
}

static void activate(int val)
{
	self->health = val;
}

static void die()
{
	self->inUse = FALSE;
}
