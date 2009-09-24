/*
Copyright (C) 2009 Parallel Realities

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
#include "../system/random.h"
#include "../audio/audio.h"
#include "../collisions.h"
#include "../custom_actions.h"
#include "../game.h"
#include "../player.h"
#include "../item/item.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"

extern Entity *self, player;

static void lookForPlayer(void);
static void stompAttackInit(void);
static void stompAttack(void);
static void stompAttackFinish(void);
static void attacking(void);
static void die(void);

Entity *addCenturion(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Centurion\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &lookForPlayer;
	e->die = &die;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeDirection;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void lookForPlayer()
{
	if (self->currentFrame == 2 || self->currentFrame == 5)
	{
		if (self->maxThinkTime == 0)
		{
			playSoundToMap("sound/enemy/centurion/walk.ogg", -1, self->x, self->y, 0);

			self->maxThinkTime = 1;
		}

		self->dirX = 0;
	}

	else
	{
		self->maxThinkTime = 0;
	}

	checkToMap(self);

	if (self->currentFrame == 2 || self->currentFrame == 5)
	{
		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
	}

	if (self->dirX == 0 || isAtEdge(self) == TRUE)
	{
		self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

		self->face = (self->face == RIGHT ? LEFT : RIGHT);
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}

	if (self->thinkTime == 0 && player.health > 0 && prand() % 15 == 0)
	{
		if (collision(self->x + (self->face == RIGHT ? self->w : -160), self->y, 160, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->action = &stompAttackInit;

			self->dirX = 0;
		}
	}
}

static void stompAttackInit()
{
	setEntityAnimation(self, ATTACK_1);

	self->animationCallback = &stompAttack;

	self->action = &attacking;

	checkToMap(self);
}

static void stompAttack()
{
	setEntityAnimation(self, ATTACK_2);

	playSoundToMap("sound/common/crash.ogg", -1, self->x, self->y, 0);

	shakeScreen(MEDIUM, 60);

	self->thinkTime = 60;

	if (player.flags & ON_GROUND)
	{
		setPlayerStunned(120);
	}

	self->action = &stompAttackFinish;
}

static void stompAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, STAND);

		self->action = &lookForPlayer;

		self->thinkTime = 60;

		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
	}
}

static void attacking()
{
	checkToMap(self);
}

static void die()
{
	int i;
	Entity *e;
	char name[MAX_VALUE_LENGTH];

	snprintf(name, sizeof(name), "%s_piece", self->name);

	fireTrigger(self->objectiveName);

	fireGlobalTrigger(self->objectiveName);

	for (i=0;i<7;i++)
	{
		e = addTemporaryItem(name, self->x, self->y, self->face, 0, 0);

		e->x += (self->w - e->w) / 2;
		e->y += (self->w - e->w) / 2;

		e->dirX = (prand() % 5) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		if (i == 2 || i == 3)
		{
			setEntityAnimation(e, i == 2 ? 0 : 1);
		}

		else
		{
			setEntityAnimation(e, i);
		}

		e->thinkTime = 60 + (prand() % 180);
	}

	self->damage = 0;

	if (!(self->flags & INVULNERABLE))
	{
		self->flags &= ~FLY;

		self->flags |= (DO_NOT_PERSIST|NO_DRAW);

		self->thinkTime = 60;

		setCustomAction(self, &invulnerableNoFlash, 240, 0);

		self->frameSpeed = 0;

		self->action = &standardDie;

		self->damage = 0;
	}
}
