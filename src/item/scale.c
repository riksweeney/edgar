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

static void init(void);
static void entityWait(void);
static void touch(Entity *);
static void addChain(void);
static void chainWait(void);
static int drawChain(void);

Entity *addScale(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Scale");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;
	e->touch = &touch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	e->thinkTime = 0;

	return e;
}

static void init()
{
	EntityList *list;
	EntityList *l;
	Entity *e;

	if (strlen(self->objectiveName) == 0)
	{
		showErrorAndExit("Scale at %d %d has no name", (int)self->x, (int)self->y);
	}

	list = getEntitiesByObjectiveName(self->objectiveName);

	for (l=list->next;l!=NULL;l=l->next)
	{
		e = l->entity;

		if (e != self)
		{
			self->target = e;

			break;
		}
	}

	freeEntityList(list);

	if (self->target == NULL)
	{
		showErrorAndExit("Scale could not find partner %s", self->objectiveName);
	}

	self->targetY = self->startY + (self->endY - self->startY) / 2;

	addChain();

	self->mental = getMapCeiling(self->x, self->y);

	self->action = &entityWait;
}

static void entityWait()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		self->dirY = self->speed;

		checkToMap(self);

		if (self->y >= self->endY)
		{
			self->y = self->endY;

			self->dirY = 0;
		}
	}

	else if (self->target->thinkTime > 0)
	{
		self->dirY = -self->speed;

		checkToMap(self);

		if (self->y <= self->startY)
		{
			self->y = self->startY;

			self->dirY = 0;
		}
	}

	else
	{
		self->dirY = (self->y < self->targetY ? 0.5 : -0.5);

		checkToMap(self);

		if ((self->dirY < 0 && self->y <= self->targetY) || (self->dirY > 0 && self->y >= self->targetY))
		{
			self->dirY = 0;

			self->y = self->targetY;
		}
	}
}

static void touch(Entity *other)
{
	int bottomBefore;

	if (other->dirY > 0)
	{
		/* Trying to move down */

		if (collision(other->x, other->y, other->w, other->h, self->x, self->y, self->w, self->h) == 1)
		{
			bottomBefore = other->y + other->h - other->dirY - 1;

			if (abs(bottomBefore - self->y) < self->h - 1)
			{
				/* Place the player as close to the solid tile as possible */

				other->y = self->y;
				other->y -= other->h;

				other->standingOn = self;
				other->dirY = 0;
				other->flags |= ON_GROUND;

				if (self->target->thinkTime <= 0)
				{
					self->thinkTime = 30;
				}
			}
		}
	}
}

static void addChain()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Scale Chain");
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
}

static int drawChain()
{
	int y;

	self->y = self->head->y - self->h;

	y = self->head->mental - self->h * 2;

	drawLoopingAnimationToMap();

	while (self->y >= y)
	{
		self->x = self->head->x;

		drawSpriteToMap();

		self->x = self->head->x + self->head->w - self->w;

		drawSpriteToMap();

		self->y -= self->h;
	}

	return TRUE;
}
