/*
Copyright (C) 2009-2011 Parallel Realities

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
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../custom_actions.h"
#include "../projectile.h"
#include "../hud.h"
#include "../collisions.h"
#include "../item/item.h"
#include "../system/random.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"
#include "../system/error.h"

extern Entity *self, player;

static void lookForPlayer(void);
static void ballAttackInit(void);
static void ballAttack(void);
static void ballAttackWait(void);
static void ballAttackFinish(void);
static void takeDamage(Entity *, int);
static void reflect(Entity *);
static void die(void);

Entity *addPhantasmalMouth(int x, int y, char *name)
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

	setEntityAnimation(e, "STAND");

	return e;
}

static void lookForPlayer()
{
	if (player.health > 0)
	{
		if (collision(self->x + (self->face == RIGHT ? self->w : -320), self->y, 320, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->thinkTime = 180;

			self->action = &ballAttackInit;
		}
	}

	checkToMap(self);
}

static void ballAttackInit()
{
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

	self->action = &ballAttackWait;

	self->frameSpeed = 0;

	self->thinkTime = 60;

	e = addProjectile("enemy/phantasmal_shot", self, self->x + self->w / 2, self->y, 0, 3 * (self->flags & FLY) ? 1 : -1);

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
	if (other->mental < 30)
	{
		self->damage = 50;
	}

	else if (other->mental < 60)
	{
		self->damage = 30;
	}

	else if (other->mental < 90)
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

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;
	
	if (self->flags & INVULNERABLE)
	{
		return;
	}
	
	if (other->element == PHANTASMAL && damage >= 50)
	{
		self->die();
	}
	
	else
	{
		playSoundToMap("sound/common/dink.ogg", -1, self->x, self->y, 0);

		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

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
}

static void die()
{
	int i;
	Entity *e;
	char name[MAX_VALUE_LENGTH];
	
	playSoundToMap("sound/common/crumble.ogg", -1, self->x, self->y, 0);

	snprintf(name, sizeof(name), "%s_piece", self->name);

	fireTrigger(self->objectiveName);

	fireGlobalTrigger(self->objectiveName);

	for (i=0;i<9;i++)
	{
		e = addTemporaryItem(name, self->x, self->y, self->face, 0, 0);

		e->x += (self->w - e->w) / 2;
		e->y += (self->w - e->w) / 2;

		e->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimationByID(e, i);

		e->thinkTime = 60 + (prand() % 180);
	}

	self->damage = 0;

	self->inUse = FALSE;
}

static void ballAttackWait()
{
	checkToMap(self);
}
