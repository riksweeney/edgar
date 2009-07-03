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
#include "../collisions.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "enemies.h"
#include "../custom_actions.h"
#include "../hud.h"
#include "../player.h"

extern Entity *self, player;
extern Game game;

static void lookForPlayer(void);
static void clawAttack(void);
static void stingAttack(void);
static void attackFinished(void);
static void attack(void);

Entity *addScorpion(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Baby Slime\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &lookForPlayer;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDie;
	e->pain = NULL;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = NULL;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void lookForPlayer()
{
	checkToMap(self);

	if (self->dirX == 0)
	{
		self->face = self->face == LEFT ? RIGHT : LEFT;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}

	if (prand() % 5 == 0)
	{
		if (collision(self->x - 320, self->y, 640, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->action = &attack;

			self->dirX = 0;
		}
	}

	facePlayer();
}

static void attack()
{
	int width = self->face == LEFT ? self->x : self->x + self->w - 1;
	int move;

	/* Move into attack range */

	if (abs(width - self->x) > 32)
	{
		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		self->thinkTime = 60 + prand() % 120;
	}

	else
	{
		move = prand() % 3;

		switch (move)
		{
			case 0:
			case 1:
				self->action = &clawAttack;
			break;

			default:
				self->action = &stingAttack;
			break;
		}
	}

	facePlayer();

	checkToMap(self);
}

static void clawAttack()
{
	setEntityAnimation(self, ATTACK_1);

	self->animationCallback = &attackFinished;
}

static void stingAttack()
{
	setEntityAnimation(self, ATTACK_2);

	self->flags |= UNBLOCKABLE;

	self->animationCallback = &attackFinished;
}

static void attackFinished()
{
	self->flags &= ~UNBLOCKABLE;

	self->thinkTime = 60 + prand() % 120;

	if (collision(self->x - 320, self->y, 640, self->h, player.x, player.y, player.w, player.h) == 1)
	{
		self->action = &attack;
	}

	else
	{
		self->action = &lookForPlayer;
	}
}
