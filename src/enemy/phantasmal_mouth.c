/*
Copyright (C) 2009-2010 Parallel Realities

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
#include "../system/properties.h"
#include "../entity.h"
#include "../medal.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../collisions.h"
#include "../geometry.h"
#include "../system/error.h"

extern Entity *self, player;

static void lookForPlayer(void);
static void ballAttackInit(void);
static void ballAttack(void);
static void ballFinish(void);

Entity *addBat(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Phantasmal Mouth");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &die;
	e->takeDamage = &takeDamage;

	e->action = &lookForPlayer;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void lookForPlayer()
{
	if (player.health > 0)
	{
		if (collision(self->x + (self->face == RIGHT ? self->w : -160), self->y, 160, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->thinkTime = 180;

			self->action = &ballAttackInit;
		}
	}

	checkToMap(self);
}

static void ballAttackInit()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		ballAttack();

		self->thinkTime = 60;

		self->action = &ballAttackFinish;
	}

	checkToMap(self);
}

static void ballAttack()
{
	Entity *e;

	self->action = &attackWait;

	self->frameSpeed = 0;

	self->thinkTime = 60;

	e = addProjectile("common/green_blob", self, self->x + self->w / 2, self->y, 0, 3 * (self->flags & FLY) ? 1 : -1);

	e->x -= e->w / 2;

	e->flags |= FLY;

	e->y += !(self->flags & FLY) > 0 ? 5 : (self->h - 5);

	e->type = ENEMY;

	e->parent = NULL;

	e->element = PHANTASMAL;

	e->reactToBlock = &reflect;

	e->flags |= DO_NOT_PERSIST;

	e->head = self;

	e->thinkTime = 5;
}

static void ballAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &lookForPlayer;
	}

	checkToMap(self);
}

static void reflect(Entity *other)
{
	if (other->mental < 15)
	{
		self->damage = 50;
	}

	else if (other->mental < 30)
	{
		self->damage = 30;
	}

	else if (other->mental < 60)
	{
		self->damage = 20;
	}

	else
	{
		self->damage = 10;
	}

	self->parent = other;

	self->dirX = -self->dirX;
}
