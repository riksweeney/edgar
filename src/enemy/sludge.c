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
#include "../graphics/animation.h"
#include "../item/item.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void wander(void);
static void init(void);
static void teleportAttackStart(void);
static void teleportAttack(void);
static void teleportAttackFinishPause(void);
static void teleportAttackFinish(void);
static void touch(Entity *);
static void turnToFacePlayer(void);
static void lookForPlayer(void);
static void takeDamage(Entity *, int);
static void die(void);
static void redDie(void);
static void redTeleportAttackStart(void);
static void redTeleportAttack(void);
static void redTeleportAttackFinishPause(void);
static void redTeleportAttackFinish(void);
static void redTurnToFacePlayer(void);
static void vomitAttackStart(void);
static void vomit(void);
static void vomitAttackFinish(void);
static void vomitWait(void);
static void vomitFall(void);
static void creditsMove(void);
static void creditsRedMove(void);
static void creditsTeleportAttack(void);
static void creditsTeleportAttackFinish(void);

Entity *addSludge(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		if (strcmpignorecase(name, "red_sludge") == 0)
		{
			showErrorAndExit("No free slots to add a Red Sludge");
		}

		else
		{
			showErrorAndExit("No free slots to add a Sludge");
		}
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->reactToBlock = &changeDirection;

	if (strcmpignorecase(name, "enemy/red_sludge") == 0)
	{
		e->action = &lookForPlayer;
		e->takeDamage = &takeDamage;
		e->die = &redDie;

		e->creditsAction = &creditsRedMove;
	}

	else
	{
		e->action = &init;
		e->takeDamage = &entityTakeDamageNoFlinch;
		e->die = &die;

		e->creditsAction = &creditsMove;
	}

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void wander()
{
	setEntityAnimation(self, "STAND");

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
	setEntityAnimation(self, "ATTACK_1");

	self->animationCallback = &teleportAttack;

	self->thinkTime = 120 + prand() % 180;

	checkToMap(self);
}

static void teleportAttack()
{
	if (self->action != &teleportAttack)
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

	setEntityAnimation(self, "ATTACK_2");

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

		self->face = (prand() % 2 == 0) ? LEFT : RIGHT;
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

	setEntityAnimation(self, "ATTACK_1");

	self->animationCallback = &turnToFacePlayer;

	checkToMap(self);
}

static void turnToFacePlayer()
{
	self->frameSpeed = 1;

	self->dirX = self->face == LEFT ? -self->speed : self->speed;

	self->action = &wander;

	self->thinkTime = 120 + prand() % 180;
}

static void redTurnToFacePlayer()
{
	self->thinkTime = 120 + prand() % 180;

	self->frameSpeed = 1;

	self->dirX = self->face == LEFT ? -self->speed : self->speed;

	self->action = &lookForPlayer;

	self->creditsAction = &creditsRedMove;

	setEntityAnimation(self, "STAND");
}

static void touch(Entity *other)
{

}

static void init()
{
	self->thinkTime = 120 + prand() % 180;

	self->action = &wander;
}

static void lookForPlayer()
{
	self->thinkTime--;

	moveLeftToRight();

	if (player.health > 0 && (prand() % 60 == 0))
	{
		if (collision(self->x + (self->face == LEFT ? -128 : self->w), self->y, 128, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->thinkTime = 240;

			self->dirX = 0;

			self->action = &redTeleportAttackStart;
		}
	}

	else if (self->thinkTime <= 0)
	{
		self->action = &vomitAttackStart;

		playSoundToMap("sound/enemy/snail/spit", -1, self->x, self->y, 0);

		self->dirX = 0;

		self->thinkTime = 0;
	}
}

static void takeDamage(Entity *other, int damage)
{
	entityTakeDamageNoFlinch(other, damage);

	if ((prand() % 3 == 0) && self->face == other->face && self->health > 0 && self->dirX != 0)
	{
		self->dirX = 0;

		self->action = &redTeleportAttackStart;
	}
}

static void die()
{
	playSoundToMap("sound/enemy/sludge/sludge_die", -1, self->x, self->y, 0);

	entityDie();
}

static void redDie()
{
	Entity *e;

	if (prand() % 2 == 0)
	{
		e = dropCollectableItem("item/sludge_tentacle", self->x + self->w / 2, self->y, self->face);

		e->x -= e->w / 2;
	}

	playSoundToMap("sound/enemy/sludge/sludge_die", -1, self->x, self->y, 0);

	entityDie();
}

static void redTeleportAttackStart()
{
	setEntityAnimation(self, "ATTACK_1");

	self->animationCallback = &redTeleportAttack;

	checkToMap(self);
}

static void redTeleportAttack()
{
	float target = player.x - self->w / 2 + player.w / 2;

	self->action = &redTeleportAttack;

	if (fabs(target - self->x) <= fabs(self->dirX))
	{
		self->dirX = 0;

		self->thinkTime = 0;
	}

	else
	{
		self->dirX = target < self->x ? -self->speed * 3 : self->speed * 3;
	}

	self->touch = &touch;

	self->flags |= INVULNERABLE;

	setEntityAnimation(self, "ATTACK_2");

	if (isAtEdge(self) == 1)
	{
		self->dirX = 0;
	}

	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &redTeleportAttackFinishPause;

		self->dirX = 0;

		self->thinkTime = 30;
	}
}

static void redTeleportAttackFinishPause()
{
	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		facePlayer();

		self->flags &= ~INVULNERABLE;

		self->action = &redTeleportAttackFinish;
	}
}

