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
#include "../audio/music.h"
#include "../collisions.h"
#include "../credits.h"
#include "../custom_actions.h"
#include "../entity.h"
#include "../event/trigger.h"
#include "../game.h"
#include "../graphics/animation.h"
#include "../graphics/gib.h"
#include "../hud.h"
#include "../item/key_items.h"
#include "../map.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void entityWait(void);
static void spitStart(void);
static void spit(void);
static void spitEnd(void);
static void initialise(void);
static void takeDamage(Entity *, int);
static void die(void);
static void attackFinished(void);
static void spinAttackStart(void);
static void spinAttack(void);
static void spinAttackEnd(void);
static void bounceAttackStart(void);
static void bounceAttack(void);
static void bounceAttackEnd(void);
static void doIntro(void);
static void introPause(void);
static void shudder(void);

Entity *addGrubBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Grub Boss");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;

	e->takeDamage = NULL;

	e->type = ENEMY;

	e->flags |= NO_DRAW|FLY;

	e->active = FALSE;

	e->creditsAction = &bossMoveToMiddle;

	setEntityAnimation(e, "STAND");

	return e;
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE))
	{
		self->health -= damage;

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);
			setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

			enemyPain();
		}

		else
		{
			self->health = 0;

			self->thinkTime = 180;

			self->flags &= ~FLY;

			setEntityAnimation(self, "STAND");

			self->frameSpeed = 0;

			self->takeDamage = NULL;

			self->touch = NULL;

			self->action = &die;

			self->startX = self->x;

			playSoundToMap("sound/boss/grub_boss/death", BOSS_CHANNEL, self->x, self->y, 0);
		}

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}
	}
}

static void initialise()
{
	if (self->active == TRUE)
	{
		if (cameraAtMinimum())
		{
			centerMapOnEntity(NULL);

			self->dirX = self->speed;

			setEntityAnimation(self, "ATTACK_2");

			self->action = &doIntro;

			self->flags &= ~NO_DRAW;
			self->flags &= ~FLY;

			self->touch = &entityTouch;

			self->endX = self->damage;

			initBossHealthBar();

			playDefaultBossMusic();

			setContinuePoint(FALSE, self->name, NULL);

			self->damage = 0;
		}
	}
}

static void doIntro()
{
	if (self->dirX == 0)
	{
		shakeScreen(MEDIUM, 15);

		playSoundToMap("sound/common/crunch", -1, self->x, self->y, 0);

		self->face = LEFT;

		self->dirX = -3;

		self->dirY = -8;

		self->thinkTime = 1;
	}

	else if ((self->flags & ON_GROUND) && self->thinkTime == 1)
	{
		setEntityAnimation(self, "STAND");

		self->thinkTime = 60;

		self->dirX = 0;

		self->action = &introPause;

		playSoundToMap("sound/boss/grub_boss/roar", BOSS_CHANNEL, self->x, self->y, 0);
	}

	checkToMap(self);
}

static void introPause()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->touch = &entityTouch;

		self->takeDamage = &takeDamage;

		self->damage = self->endX;

		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void entityWait()
{
	int attack;

	self->dirX = 0;

	self->thinkTime--;

	if (self->thinkTime <= 0 && player.health > 0)
	{
		attack = prand() % 3;

		switch (attack)
		{
			case 0:
				self->action = &spitStart;

				self->thinkTime = (prand() % 3) + 1;
			break;

			case 1:
				self->action = &spinAttackStart;

				self->thinkTime = 60;
			break;

			default:
				self->action = &bounceAttackStart;

				self->thinkTime = 60;
			break;
		}
	}

	checkToMap(self);
}

static void spitStart()
{
	if (self->frameSpeed > 0)
	{
		if (self->thinkTime > 0)
		{
			setEntityAnimation(self, "ATTACK_1");

			self->animationCallback = &spit;
		}

		else
		{
			self->action = &attackFinished;
		}
	}

	else
	{
		self->animationCallback = &spitEnd;
	}

	checkToMap(self);
}

static void spitEnd()
{
	self->frameSpeed *= -1;
}

