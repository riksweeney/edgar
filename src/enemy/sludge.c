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
#include "../geometry.h"
#include "../player.h"

extern Entity *self, player;

static void lookForPlayer(void);
static void teleportAttackStart(void);
static void teleportAttack(void);
static void teleportAttackFinishPause(void);
static void teleportAttackFinish(void);
static void touch(Entity *);
static void turnToFacePlayer(void);

Entity *addSludge(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Sludge\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &lookForPlayer;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->reactToBlock = &changeDirection;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->die = &entityDie;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void lookForPlayer()
{
	setEntityAnimation(self, STAND);
	
	self->action = &lookForPlayer;
	
	moveLeftToRight();
	
	if (player.health > 0 && prand() % 60 == 0)
	{
		if (collision(self->x + (self->face == RIGHT ? self->w : -160), self->y, 160, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->action = &teleportAttackStart;

			self->dirX = 0;
		}
	}
}

static void teleportAttackStart()
{
	setEntityAnimation(self, ATTACK_1);
	
	self->animationCallback = &teleportAttack;
	
	self->thinkTime = 180;
	
	checkToMap(self);
}

static void teleportAttack()
{
	self->action = &teleportAttack;
	
	self->touch = &touch;
	
	self->flags |= INVULNERABLE;
	
	setEntityAnimation(self, ATTACK_2);
	
	self->dirX = self->x < player.x ? self->speed * 3 : -self->speed * 3;
	
	if (isAtEdge(self) == 1)
	{
		self->dirX = 0;
	}
	
	checkToMap(self);
	
	if (self->dirX == 0)
	{	
		self->thinkTime--;
		
		if (self->thinkTime <= 0)
		{
			/* Give up */
			
			self->action = &teleportAttackFinish;
		}
	}
	
	else
	{
		if (fabs(self->x - player.x) < fabs(self->dirX))
		{
			self->action = &teleportAttackFinishPause;
			
			self->dirX = 0;
			
			self->thinkTime = 30;
		}
		
		else
		{
			self->thinkTime = 180;
		}
	}
}

static void teleportAttackFinishPause()
{
	checkToMap(self);
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->flags &= ~INVULNERABLE;
		
		self->action = &teleportAttackFinish;
	}
}

static void teleportAttackFinish()
{
	self->touch = &entityTouch;
	
	setEntityAnimation(self, ATTACK_3);
	
	self->animationCallback = &turnToFacePlayer;
	
	checkToMap(self);
}

static void turnToFacePlayer()
{
	facePlayer();
	
	self->dirX = self->face == LEFT ? -self->speed : self->speed;
	
	self->action = &lookForPlayer;
}

static void touch(Entity *other)
{

}