static void redTeleportAttackFinish()
{
	self->touch = &entityTouch;

	self->frameSpeed = -1;

	setEntityAnimation(self, "ATTACK_1");

	self->animationCallback = &redTurnToFacePlayer;

	checkToMap(self);
}

static void vomitAttackStart()
{
	setEntityAnimation(self, "ATTACK_3");

	self->animationCallback = &vomit;

	checkToMap(self);
}

static void vomit()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Sludge Vomit");
	}

	loadProperties("enemy/sludge_vomit", e);

	if (self->face == LEFT)
	{
		e->x = self->x + self->w - e->w - self->offsetX;
	}

	else
	{
		e->x = self->x + self->offsetX;
	}

	e->y = self->y + self->offsetY;

	e->dirX = self->face == LEFT ? -5 : 5;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->action = &vomitFall;

	e->creditsAction = &vomitFall;

	setEntityAnimation(e, "STAND");

	self->action = &vomitAttackFinish;

	self->creditsAction = &vomitAttackFinish;

	checkToMap(self);
}

static void vomitAttackFinish()
{
	self->frameSpeed = -1;

	setEntityAnimation(self, "ATTACK_3");

	self->animationCallback = &redTurnToFacePlayer;

	checkToMap(self);
}

static void vomitFall()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		playSoundToMap("sound/enemy/red_sludge/acid", -1, self->x, self->y, 0);

		self->dirX = 0;

		setEntityAnimation(self, "ATTACK_1");

		self->action = &vomitWait;

		self->creditsAction = &vomitWait;
	}
}

static void vomitWait()
{
	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime < 90)
	{
		if (self->thinkTime % 3 == 0)
		{
			self->flags ^= NO_DRAW;
		}
	}

	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}
}

static void creditsRedMove()
{
	setEntityAnimation(self, "STAND");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}

	self->mental++;

	if (self->mental != 0 && (self->mental % 180) == 0)
	{
		self->creditsAction = &vomitAttackStart;

		playSoundToMap("sound/enemy/snail/spit", -1, self->x, self->y, 0);

		self->dirX = 0;

		self->thinkTime = 0;
	}
}

static void creditsMove()
{
	float dirX;

	self->creditsAction = &creditsMove;

	if (self->mental == 0)
	{
		setEntityAnimation(self, "ATTACK_1");

		self->animationCallback = &creditsTeleportAttack;

		self->thinkTime = 120;
	}

	else
	{
		setEntityAnimation(self, "STAND");

		self->dirX = self->speed;
	}

	dirX = self->dirX;

	checkToMap(self);

	if (self->dirX == 0 && dirX != 0 && self->mental != 0)
	{
		self->inUse = FALSE;
	}
}

static void creditsTeleportAttack()
{
	self->creditsAction = &creditsTeleportAttack;

	self->dirX = self->speed * 3;

	self->flags |= INVULNERABLE;

	setEntityAnimation(self, "ATTACK_2");

	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->creditsAction = &creditsTeleportAttackFinish;

		self->dirX = 0;

		self->thinkTime = 30 + prand() % 90;
	}
}

static void creditsTeleportAttackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental = 1;

		self->frameSpeed = -1;

		setEntityAnimation(self, "ATTACK_1");

		self->animationCallback = &creditsMove;
	}

	checkToMap(self);
}
