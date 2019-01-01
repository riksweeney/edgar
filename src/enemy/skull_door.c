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
#include "../graphics/decoration.h"
#include "../hud.h"
#include "../item/item.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void lookForPlayer(void);
static void skullAttackChargeUp(void);
static void skullAttack(void);
static void skullAttackFinish(void);
static void takeDamage(Entity *, int);
static void skullShotMove(void);
static void touch(Entity *);
static void skullShotReflect(Entity *);
static void die(void);

Entity *addSkullDoor(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Skull Door");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
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
		if (collision(self->x + (self->face == LEFT ? -400 : self->w), self->y, 400, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			setEntityAnimation(self, "CHARGE");

			self->animationCallback = &skullAttackChargeUp;
		}
	}

	checkToMap(self);
}

static void skullAttackChargeUp()
{
	setEntityAnimation(self, "CHARGE_WAIT");

	self->thinkTime = 30;

	self->action = &skullAttack;
}

static void skullAttack()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "ATTACK");

		e = addProjectile("enemy/skull_shot", self, self->x, self->y, self->face == LEFT ? -6 : 6, 0);

		playSoundToMap("sound/boss/snake_boss/snake_boss_shot", -1, self->x, self->y, 0);

		if (self->face == LEFT)
		{
			e->x = self->x + self->w - e->w - self->offsetX;
		}

		else
		{
			e->x = self->x + self->offsetX;
		}

		e->y = self->y + self->offsetY;

		e->face = self->face;

		e->action = &skullShotMove;

		e->flags |= FLY;

		e->reactToBlock = &skullShotReflect;

		e->thinkTime = 1200;

		e->mental = 2;

		self->thinkTime = 60;

		self->action = &skullAttackFinish;
	}

	checkToMap(self);
}

static void skullAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "STAND");

		self->action = &lookForPlayer;
	}

	checkToMap(self);
}

static void skullShotMove()
{
	Entity *e;

	self->dirX = self->face == LEFT ? -fabs(self->dirX) : fabs(self->dirX);

	self->mental--;

	if (self->mental <= 0)
	{
		e = addBasicDecoration(self->x, self->y, "decoration/skull_trail");

		if (e != NULL)
		{
			e->x = self->face == LEFT ? self->x + self->w - e->w : self->x;

			e->y = self->y + self->h / 2 - e->h / 2;

			e->y += (prand() % 8) * (prand() % 2 == 0 ? 1 : -1);

			e->thinkTime = 15 + prand() % 15;

			e->dirY = (1 + prand() % 10) * (prand() % 2 == 0 ? 1 : -1);

			e->dirY /= 10;
		}

		self->mental = 2;
	}

	checkToMap(self);

	if (self->dirX == 0 || self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
}

static void skullShotReflect(Entity *other)
{
	if (other->element != PHANTASMAL)
	{
		self->inUse = FALSE;

		return;
	}

	if (other->mental <= 7)
	{
		self->damage = 50;
	}

	else if (other->mental <= 15)
	{
		self->damage = 30;
	}

	else if (other->mental <= 30)
	{
		self->damage = 20;
	}

	else
	{
		self->damage = 5;
	}

	self->parent = other;

	self->face = self->face == LEFT ? RIGHT : LEFT;
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (other->element == PHANTASMAL)
	{
		entityTakeDamageNoFlinch(other, damage);
	}

	else
	{
		playSoundToMap("sound/common/dink", -1, self->x, self->y, 0);

		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

		if (other->reactToBlock != NULL)
		{
			temp = self;

			self = other;

			self->reactToBlock(temp);

			self = temp;
		}

		if (other->type != PROJECTILE && prand() % 10 == 0)
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

	playSoundToMap("sound/common/crumble", -1, self->x, self->y, 0);

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

	if (!(self->flags & INVULNERABLE))
	{
		self->touch = &entityTouch;

		self->flags &= ~FLY;

		self->flags |= (DO_NOT_PERSIST|NO_DRAW);

		self->thinkTime = 60;

		setCustomAction(self, &invulnerableNoFlash, 240, 0, 0);

		self->frameSpeed = 0;

		self->action = &standardDie;

		self->damage = 0;
	}
}

static void touch(Entity *other)
{
	if (!(other->parent != NULL && other->parent == self))
	{
		if (other->flags & ATTACKING)
		{
			takeDamage(other, other->damage);
		}

		if (self->inUse == TRUE && self->touch != NULL)
		{
			pushEntity(other);
		}
	}
}
