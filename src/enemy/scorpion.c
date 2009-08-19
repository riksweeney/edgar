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
#include "../geometry.h"

extern Entity *self, player;
extern Game game;

static void lookForPlayer(void);
static void clawAttack(void);
static void stingAttack(void);
static void attackFinished(void);
static void attack(void);
static void takeDamage(Entity *, int);

Entity *addScorpion(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Scorpion\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &lookForPlayer;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDie;
	e->pain = NULL;
	e->touch = &entityTouch;
	e->takeDamage = &takeDamage;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
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
	setEntityAnimation(self, WALK);

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

	if (distance > 2)
	{
		setEntityAnimation(self, WALK);

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}

	else
	{
		self->dirX = 0;

		setEntityAnimation(self, STAND);

		move = prand() % 3;

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
	setEntityAnimation(self, ATTACK_1);

	self->flags |= ATTACKING;
	
	self->thinkTime = 60;

	self->animationCallback = &attackFinished;
	
	checkToMap(self);
}

static void stingAttack()
{
	setEntityAnimation(self, ATTACK_2);

	self->flags |= ATTACKING|UNBLOCKABLE;

	self->animationCallback = &attackFinished;
}

static void attackFinished()
{
	self->thinkTime--;
	
	self->action = &attackFinished;

	setEntityAnimation(self, STAND);

	self->flags &= ~(UNBLOCKABLE|ATTACKING);
	
	if (self->thinkTime <= 0)
	{
		if (collision(self->x + self->face == LEFT ? -160 : self->w, self->y, 160, self->h, player.x, player.y, player.w, player.h) == 1)
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
