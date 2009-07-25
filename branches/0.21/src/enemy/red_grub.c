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

extern Entity *self, player;

static void die(void);
static void reactToBlock(void);
static void lookForPlayer(void);
static void spinAttackStart(void);
static void spinAttack(void);
static void spinAttackEnd(void);

Entity *addRedGrub(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Red Grub\n");

		exit(1);
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

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

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

		self->face = (self->face == RIGHT ? LEFT : RIGHT);
	}

	if (prand() % 120 == 0)
	{
		if (collision(self->x + (self->face == RIGHT ? self->w : -320), self->y, 320, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->action = &spinAttackStart;

			self->thinkTime = 60;
		}
	}
}

static void spinAttackStart()
{
	setEntityAnimation(self, ATTACK_2);

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
		self->speed = self->originalSpeed * 4;

		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);

		self->action = &spinAttack;

		self->thinkTime = 180;

		self->flags |= ATTACKING;
	}

	checkToMap(self);
}

static void spinAttack()
{
	float speed = self->dirX;

	self->thinkTime--;

	checkToMap(self);

	if (self->dirX == 0 || isAtEdge(self))
	{
		self->face = (player.x > self->x ? RIGHT : LEFT);

		self->dirX = speed < 0 ? 3 : -3;

		self->dirY = -6;

		self->action = &spinAttackEnd;

		self->thinkTime = 0;

		playSoundToMap("sound/enemy/red_grub/thud.ogg", -1, self->x, self->y, 0);
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

		setEntityAnimation(self, STAND);

		self->dirX = 0;

		self->flags &= ~(ATTACKING|INVULNERABLE);

		self->action = &lookForPlayer;

		self->frameSpeed = 1;

		self->speed = self->originalSpeed;
	}
}

static void reactToBlock()
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
		changeDirection();
	}
}
