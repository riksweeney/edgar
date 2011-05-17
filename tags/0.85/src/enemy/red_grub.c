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
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../collisions.h"
#include "../system/error.h"

extern Entity *self, player;

static void die(void);
static void reactToBlock(Entity *);
static void lookForPlayer(void);
static void spinAttackStart(void);
static void spinAttack(void);
static void spinAttackEnd(void);
static void creditsMove(void);
static void creditsSpinAttack(void);

Entity *addRedGrub(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Red Grub");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &lookForPlayer;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &die;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &reactToBlock;
	e->resumeNormalFunction = &spinAttack;
	
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
	checkToMap(self);

	if (self->dirX == 0 || isAtEdge(self) == TRUE)
	{
		self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

		self->face = (self->dirX < 0 ? LEFT : RIGHT);
	}

	if (player.health != 0 && prand() % 120 == 0)
	{
		if (collision(self->x + (self->face == RIGHT ? self->w : -320), self->y, 320, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->dirX = 0;

			self->action = &spinAttackStart;

			self->thinkTime = 60;
		}
	}
}

static void spinAttackStart()
{
	setEntityAnimation(self, "ATTACK_2");

	self->flags |= INVULNERABLE;

	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			self->face = (player.x > self->x ? RIGHT : LEFT);

			self->frameSpeed = 2;

			self->dirY = -8;
		}
	}

	else if (self->thinkTime == 0 && self->flags & ON_GROUND)
	{
		playSoundToMap("sound/enemy/red_grub/spin.ogg", -1, self->x, self->y, 0);

		self->speed = self->originalSpeed * 4;

		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);

		self->action = &spinAttack;
		
		self->creditsAction = &creditsSpinAttack;

		self->thinkTime = 180;

		self->flags |= ATTACKING;
	}

	checkToMap(self);
}

static void spinAttack()
{
	self->thinkTime--;

	checkToMap(self);

	if (self->dirX == 0 || isAtEdge(self))
	{
		if (self->dirX == 0)
		{
			self->dirX = self->face == LEFT ? 3 : -3;
		}

		else
		{
			self->dirX = self->dirX < 0 ? 3 : -3;
		}

		self->dirY = -6;

		self->action = &spinAttackEnd;

		self->thinkTime = 0;

		playSoundToMap("sound/enemy/red_grub/thud.ogg", -1, self->x, self->y, 0);

		self->face = (player.x > self->x ? RIGHT : LEFT);
	}

	else if (self->thinkTime <= 0)
	{
		self->action = &spinAttackEnd;

		self->thinkTime = 0;
	}
}

static void spinAttackEnd()
{
	checkToMap(self);

	if ((self->flags & ON_GROUND) && self->thinkTime == 0)
	{
		self->face = (player.x > self->x ? RIGHT : LEFT);

		setEntityAnimation(self, "STAND");

		self->dirX = 0;

		self->flags &= ~(ATTACKING|INVULNERABLE);

		self->action = &lookForPlayer;

		self->frameSpeed = 1;

		self->speed = self->originalSpeed;
	}
}

static void reactToBlock(Entity *other)
{
	if (player.face == LEFT)
	{
		self->x = player.x - self->w;
	}

	else
	{
		self->x = player.x + player.w;
	}

	if (self->action == &spinAttack)
	{
		self->dirX = player.face == LEFT ? -5 : 5;

		self->dirY = -6;

		self->action = &spinAttackEnd;

		self->thinkTime = 0;
	}

	else
	{
		changeDirection(NULL);
	}
}

static void creditsMove()
{
	self->dirX = self->speed;
	
	checkToMap(self);
	
	self->thinkTime++;
	
	if (self->thinkTime >= 180)
	{
		self->dirX = 0;

		self->creditsAction = &spinAttackStart;

		self->thinkTime = 60;
	}
}

static void creditsSpinAttack()
{
	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}