static void spit()
{
	int x = (self->face == RIGHT ? self->offsetX : self->w - self->offsetX);
	int y = self->offsetY;

	addProjectile("boss/grub_boss_shot", self, self->x + x, self->y + y, (self->face == RIGHT ? 7 : -7), -12);
	addProjectile("boss/grub_boss_shot", self, self->x + x, self->y + y, (self->face == RIGHT ? 4 : -4), -12);
	addProjectile("boss/grub_boss_shot", self, self->x + x, self->y + y, (self->face == RIGHT ? 1 : -1), -12);

	if (self->health < 100)
	{
		addProjectile("boss/grub_boss_shot", self, self->x + x, self->y + y, (self->face == RIGHT ? 2.5 : -2.5), -12);
		addProjectile("boss/grub_boss_shot", self, self->x + x, self->y + y, (self->face == RIGHT ? 5.5 : -5.5), -12);
	}

	playSoundToMap("sound/boss/grub_boss/fire", BOSS_CHANNEL, self->x, self->y, 0);

	self->thinkTime--;

	self->frameSpeed *= -1;
}

static void spinAttackStart()
{
	self->flags |= INVULNERABLE;

	setEntityAnimation(self, "ATTACK_2");

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
		shakeScreen(MEDIUM, 15);

		playSoundToMap("sound/common/crunch", -1, self->x, self->y, 0);

		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);

		self->action = &spinAttack;
	}

	checkToMap(self);
}

static void spinAttack()
{
	float speed = self->dirX;

	self->flags |= INVULNERABLE;

	checkToMap(self);

	if (self->dirX == 0)
	{
		shakeScreen(MEDIUM, 15);

		playSoundToMap("sound/common/crunch", -1, self->x, self->y, 0);

		self->face = (player.x > self->x ? RIGHT : LEFT);

		self->dirX = speed < 0 ? 3 : -3;

		self->dirY = -6;

		self->action = &spinAttackEnd;

		self->thinkTime = 0;
	}
}

static void spinAttackEnd()
{
	checkToMap(self);

	if ((self->flags & ON_GROUND) && self->thinkTime == 0)
	{
		self->dirX = 0;

		self->action = &attackFinished;
	}
}

static void bounceAttackStart()
{
	self->flags |= INVULNERABLE;

	setEntityAnimation(self, "ATTACK_2");

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
		shakeScreen(MEDIUM, 15);

		playSoundToMap("sound/common/crunch", -1, self->x, self->y, 0);

		self->dirX = (self->face == RIGHT ? 3 : -3);

		self->dirY = -14;

		self->action = &bounceAttack;
	}

	checkToMap(self);
}

static void bounceAttack()
{
	float speed = self->dirX;

	self->flags |= INVULNERABLE;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		shakeScreen(MEDIUM, 15);

		playSoundToMap("sound/common/crunch", -1, self->x, self->y, 0);

		self->dirY = -14;
	}

	if (self->dirX == 0)
	{
		shakeScreen(MEDIUM, 15);

		playSoundToMap("sound/common/crunch", -1, self->x, self->y, 0);

		self->face = (player.x > self->x ? RIGHT : LEFT);

		self->dirX = speed < 0 ? 3 : -3;

		self->dirY = -6;

		self->action = &bounceAttackEnd;

		self->thinkTime = 0;
	}
}

static void bounceAttackEnd()
{
	checkToMap(self);

	if ((self->flags & ON_GROUND) && self->thinkTime == 0)
	{
		self->dirX = 0;

		self->action = &attackFinished;
	}
}

static void attackFinished()
{
	self->flags &= ~INVULNERABLE;

	setEntityAnimation(self, "STAND");

	self->frameSpeed = 1;

	self->thinkTime = 90;

	self->action = &entityWait;
}

static void die()
{
	Entity *e;

	self->thinkTime--;

	self->takeDamage = NULL;

	if (self->thinkTime <= 0)
	{
		clearContinuePoint();

		increaseKillCount();

		freeBossHealthBar();

		fireTrigger(self->objectiveName);

		freeEntityList(throwGibs("boss/grub_boss_gib", 7));

		e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

		e->dirY = ITEM_JUMP_HEIGHT;

		fadeBossMusic();
	}

	checkToMap(self);

	shudder();
}

static void shudder()
{
	self->startY += 90;

	if (self->startY >= 360)
	{
		self->startY = 0;
	}

	self->x = self->startX + sin(DEG_TO_RAD(self->startY)) * 4;
}
