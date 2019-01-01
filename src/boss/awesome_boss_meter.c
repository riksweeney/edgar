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

#include "../audio/music.h"
#include "../collisions.h"
#include "../entity.h"
#include "../game.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../item/key_items.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void init(void);
static void entityWait(void);
static void entityWait2(void);
static void takeDamage(Entity *, int);
static void resumeNormalFunction(void);

Entity *addAwesomeBossMeter(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Awesome Boss Meter");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->takeDamage = &takeDamage;

	e->draw = &drawLoopingAnimationToMap;

	e->resumeNormalFunction = &resumeNormalFunction;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	if (self->active == TRUE)
	{
		initBossHealthBar();

		self->action = &entityWait;

		self->thinkTime = 180;

		self->health = 0;
	}
}

static void entityWait()
{
	Entity *e;

	if (self->damage == 0)
	{
		self->thinkTime--;

		self->flags &= ~FLY;

		if (self->thinkTime <= 0)
		{
			clearContinuePoint();

			freeBossHealthBar();

			e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

			e->y -= e->h;

			e->dirY = ITEM_JUMP_HEIGHT;

			fadeBossMusic();

			entityDieVanish();
		}
	}

	else if (self->damage == self->mental && self->health == self->maxHealth)
	{
		self->targetY--;

		if (self->targetY <= 0)
		{
			self->targetY = 0;
		}
	}

	checkToMap(self);
}

static void entityWait2()
{
	if (self->damage == 0)
	{
		self->thinkTime--;

		self->flags &= ~FLY;

		if (self->thinkTime <= 0)
		{
			self->action = self->die;
		}
	}

	else if (self->damage == self->mental && self->health == self->maxHealth)
	{
		self->targetY--;

		if (self->targetY <= 0)
		{
			self->targetY = 0;
		}
	}

	checkToMap(self);
}

static void takeDamage(Entity *other, int damage)
{
	if (self->health < self->maxHealth)
	{
		self->health += damage;

		if (self->health >= self->maxHealth)
		{
			self->health = self->maxHealth;

			setInfoBoxMessage(120, 255, 255, 255, _("Super is ready..."));

			self->targetX = prand() % 2;

			self->targetY = 120;
		}
	}
}

static void resumeNormalFunction()
{
	self->active = TRUE;

	self->action = &entityWait2;

	self->thinkTime = 180;

	self->health = 0;
}
