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
#include "../custom_actions.h"
#include "../entity.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../hud.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void moveToTarget(void);
static void entityWait(void);
static void takeDamage(Entity *, int);
static void shudder(void);
static void touch(Entity *);
static void lookForPlayer(void);
static void attack(void);
static void init(void);
static void attackFinish(void);

Entity *addHugeSpider(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Huge Spider");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->takeDamage = &takeDamage;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	if (self->mental >= 50)
	{
		self->mental = 0;

		self->thinkTime = 0;
	}

	self->action = &entityWait;
}

static void entityWait()
{
	int x, y;
	int midX, midY;

	if (self->thinkTime > 0)
	{
		self->thinkTime--;
	}

	else if (self->thinkTime <= 0)
	{
		midX = self->startX + (self->endX - self->startX) / 2;
		midY = self->startY + (self->endY - self->startY) / 2;

		x = (prand() % 128);
		y = (prand() % 128);

		if (self->x > midX)
		{
			x *= -1;
		}

		if (self->y > midY)
		{
			y *= -1;
		}

		self->targetX = self->x + x;
		self->targetY = self->y + y;

		self->action = &moveToTarget;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= self->speed;
		self->dirY *= self->speed;

		self->thinkTime = 120;

		self->mental--;

		if (self->mental < 0)
		{
			self->mental = 0;
		}
	}

	lookForPlayer();
}

static void moveToTarget()
{
	checkToMap(self);

	if (self->x < self->startX || self->x > self->endX)
	{
		self->x = self->x < self->startX ? self->startX : self->endX;

		self->dirX = 0;

		self->dirY = 0;
	}

	if (self->y < self->startY || self->y > self->endY)
	{
		self->y = self->y < self->startY ? self->startY : self->endY;

		self->dirX = 0;

		self->dirY = 0;
	}

	if (atTarget() || (self->dirX == 0 && self->dirY == 0))
	{
		self->dirX = 0;

		self->dirY = 0;

		self->thinkTime = 60 + prand() % 180;

		self->action = &entityWait;
	}

	lookForPlayer();
}

static void lookForPlayer()
{
	int mid = self->x + self->w / 2;

	if (player.health > 0 && collision(mid - 23, self->y, 46, 640, player.x, player.y, player.w, player.h) == 1)
	{
		self->dirX = 0;

		self->dirY = 0;

		self->action = &attack;
	}
}

static void attack()
{
	int i;

	self->dirY = 12;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		playSoundToMap("sound/common/crash", BOSS_CHANNEL, self->x, self->y, 0);

		for (i=0;i<20;i++)
		{
			addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
		}

		self->thinkTime = 60;

		self->action = &attackFinish;
	}
}

static void attackFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->targetY = self->startY + (self->endY - self->startY) / 2;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= self->speed;
		self->dirY *= self->speed;

		self->action = &moveToTarget;
	}
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

	playSoundToMap("sound/common/dink", -1, self->x, self->y, 0);

	if (other->reactToBlock != NULL)
	{
		temp = self;

		self = other;

		self->reactToBlock(temp);

		self = temp;
	}

	if (other->type != PROJECTILE && prand() % 10 == 0)
	{
		setInfoBoxMessage(60, 255, 255, 255, _("This weapon is not having any effect..."));
	}

	damage = 0;
}

static void touch(Entity *other)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE) && other->type == ITEM && strcmpignorecase(other->name, "item/repellent_spray") == 0)
	{
		self->mental++;

		if (self->mental == 50)
		{
			self->action = &shudder;

			self->targetX = self->x;

			self->thinkTime = 300;

			if (player.health > 0)
			{
				setInfoBoxMessage(180, 255, 255, 255, _("Now! Run while it's stunned!"));
			}

			self->health = 0;
		}

		else if (self->mental > 50)
		{
			self->thinkTime = 300;
		}

		setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
		setCustomAction(self, &flashWhite, 6, 0, 0);
	}

	else if (other->type == PLAYER && self->action == &attack)
	{
		temp = self;

		self = other;

		freeEntityList(playerGib());

		self = temp;
	}

	else
	{
		entityTouch(other);
	}
}

static void shudder()
{
	self->health += 60;

	if (self->health >= 360)
	{
		self->health %= 360;
	}

	self->x = self->targetX + sin(DEG_TO_RAD(self->health)) * 4;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->health = 0;

		self->mental = 0;

		self->x = self->targetX;

		self->action = &entityWait;
	}

	checkToMap(self);
}
