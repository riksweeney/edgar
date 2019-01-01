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
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void entityWait(void);
static void followTarget(void);
static void drop(void);
static void rise(void);
static void init(void);
static void armWait(void);
static int drawArm(void);
static void createArm(void);
static void touch(Entity *);

Entity *addLabCrusher(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Laboratory Crusher");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->startX = x;
	e->startY = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->pain = NULL;
	e->reactToBlock = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	createArm();

	self->action = &entityWait;
}

static void entityWait()
{
	if (self->active == TRUE)
	{
		self->target = self->health == -1 ? getEntityByObjectiveName(self->requires) : &player;

		self->action = &followTarget;
	}

	checkToMap(self);
}

static void followTarget()
{
	if (self->active == TRUE)
	{
		self->targetX = self->target->x - self->w / 2 + self->target->w / 2;

		/* Position above the player */

		if (abs(self->x - self->targetX) <= abs(self->dirX))
		{
			self->x = self->targetX;

			self->dirX = 0;

			self->action = &drop;

			self->thinkTime = 15;
		}

		else
		{
			self->dirX = self->targetX < self->x ? -self->target->speed * 3 : self->target->speed * 3;
		}
	}

	checkToMap(self);

	if (self->x < self->startX)
	{
		self->dirX = 0;

		self->x = self->startX;

		self->action = &drop;

		self->thinkTime = 15;
	}

	else if (self->x > self->endX)
	{
		self->dirX = 0;

		self->x = self->endX;

		self->action = &drop;

		self->thinkTime = 15;
	}
}

static void drop()
{
	int i;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirY = 16;
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		for (i=0;i<20;i++)
		{
			addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
		}

		playSoundToMap("sound/common/crunch", -1, self->x, self->y, 0);

		self->thinkTime = 30;

		self->action = &rise;
	}
}

static void rise()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirY = -4;

		if (self->y <= self->startY)
		{
			self->y = self->startY;

			self->mental--;

			if (self->mental <= 0)
			{
				self->active = FALSE;
			}

			self->dirY = 0;

			self->action = &followTarget;
		}
	}

	checkToMap(self);
}

static void createArm()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Crusher Arm");
	}

	loadProperties("item/crusher_arm", e);

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &armWait;

	e->draw = &drawArm;

	e->head = self;

	setEntityAnimation(e, "STAND");
}

static void armWait()
{
	checkToMap(self);

	self->x = self->head->x;
	self->y = self->head->y - self->h;
}

static int drawArm()
{
	int y;

	y = self->head->endY - self->h * 2;

	drawLoopingAnimationToMap();

	while (self->y >= y)
	{
		drawSpriteToMap();

		self->y -= self->h;
	}

	return TRUE;
}

static void touch(Entity *other)
{
	int bottomBefore;
	float dirX;
	Entity *temp;

	if (other->type == PLAYER && other->dirY > 0 && other->health > 0)
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

						freeEntityList(playerGib());

						self = temp;
					}
				}

				/* Place the player as close to the solid tile as possible */

				other->y = self->y;
				other->y -= other->h;

				other->standingOn = self;
				other->dirY = 0;
				other->flags |= ON_GROUND;
			}

			else
			{
				entityTouch(other);
			}
		}
	}

	else
	{
		entityTouch(other);
	}
}
