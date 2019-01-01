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
#include "../system/random.h"

extern Entity *self;

static void init(void);
static void move(void);
static void moveToSkeleton(void);
static void resurrect(void);
static void resurrectFinish(void);
static void hover(void);
static void touch(Entity *);
static void creditsMove(void);

Entity *addGhost(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a G-G-G-G-G-Ghost!");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;

	e->creditsAction = &init;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	self->endY = MIN(getMapFloor(self->startX + self->w - 1, self->y), getMapFloor(self->startX, self->y));

	self->mental = 0;

	self->action = &move;

	self->creditsAction = &creditsMove;
}

static void move()
{
	moveLeftToRight();

	hover();

	self->health--;

	if (self->health <= 0)
	{
		playSoundToMap("sound/enemy/ghost/ghost", -1, self->x, self->y, 0);

		self->health = (6 + prand() % 10) * 60;
	}

	self->box.h = self->endY - self->y;
}

static void touch(Entity *other)
{
	if (self->mental == 0 && other->health == 0 && other->thinkTime == 0 &&
		(strcmpignorecase(other->name, "enemy/arrow_skeleton") == 0 ||
		strcmpignorecase(other->name, "enemy/sword_skeleton") == 0))
	{
		self->mental = 1;

		self->action = &moveToSkeleton;

		self->creditsAction = &moveToSkeleton;

		self->target = other;

		self->targetX = other->x + self->w / 2 - other->w / 2;

		self->targetY = other->y + other->h / 2 - self->h;

		self->thinkTime = 30;
	}
}

static void moveToSkeleton()
{
	if (fabs(self->targetX - self->x) <= fabs(self->dirX))
	{
		self->dirX = 0;

		self->thinkTime = 30;

		self->action = &resurrect;

		self->creditsAction = &resurrect;
	}

	else
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->dirX = 0;

			self->x = self->targetX;

			self->action = &resurrect;

			self->creditsAction = &resurrect;
		}
	}

	checkToMap(self);

	hover();
}

static void resurrect()
{
	self->dirY = self->speed / 2;

	checkToMap(self);

	if (self->dirY == 0 || self->y >= self->targetY)
	{
		self->dirY = 0;

		self->thinkTime = 60;

		self->target->health = self->target->maxHealth;

		self->action = &resurrectFinish;

		self->creditsAction = &resurrectFinish;
	}
}

static void resurrectFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirY = -self->speed / 2;

		checkToMap(self);

		if (self->y <= self->startY)
		{
			self->y = self->startY;

			self->dirX = self->face == LEFT ? -self->speed : self->speed;

			self->action = &move;

			self->creditsAction = &creditsMove;

			self->mental = 0;
		}
	}
}

static void hover()
{
	self->endX += 4;

	if (self->endX >= 360)
	{
		self->endX = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->endX)) * 4;
}

static void creditsMove()
{
	if (self->health == 2)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->face = RIGHT;

			setEntityAnimation(self, "STAND");

			self->dirX = self->speed;

			checkToMap(self);

			if (self->dirX == 0)
			{
				self->inUse = FALSE;
			}

			hover();

			self->maxHealth--;

			if (self->maxHealth <= 0)
			{
				playSoundToMap("sound/enemy/ghost/ghost", -1, self->x, self->y, 0);

				self->maxHealth = (6 + prand() % 10) * 60;
			}
		}
	}

	else
	{
		self->thinkTime = 120;
	}

	self->box.h = self->endY - self->y;
}
