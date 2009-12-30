/*
Copyright (C) 2009-2010 Parallel Realities

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
#include "../system/error.h"

extern Entity *self, player;

static void wander(void);
static void init(void);
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
		showErrorAndExit("No free slots to add a Sludge");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->reactToBlock = &changeDirection;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->die = &entityDie;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void wander()
{
	setEntityAnimation(self, STAND);

	self->action = &wander;

	moveLeftToRight();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &teleportAttackStart;

		self->dirX = 0;
	}
}

static void teleportAttackStart()
{
	setEntityAnimation(self, ATTACK_1);

	self->animationCallback = &teleportAttack;

	self->thinkTime = 120 + prand() % 180;

	checkToMap(self);
}

static void teleportAttack()
{
	if (self->action != teleportAttack)
	{
		self->action = &teleportAttack;

		self->dirX = (self->face == LEFT ? -self->speed : self->speed) * 3;
	}

	if (self->dirX == 0)
	{
		self->face = self->face == LEFT ? RIGHT : LEFT;
	}

	self->touch = &touch;

	self->flags |= INVULNERABLE;

	setEntityAnimation(self, ATTACK_2);

	self->dirX = (self->face == LEFT ? -self->speed : self->speed) * 3;

	if (isAtEdge(self) == 1)
	{
		self->dirX = 0;
	}

	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &teleportAttackFinishPause;

		self->dirX = 0;

		self->thinkTime = 30;
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

	self->frameSpeed = -1;

	setEntityAnimation(self, ATTACK_1);

	self->animationCallback = &turnToFacePlayer;

	checkToMap(self);
}

static void turnToFacePlayer()
{
	self->frameSpeed = 1;

	self->dirX = (prand() % 2 == 0) ? -self->speed : self->speed;

	self->action = &wander;

	self->thinkTime = 120 + prand() % 180;
}

static void touch(Entity *other)
{

}

static void init()
{
	self->thinkTime = 120 + prand() % 180;

	self->action = &wander;
}
