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
#include "../custom_actions.h"
#include "../entity.h"
#include "../event/global_trigger.h"
#include "../event/trigger.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../medal.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;
extern Game game;

static void init(void);
static void entityWait(void);
static void addTongue(void);
static void tongueWait(void);
static void tongueTouch(Entity *);
static void moveToMouth(void);
static void tongueTakeDamage(Entity *, int);
static int drawTongue(void);
static void creeperTouch(Entity *);
static void moveToMouthFinish(void);
static void tongueExtendOut(void);

Entity *addRedCeilingCreeper(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Red Ceiling Creeper");
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
	if (self->mental != 1)
	{
		addTongue();
	}

	self->action = &entityWait;
}

static void addTongue()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Red Ceiling Creeper Tongue");
	}

	loadProperties("enemy/ceiling_creeper_tongue", e);

	e->action = &tongueWait;
	e->draw = &drawTongue;
	e->touch = &tongueTouch;
	e->takeDamage = &tongueTakeDamage;
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
	self->health = self->maxHealth;

	self->action = &tongueExtendOut;

	self->touch = &tongueTouch;

	self->x = self->head->x + self->head->w / 2 - self->w / 2;
}

static void tongueTouch(Entity *other)
{
	if ((other->type == PLAYER || other->type == ENEMY) && self->target == NULL && other != self->head)
	{
		self->head->face = other->face;

		self->head->touch = &creeperTouch;

		self->target = other;

		self->target->flags |= FLY;

		self->thinkTime = 180;

		self->action = &moveToMouth;
	}

	else
	{
		entityTouch(other);
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
}

static void moveToMouth()
{
	self->thinkTime--;

	self->y -= self->speed;

	if (self->y <= self->startY)
	{
		if (self->target != NULL)
		{
			self->target->flags &= ~FLY;
		}

		self->target = NULL;

		self->y = self->startY;

		self->action = &moveToMouthFinish;

		self->thinkTime = 180;
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
	self->inUse = FALSE;
}

static void tongueTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

	playSoundToMap("sound/common/dink", -1, self->x, self->y, 0);

	if (other->reactToBlock != NULL)
	{
		temp = self;

		self = other;

		self->reactToBlock(temp);

		self = temp;
	}

	if (prand() % 10 == 0)
	{
		setInfoBoxMessage(60, 255, 255, 255, _("This weapon is not having any effect..."));
	}

	damage = 0;
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
	if (other->type == PLAYER)
	{
		other->health = 0;

		other->flags |= NO_DRAW;

		other->fallout();

		self->mental = 2;

		self->thinkTime = 180;

		self->touch = &entityTouch;

		playSoundToMap("sound/enemy/whirlwind/suck", -1, self->x, self->y, 0);

		game.timesEaten++;

		if (game.timesEaten == 5)
		{
			addMedal("eaten_5");
		}

		self->mental = 1;
	}

	else if (other->head == NULL || (other->head != NULL && other->head != self))
	{
		playSoundToMap("sound/enemy/whirlwind/suck", -1, self->x, self->y, 0);

		fireTrigger(self->objectiveName);

		fireGlobalTrigger(self->objectiveName);

		other->inUse = FALSE;

		self->mental = 1;
	}
}
