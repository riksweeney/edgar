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
#include "../graphics/animation.h"
#include "../medal.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self, player;
extern Game game;

static void init(void);
static void entityWait(void);
static void addTongue(void);
static void tongueWait(void);
static void tongueTouch(Entity *);
static void moveToMouth(void);
static void tongueTakeDamage(Entity *, int);
static void tongueRetreat(void);
static void tongueExtendOut(void);
static int drawTongue(void);
static void creeperTouch(Entity *);
static void moveToMouthFinish(void);
static void creditsAction(void);
static void creditsMove(void);

Entity *addCeilingCreeper(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Ceiling Creeper");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;

	e->creditsAction = &creditsAction;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	addTongue();

	self->action = &entityWait;
}

static void addTongue()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Ceiling Creeper Tongue");
	}

	loadProperties("enemy/ceiling_creeper_tongue", e);

	e->action = &tongueWait;
	e->draw = &drawTongue;
	e->touch = &tongueTouch;
	e->takeDamage = &tongueTakeDamage;
	e->pain = &enemyPain;

	e->type = ENEMY;

	e->head = self;

	setEntityAnimation(e, "STAND");

	e->x = self->x + self->w / 2 - e->w / 2;

	e->y = self->y + self->h / 2 - e->h / 2;

	e->startY = e->y;
	e->endY = self->endY;
}

static void entityWait()
{
	int height;

	if (self->mental == 0 && self->startX != self->endX)
	{
		if (self->face == LEFT)
		{
			if (self->x <= self->startX)
			{
				self->x = self->startX;

				self->face = RIGHT;
			}
		}

		else
		{
			if (self->x + self->w >= self->endX)
			{
				self->x = self->endX - self->w;

				self->face = LEFT;
			}
		}

		self->dirX = self->face == LEFT ? -self->speed : self->speed;
	}

	height = self->endY - self->startY;

	if (player.health > 0 && self->mental == 0 && !(player.flags & FLY))
	{
		if (self->startX == self->endX &&
			collision(self->x, self->y, self->w, height, player.x ,player.y, player.w, player.h) == 1)
		{
			self->dirX = 0;

			self->mental = 1;
		}

		else if (self->startX != self->endX &&
			collision(self->x - 4 + self->w / 2, self->y, 8, height, player.x ,player.y, player.w, player.h) == 1)
		{
			self->dirX = 0;

			self->mental = 1;
		}
	}

	else if (self->mental == 2)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->mental = 0;
		}
	}

	checkToMap(self);
}

static void tongueWait()
{
	if (self->head->mental == 1)
	{
		self->health = self->maxHealth;

		self->action = &tongueExtendOut;

		self->touch = &tongueTouch;

		playSoundToMap("sound/boss/armour_boss/tongue_start", -1, self->x, self->y, 0);
	}

	self->x = self->head->x + self->head->w / 2 - self->w / 2;
}

static void tongueTouch(Entity *other)
{
	int y;

	if (self->target == NULL && other->type == PLAYER && other->health > 0)
	{
		self->head->face = other->face;

		self->head->touch = &creeperTouch;

		self->target = other;

		self->target->flags |= FLY;

		self->thinkTime = 180;

		self->action = &moveToMouth;

		y = other->y + other->h - self->h;

		if (y < self->endY)
		{
			self->y = y;
		}
	}

	else
	{
		entityTouch(other);
	}
}

static void moveToMouth()
{
	self->thinkTime--;

	self->y -= self->y - self->startY > 96 ? 1 : self->speed;

	if (self->y <= self->startY)
	{
		if (self->target != NULL)
		{
			self->target->flags &= ~FLY;
		}

		self->target = NULL;

		self->y = self->startY;

		self->action = &moveToMouthFinish;

		self->thinkTime = 180;
	}

	if (self->target != NULL)
	{
		self->target->x = self->x + self->w / 2 - self->target->w / 2;
		self->target->y = self->y + self->h / 2 - self->target->h / 2;

		self->target->dirY = 0;
	}
}

static void moveToMouthFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &tongueWait;
	}
}

static void tongueTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}

		if (self->health > 0)
		{
			setCustomAction(self, &flashWhite, 6, 0, 0);

			/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

			if (other->type != PROJECTILE)
			{
				setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
			}

			if (self->pain != NULL)
			{
				self->pain();
			}
		}

		else
		{
			self->touch = NULL;

			if (self->target != NULL)
			{
				self->target->flags &= ~FLY;
			}

			self->target = NULL;

			self->action = &tongueRetreat;

			self->thinkTime = 180;
		}
	}
}

static void tongueRetreat()
{
	self->y -= self->speed;

	if (self->y <= self->startY)
	{
		self->y = self->startY;

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->head->mental = 0;

			self->action = &tongueWait;
		}
	}
}

static void tongueExtendOut()
{
	self->y += self->speed;

	if (self->y >= self->endY)
	{
		self->y = self->endY;
	}

	self->box.h = self->endY - self->startY;
	self->box.y = -self->box.h;
}

static int drawTongue()
{
	float y;

	y = self->y;

	setEntityAnimation(self, "WALK");

	while (self->y >= self->startY)
	{
		drawSpriteToMap();

		self->y -= self->h;
	}

	setEntityAnimation(self, "STAND");

	self->y = y;

	drawLoopingAnimationToMap();

	return TRUE;
}

static void creeperTouch(Entity *other)
{
	if (self->mental == 1 && other->type == PLAYER)
	{
		other->flags |= NO_DRAW;

		other->fallout();

		self->mental = 2;

		self->thinkTime = 300;

		self->touch = &entityTouch;

		playSoundToMap("sound/enemy/whirlwind/suck", -1, self->x, self->y, 0);

		game.timesEaten++;

		if (game.timesEaten == 5)
		{
			addMedal("eaten_5");
		}
	}

	else
	{
		entityTouch(other);
	}
}

static void creditsAction()
{
	self->dirY -= GRAVITY_SPEED * self->weight;

	checkToMap(self);

	if (self->dirY == 0)
	{
		self->creditsAction = &creditsMove;
	}
}

static void creditsMove()
{
	self->face = RIGHT;

	setEntityAnimation(self, "STAND");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}
