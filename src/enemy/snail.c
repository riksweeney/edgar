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
#include "../graphics/animation.h"
#include "../hud.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "snail_shell.h"

extern Entity *self, player;

static void takeDamage(Entity *, int);
static void die(void);
static void lookForPlayer(void);
static void spitAttackInit(void);
static void spitAttack(void);
static void spitAttackFinish(void);
static void attacking(void);
static void creditsMove(void);
static void creditsPurpleMove(void);

Entity *addSnail(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Snail");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	if (strcmpignorecase(name, "enemy/purple_snail") == 0)
	{
		e->action = &lookForPlayer;

		e->creditsAction = &creditsPurpleMove;
	}

	else
	{
		e->action = &moveLeftToRight;

		e->creditsAction = &creditsMove;
	}

	e->die = &die;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->takeDamage = &takeDamage;
	e->reactToBlock = &changeDirection;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE))
	{
		/* Can't be hurt if not facing the player unless using pickaxe */

		if (self->face == other->face)
		{
			if (strcmpignorecase(other->name, "weapon/pickaxe") != 0)
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

				if (prand() % 10 == 0)
				{
					setInfoBoxMessage(60, 255, 255, 255, _("This weapon is not having any effect..."));
				}

				damage = 0;
			}

			else
			{
				entityTakeDamageNoFlinch(other, damage * 5);
			}
		}

		else
		{
			entityTakeDamageNoFlinch(other, damage);
		}
	}
}

static void die()
{
	Entity *e;

	if ((prand() % 3 == 0) && isSpaceEmpty(self) == NULL)
	{
		/* Drop a shell */

		setEntityAnimation(self, "WALK");

		if (strcmpignorecase(self->name, "enemy/purple_snail") == 0)
		{
			e = addSnailShell(self->x, self->y, "enemy/purple_snail_shell");
		}

		else
		{
			e = addSnailShell(self->x, self->y, "enemy/snail_shell");
		}

		e->face = self->face;

		e->x += (self->w - e->w) / 2;
	}

	playSoundToMap("sound/enemy/snail/snail_die", -1, self->x, self->y, 0);

	entityDie();
}

static void lookForPlayer()
{
	checkToMap(self);

	if (self->dirX == 0 || isAtEdge(self) == TRUE)
	{
		self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

		self->face = (self->face == RIGHT ? LEFT : RIGHT);
	}

	if (player.health > 0 && prand() % 30 == 0)
	{
		if (collision(self->x + (self->face == RIGHT ? self->w : -320), self->y, 320, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->action = &spitAttackInit;

			self->dirX = 0;
		}
	}
}

static void spitAttackInit()
{
	setEntityAnimation(self, "ATTACK_1");

	playSoundToMap("sound/enemy/snail/spit", -1, self->x, self->y, 0);

	self->animationCallback = &spitAttack;

	self->action = &attacking;

	self->creditsAction = &attacking;

	checkToMap(self);
}

static void spitAttack()
{
	int x, y;
	Entity *e;

	x = self->x + (self->face == LEFT ? -5 : self->w - 6);
	y = self->y + 21;

	e = addProjectile("common/green_blob", self, x, y, (self->face == LEFT ? -6 : 6), 0);

	e->y -= e->h / 2;

	e->flags |= FLY;

	e->reactToBlock = &bounceOffShield;

	setEntityAnimation(self, "ATTACK_2");

	self->animationCallback = &spitAttackFinish;
}

static void spitAttackFinish()
{
	setEntityAnimation(self, "STAND");

	self->action = &lookForPlayer;

	self->creditsAction = &creditsPurpleMove;

	self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
}

static void attacking()
{
	checkToMap(self);
}

static void creditsMove()
{
	Entity *e;

	self->thinkTime++;

	setEntityAnimation(self, "STAND");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->thinkTime >= 900)
	{
		e = addSnailShell(self->x, self->y, "enemy/snail_shell");

		e->face = self->face;

		e->touch = NULL;

		self->inUse = FALSE;
	}
}

static void creditsPurpleMove()
{
	self->mental++;

	setEntityAnimation(self, "STAND");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}

	if (self->mental != 0 && (self->mental % 300) == 0)
	{
		self->creditsAction = &spitAttackInit;

		self->dirX = 0;
	}
}
