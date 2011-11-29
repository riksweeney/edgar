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
#include "../entity.h"
#include "../collisions.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../audio/audio.h"

extern Entity *self, player;

static void lookForPlayer(void);
static void readyArrow(void);
static void attackPlayer(void);
static void slashInit(void);
static void slash(void);
static void addSwordSwing(void);
static void swordSwingWait(void);
static void swordSwingAttack(void);
static void swordSwingAttackFinish(void);
static void swordReactToBlock(Entity *);
static void creditsMove(void);
static void die(void);

Entity *addSwordSkeleton(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Sword Skeleton");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &lookForPlayer;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeDirection;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void die()
{
	entityDie();
}

static void lookForPlayer()
{
	setEntityAnimation(self, "WALK");

	moveLeftToRight();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}

	if (player.health > 0 && self->thinkTime <= 0)
	{
		/* Must be within a certain range */

		if (collision(self->x + (self->face == LEFT ? -240 : self->w), self->y, 240, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->thinkTime = 300;

			self->action = &attackPlayer;
		}
	}
}

static void attackPlayer()
{
	/* Get close to the player */

	facePlayer();

	if ((self->face == LEFT && abs(self->x - (player.x + player.w)) < 16) || (self->face == RIGHT && abs(player.x - (self->x + self->w)) < 16))
	{
		setEntityAnimation(self, "STAND");

		self->dirX = 0;

		self->mental = 3;

		self->thinkTime = 30;

		self->action = &slashInit;
	}

	else
	{
		setEntityAnimation(self, "WALK");

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->face = self->face == LEFT ? RIGHT : LEFT;

			self->action = &lookForPlayer;

			self->thinkTime = 240;
		}

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}

	checkToMap(self);
}

static void slashInit()
{
	self->thinkTime--;

	if (self->thinkTime == 0)
	{
		self->thinkTime = 60;

		self->action = &slash;

		setEntityAnimation(self, "ATTACK_1");

		self->flags |= ATTACKING;
	}

	checkToMap(self);
}

static void slash()
{
	if (self->mental == 0)
	{
		setEntityAnimation(self, "STAND");

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->endY--;

			if (self->endY <= 0)
			{
				self->thinkTime = 0;
			}

			else
			{
				self->thinkTime = 30;
			}

			self->thinkTime = 300;

			self->action = &attackPlayer;
		}
	}

	checkToMap(self);
}

static void addSwordSwing()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Skeleton Sword");
	}

	loadProperties("enemy/sword_skeleton_sword", e);

	e->x = 0;
	e->y = 0;

	e->action = &swordSwingWait;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = NULL;
	e->takeDamage = NULL;

	e->type = ENEMY;

	e->head = self;

	e->flags |= ATTACKING;

	setEntityAnimation(e, "STAND");
}

static void swordSwingWait()
{
	self->face = self->head->face;

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;

	if (self->head->flags & ATTACKING)
	{
		self->action = &swordSwingAttack;

		self->reactToBlock = &swordReactToBlock;

		setEntityAnimation(self, "ATTACK_1");

		self->animationCallback = &swordSwingAttackFinish;

		playSoundToMap("sound/edgar/swing.ogg", -1, self->x, self->y, 0);
	}

	else
	{
		self->damage = 0;

		self->flags |= NO_DRAW;
	}

	if (self->head->health <= 0)
	{
		self->inUse = FALSE;
	}
}

static void swordSwingAttack()
{
	self->flags &= ~NO_DRAW;

	self->damage = 1;

	if (self->face == LEFT)
	{
		self->x = self->head->x + self->head->w - self->w - self->offsetX;
	}

	else
	{
		self->x = self->head->x + self->offsetX;
	}

	self->y = self->head->y + self->offsetY;
}

static void swordSwingAttackFinish()
{
	self->damage = 0;

	self->flags |= NO_DRAW;

	setEntityAnimation(self, "STAND");

	self->action = &swordSwingWait;

	self->head->mental--;

	if (self->head->mental == 0)
	{
		self->head->flags &= ~ATTACKING;
	}
}

static void swordReactToBlock(Entity *other)
{
	self->damage = 0;
}
