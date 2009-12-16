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
#include "../graphics/decoration.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/random.h"

extern Entity *self;

static void wait(void);
static void touch(Entity *);
static void initFall(void);
static void resetWait(void);
static void resetPlatform(void);
static void touch(Entity *);

Entity *addMataeusWall(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Mataeus Wall");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &wait;
	e->touch = &touch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	checkToMap(self);

	if (self->mental == 1)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->thinkTime = 60;

			self->action = &initFall;
		}
	}
}

static void initFall()
{
	int i;
	long onGround = self->flags & ON_GROUND;

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

		self->damage = 1;

		checkToMap(self);

		if (self->flags & ON_GROUND)
		{
			if (onGround == 0)
			{
				for (i=0;i<20;i++)
				{
					addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
				}
			}

			self->damage = 0;

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

		self->action = &wait;

		self->mental = 0;
	}
}

static void touch(Entity *other)
{
	int bottomBefore;
	float dirX;
	Entity *temp;

	entityTouch(other);

	if (other->type == PLAYER && other->dirY > 0)
	{
		/* Trying to move down */

		if (collision(other->x, other->y, other->w, other->h, self->x, self->y, self->w, self->h) == 1)
		{
			bottomBefore = other->y + other->h - other->dirY - 1;

			if (abs(bottomBefore - self->y) < self->h - 1)
			{
				if (self->dirY < 0)
				{
					other->y -= self->dirY;

					other->dirY = self->dirY;

					dirX = other->dirX;

					other->dirX = 0;

					checkToMap(other);

					other->dirX = dirX;

					if (other->dirY == 0)
					{
						/* Gib the player */

						temp = self;

						self = other;

						playerGib();

						self = temp;
					}
				}

				/* Place the player as close to the solid tile as possible */

				other->y = self->y;
				other->y -= other->h;

				other->standingOn = self;
				other->dirY = 0;
				other->flags |= ON_GROUND;

				self->mental = 1;
			}
		}
	}
}
