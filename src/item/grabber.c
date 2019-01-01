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
#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void entityWait(void);
static void init(void);
static void dropRock(void);
static void grabRock(void);
static void moveToTarget(void);
static void moveToStart(void);
static void moveToEnd(void);
static void touch(Entity *);
static void activate(int);
static void addChain(void);
static void chainWait(void);
static int drawChain(void);
static void fallout(void);

Entity *addGrabber(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Grabber");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = LEFT;

	e->action = &init;
	e->activate = &activate;
	e->touch = &touch;
	e->fallout = &fallout;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	if (self->health != 1 && self->active == TRUE)
	{
		if (self->target == NULL)
		{
			setEntityAnimation(self, "WALK");

			self->action = &grabRock;
		}

		else
		{
			self->target->y = self->y + self->h / 2;

			self->target->x = self->x + (self->w - self->target->w) / 2;

			self->thinkTime = 60;

			setEntityAnimation(self, "WALK");

			self->action = &dropRock;
		}
	}

	else if (self->health == 1)
	{
		self->action = &moveToEnd;
	}

	checkToMap(self);
}

static void dropRock()
{
	Entity *temp;

	if (self->target != NULL)
	{
		self->target->flags &= ~(HELPLESS|FLY);

		self->target->y = self->y + self->h / 2;

		self->target->x = self->x + (self->w - self->target->w) / 2;

		if (self->target->type == ENEMY)
		{
			self->target->type = PROJECTILE;

			self->target->damage = 100;

			self->target->parent = self;
		}

		else if (self->target->resumeNormalFunction != NULL)
		{
			temp = self;

			self = self->target;

			self->resumeNormalFunction();

			self = temp;
		}

		else
		{
			self->target->touch = &pushEntity;
		}

		self->target = NULL;
	}

	else
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			setEntityAnimation(self, "STAND");

			moveToStart();
		}
	}
}

static void grabRock()
{
	self->dirY = self->speed;

	if (self->standingOn != NULL)
	{
		touch(self->standingOn);
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		setEntityAnimation(self, "STAND");

		moveToStart();
	}
}

static void moveToTarget()
{
	/* Move vertically first */

	if (self->y > self->targetY)
	{
		self->dirY = -self->speed;

		self->dirX = 0;
	}

	/* Now move horizontally */

	else
	{
		self->y = self->targetY;

		self->dirX = self->targetX < self->x ? -self->speed : self->speed;

		self->dirY = 0;
	}

	checkToMap(self);

	if (self->target != NULL)
	{
		self->target->y = self->y + self->h / 2;

		self->target->x = self->x + (self->w - self->target->w) / 2;

		self->target->x += self->dirX;
		self->target->y += self->dirY;
	}

	if (fabs(self->x - self->targetX) <= fabs(self->dirX) && fabs(self->y - self->targetY) <= fabs(self->dirY))
	{
		self->x = self->targetX;
		self->y = self->targetY;

		self->dirX = 0;
		self->dirY = 0;

		if (self->health == 1)
		{
			if (self->x == self->endX && self->y == self->endY)
			{
				setEntityAnimation(self, "STAND");

				self->health = 2;

				self->startX = self->endX;
				self->startY = self->endY;
			}
		}

		else if (self->health == -1)
		{
			self->health = 0;
		}

		self->active = FALSE;

		self->action = &entityWait;
	}
}

static void touch(Entity *other)
{
	Entity *temp;

	if (self->active == TRUE)
	{
		if (strcmpignorecase(other->name, "boss/golem_boss") == 0)
		{
			setEntityAnimation(self, "STAND");

			if (strcmpignorecase(other->animationName, "CUSTOM_1") == 0 && self->health == 2)
			{
				temp = self;

				self = other;

				self->die();

				self = temp;
			}

			else
			{
				moveToStart();
			}
		}

		else if ((other->flags & PUSHABLE) && ((other->flags & ON_GROUND) || (self->health == -1 && self->target == NULL)))
		{
			setEntityAnimation(self, "STAND");

			other->flags |= HELPLESS|FLY;

			other->touch = &entityTouch;

			other->damage = 0;

			other->dirX = 0;

			other->dirY = 0;

			other->y = self->y + self->h / 2;

			other->x = self->x + (self->w - other->w) / 2;

			self->target = other;

			self->action = &moveToEnd;

			self->active = FALSE;

			self->dirY = -self->speed;

			self->dirX = 1;

			/* Allow grabbing to be resumed when reloading the game */

			self->health = -1;
		}
	}
}

static void moveToStart()
{
	self->targetX = self->startX;
	self->targetY = self->startY;

	self->action = &moveToTarget;

	self->active = FALSE;
}

static void moveToEnd()
{
	self->targetX = self->endX;
	self->targetY = self->endY;

	self->action = &moveToTarget;

	self->active = TRUE;
}

static void activate(int val)
{
	self->active = TRUE;
}

static void addChain()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Grabber Chain");
	}

	loadProperties("item/grabber_chain", e);

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &chainWait;

	e->draw = &drawChain;

	e->head = self;

	setEntityAnimation(e, "STAND");
}

static void chainWait()
{
	checkToMap(self);

	self->x = self->head->x + self->head->w / 2 - self->w / 2;
	self->y = self->head->y - self->h;
}

static void init()
{
	addChain();

	self->dirX = 0;

	self->dirY = 0;

	self->action = &entityWait;
}

static int drawChain()
{
	int y;

	y = self->head->startY - self->h * 2;

	drawLoopingAnimationToMap();

	while (self->y >= y)
	{
		drawSpriteToMap();

		self->y -= self->h;
	}

	return TRUE;
}

static void fallout()
{
	/* Do nothing */
}
