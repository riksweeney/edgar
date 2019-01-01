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
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;
extern Game game;

static void lookForPlayer(void);
static void clawAttack(void);
static void stingAttack(void);
static void attackFinished(void);
static void attack(void);
static void takeDamage(Entity *, int);
static void clawWait(void);
static void addClaw(void);
static void clawAttackFinished(void);
static void creditsMove(void);
static void die(void);

Entity *addScorpion(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Scorpion");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &addClaw;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->pain = NULL;
	e->touch = &entityTouch;
	e->takeDamage = &takeDamage;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void die()
{
	playSoundToMap("sound/enemy/jumping_slime/slime_die", -1, self->x, self->y, 0);

	entityDie();
}

static void addClaw()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Scorpion Claw");
	}

	loadProperties("enemy/scorpion_claw", e);

	e->action = &clawWait;
	e->draw = &drawLoopingAnimationToMap;
	e->pain = NULL;
	e->touch = NULL;

	e->creditsAction = &clawWait;

	e->head = self;

	e->type = ENEMY;

	e->flags |= ATTACKING;

	setEntityAnimation(e, "STAND");

	self->action = &lookForPlayer;
}

static void takeDamage(Entity *other, int damage)
{
	entityTakeDamageNoFlinch(other, damage);

	if (other->type == WEAPON && self->action != &attack && self->health > 0)
	{
		self->action = &attack;

		facePlayer();
	}
}

static void lookForPlayer()
{
	setEntityAnimation(self, "WALK");

	moveLeftToRight();

	if (player.health > 0 && prand() % 10 == 0)
	{
		if (collision(self->x + (self->face == LEFT ? -160 : self->w), self->y, 160, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->action = &attack;

			facePlayer();
		}
	}
}

static void attack()
{
	int distance, move;

	distance = getHorizontalDistance(self, &player);

	/* Move into attack range */

	if (distance > 18)
	{
		setEntityAnimation(self, "WALK");

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}

	else
	{
		self->dirX = 0;

		setEntityAnimation(self, "STAND");

		move = prand() % 3;

		self->mental = 1;

		switch (move)
		{
			case 0:
			case 1:
				self->action = &clawAttack;
			break;

			default:
				self->action = &stingAttack;
				self->action = &clawAttack;
			break;
		}
	}

	facePlayer();

	checkToMap(self);

	if (isAtEdge(self) == TRUE)
	{
		self->action = &lookForPlayer;
	}
}

static void clawAttack()
{
	self->dirX = 0;

	setEntityAnimation(self, "ATTACK_1");

	self->flags |= ATTACKING;

	self->thinkTime = 60;

	checkToMap(self);

	if (self->mental == 2)
	{
		self->action = &attackFinished;
	}
}

static void stingAttack()
{
	setEntityAnimation(self, "ATTACK_2");

	self->flags |= ATTACKING|UNBLOCKABLE;

	self->animationCallback = &attackFinished;
}

static void attackFinished()
{
	self->mental = 0;

	self->thinkTime--;

	self->dirX = 0;

	setEntityAnimation(self, "STAND");

	self->flags &= ~(UNBLOCKABLE|ATTACKING);

	if (self->thinkTime <= 0)
	{
		if (player.health > 0 && collision(self->x + (self->face == LEFT ? -160 : self->w), self->y, 160, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->action = &attack;
		}

		else
		{
			self->dirX = self->face == LEFT ? -self->speed : self->speed;

			self->action = &lookForPlayer;
		}
	}

	checkToMap(self);
}

static void clawWait()
{
	self->face = self->head->face;

	if (self->head->health <= 0)
	{
		entityDieNoDrop();
	}

	else if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}

	else
	{
		if (self->head->mental == 1)
		{
			self->touch = &entityTouch;

			self->animationCallback = &clawAttackFinished;

			self->dirX = self->face == LEFT ? -4 : 4;
		}
	}

	setEntityAnimation(self, getAnimationTypeAtIndex(self->head));

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;

	if (self->head->flags & FLASH)
	{
		self->flags |= FLASH;
	}

	else
	{
		self->flags &= ~FLASH;
	}
}

static void clawAttackFinished()
{
	self->touch = NULL;

	self->head->mental = 2;
}

static void creditsMove()
{
	if (self->mental == 0)
	{
		addClaw();

		self->mental = 1;
	}

	setEntityAnimation(self, "WALK");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}
