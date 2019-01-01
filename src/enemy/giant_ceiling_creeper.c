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

extern Entity *self, player;
extern Game game;

static void init(void);
static void entityWait(void);
static void addTongue(void);
static void tongueWait(void);
static void tongueTouch(Entity *);
static void moveToMouth(void);
static void tongueExtendOut(void);
static int drawTongue(void);
static void creeperTouch(Entity *);
static void moveToMouthFinish(void);

Entity *addGiantCeilingCreeper(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Ceiling Creeper");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	addTongue();

	self->action = &entityWait;
}

static void addTongue()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Giant Ceiling Creeper Tongue");
	}

	loadProperties("enemy/giant_ceiling_creeper_tongue", e);

	e->action = &tongueWait;
	e->draw = &drawTongue;
	e->touch = &tongueTouch;
	e->pain = &enemyPain;

	e->type = ENEMY;

	e->head = self;

	setEntityAnimation(e, "STAND");

	e->x = self->x + self->w / 2 - e->w / 2;

	e->y = self->y + self->h / 2 - e->h / 2;

	e->startY = e->y;
	e->endY = self->endY;
}

static void entityWait()
{
	checkToMap(self);
}

static void tongueWait()
{
	if (self->head->mental == 1)
	{
		self->health = self->maxHealth;

		self->action = &tongueExtendOut;

		self->touch = &tongueTouch;

		playSoundToMap("sound/boss/armour_boss/tongue_start", EDGAR_CHANNEL, self->x, self->y, 0);
	}

	self->x = self->head->x + self->head->w / 2 - self->w / 2;
}

static void tongueTouch(Entity *other)
{
	int y;

	if (self->target == NULL && other->type == ENEMY && other != self->head)
	{
		self->head->face = other->face;

		self->head->touch = &creeperTouch;

		self->target = other;

		self->target->flags |= FLY;

		self->action = &moveToMouth;

		y = other->y + other->h - self->h;

		if (y < self->endY)
		{
			self->y = y;
		}
	}

	else
	{
		entityTouch(other);
	}
}

static void moveToMouth()
{
	if (self->head->mental == 2)
	{
		self->y -= self->head->speed;

		if (self->y <= self->startY)
		{
			if (self->target != NULL)
			{
				self->target->inUse = FALSE;
			}

			self->target = NULL;

			self->y = self->startY;

			self->action = &moveToMouthFinish;

			self->thinkTime = 180;
		}
	}

	if (self->target != NULL)
	{
		self->target->x = self->x + self->w / 2 - self->target->w / 2;
		self->target->y = self->y + self->h / 2 - self->target->h / 2;

		self->target->dirY = 0;
	}
}

static void moveToMouthFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &tongueWait;
	}
}

static void tongueExtendOut()
{
	self->y += self->speed;

	if (self->y >= self->endY)
	{
		self->y = self->endY;
	}

	self->box.h = self->endY - self->startY;
	self->box.y = -self->box.h;
}

static int drawTongue()
{
	float y;

	y = self->y;

	setEntityAnimation(self, "WALK");

	while (self->y >= self->startY)
	{
		drawSpriteToMap();

		self->y -= self->h;
	}

	setEntityAnimation(self, "STAND");

	self->y = y;

	drawLoopingAnimationToMap();

	return TRUE;
}

static void creeperTouch(Entity *other)
{
	if (other->head == NULL || (other->head != NULL && other->head != self))
	{
		playSoundToMap("sound/enemy/whirlwind/suck", EDGAR_CHANNEL, self->x, self->y, 0);

		other->action = other->die;

		self->touch = NULL;
	}
}
