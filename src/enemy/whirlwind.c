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
#include "../graphics/animation.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void move(void);
static void entityWait(void);
static void touch(Entity *);
static void suckIn(void);
static void die(void);
static void addExitTrigger(Entity *);
static void creditsMove(void);

Entity *addWhirlwind(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Whirlwind");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &moveLeftToRight;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->touch = &touch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeDirection;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void move()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 60;

		self->dirX = 0;

		self->action = &entityWait;
	}

	else
	{
		moveLeftToRight();
	}
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 120 + prand() % 120;

		self->action = &move;
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	if (other->health > 0 && self->health > 0 && other->type == PLAYER && !(other->flags & INVULNERABLE))
	{
		if (self->target != NULL)
		{
			self->target->inUse = FALSE;
		}

		self->target = other;

		self->dirX = 0;

		self->thinkTime = 60;

		other->dirX = 0;
		other->dirY = 0;

		setPlayerLocked(TRUE);

		setPlayerLocked(FALSE);

		setCustomAction(other, &helpless, 60, 0, 0);
		setCustomAction(other, &invulnerableNoFlash, 60, 0, 0);

		self->action = &suckIn;

		playSoundToMap("sound/enemy/whirlwind/suck", -1, self->x, self->y, 0);

		self->targetX = self->x + self->w / 2 - other->w / 2;
	}

	else if (other->type == WEAPON && (other->flags & ATTACKING))
	{
		/* Take very little damage */

		if (self->takeDamage != NULL && !(self->flags & INVULNERABLE))
		{
			self->takeDamage(other, other->damage > 2 ? 2 : other->damage);
		}
	}

	else if (other->type == PROJECTILE && other->parent != NULL && other->parent != self && other->parent->type != ENEMY)
	{
		/* Reflect the projectile */

		other->face = other->face == LEFT ? RIGHT : LEFT;

		other->damage = 2;

		other->dirX *= -1;

		other->parent = self;

		playSoundToMap("sound/enemy/whirlwind/ricochet", -1, self->x, self->y, 0);
	}
}

static void suckIn()
{
	Entity *e;

	if (fabs(self->target->x - self->targetX) > fabs(4))
	{
		self->target->x += self->target->x > self->targetX ? -4 : 4;

		setCustomAction(self->target, &helpless, 60, 0, 0);
		setCustomAction(self->target, &invulnerableNoFlash, 60, 0, 0);
	}

	else
	{
		self->target->flags |= NO_DRAW;

		self->target->x = self->targetX;

		self->target->y = self->y;

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			e = removePlayerWeapon();

			if (e != NULL)
			{
				e->x = self->x;
				e->y = self->y;

				e->dirX = (6 + prand() % 3) * (prand() % 2 == 0 ? -1 : 1);
				e->dirY = -12;

				setCustomAction(e, &invulnerable, 120, 0, 0);

				addExitTrigger(e);
			}

			e = removePlayerShield();

			if (e != NULL)
			{
				e->x = self->x;
				e->y = self->y;

				e->dirX = (6 + prand() % 3) * (prand() % 2 == 0 ? -1 : 1);
				e->dirY = -12;

				setCustomAction(e, &invulnerable, 120, 0, 0);

				addExitTrigger(e);
			}

			setCustomAction(self->target, &invulnerable, 60, 0, 0);

			setPlayerStunned(30);

			self->target->dirX = (6 + prand() % 3) * (prand() % 2 == 0 ? -1 : 1);
			self->target->dirY = -8;

			self->target->flags &= ~NO_DRAW;

			self->target = NULL;

			self->thinkTime = 180 + prand() % 180;

			self->action = &move;
		}
	}

	checkToMap(self);
}

static void die()
{
	if (self->target != NULL)
	{
		if (self->target->type == PLAYER)
		{
			setCustomAction(self->target, &invulnerable, 60, 0, 0);

			setPlayerStunned(30);

			self->target->flags &= ~NO_DRAW;
		}

		else
		{
			self->target->inUse = FALSE;
		}
	}

	playSoundToMap("sound/enemy/whirlwind/whirlwind_die", -1, self->x, self->y, 0);

	entityDie();
}

static void addExitTrigger(Entity *e)
{
	char itemName[MAX_LINE_LENGTH];

	snprintf(itemName, MAX_LINE_LENGTH, "\"%s\" 1 UPDATE_EXIT \"WHIRLWIND\"", e->objectiveName);

	addGlobalTriggerFromScript(itemName);
}

static void creditsMove()
{
	setEntityAnimation(self, "STAND");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}
