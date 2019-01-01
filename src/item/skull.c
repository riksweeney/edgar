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
#include "../game.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void swing(void);
static void entityWait(void);
static int draw(void);
static void activate(int);
static void chainWait(void);
static int drawChain(void);
static void followPlayer(void);
static void dropOnPlayer(void);
static void dropWait(void);
static void init(void);

Entity *addSkull(int x, int y, char *name)
{
	Entity *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Skull");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->touch = &entityTouch;
	e->activate = &activate;
	e->die = &entityDie;
	e->pain = &enemyPain;

	e->draw = &draw;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	Entity *chain;

	self->action = self->mental == 0 ? &swing : &entityWait;

	if (self->mental == 0)
	{
		chain = getFreeEntity();

		if (chain == NULL)
		{
			showErrorAndExit("No free slots to add a Skull Chain");
		}

		loadProperties("item/skull_chain", chain);

		chain->head = self;

		chain->draw = &drawChain;

		chain->action = &chainWait;

		chain->die = &entityDieVanish;
	}
}

static int draw()
{
	drawLoopingAnimationToMap();

	return TRUE;
}

static void chainWait()
{
	if (self->head->health > 0)
	{
		self->x = self->head->endX + self->head->w / 2 - self->w / 2;
		self->y = self->head->startY;
	}

	else
	{
		self->die();
	}
}

static int drawChain()
{
	int i;
	float partDistanceX, partDistanceY;

	partDistanceX = self->head->x + self->head->w / 2 - self->w / 2 - self->x;
	partDistanceY = self->head->y - self->y;

	partDistanceX /= 8;
	partDistanceY /= 8;

	for (i=0;i<8;i++)
	{
		drawSpriteToMap();

		self->x += partDistanceX;
		self->y += partDistanceY;
	}

	return TRUE;
}

static void swing()
{
	self->thinkTime += 2;

	if (self->thinkTime >= 360)
	{
		self->thinkTime = 0;
	}

	self->x = self->endX + sin(DEG_TO_RAD(self->thinkTime)) * 8;
}

static void entityWait()
{
	self->thinkTime += 2;

	if (self->thinkTime >= 360)
	{
		self->thinkTime = 0;
	}

	self->y = self->startY + sin(DEG_TO_RAD(self->thinkTime)) * 16;
}

static void activate(int val)
{
	if (self->mental == 1)
	{
		self->action = &followPlayer;

		self->mental = 2;
	}

	setEntityAnimation(self, val == 1 ? "WALK" : "STAND");
}

static void followPlayer()
{
	self->targetX = player.x - self->w / 2 + player.w / 2;

	/* Position above the player */

	if (abs(self->x - self->targetX) <= abs(self->dirX))
	{
		self->x = self->targetX;

		self->dirX = 0;

		self->thinkTime = 15;

		self->action = &dropOnPlayer;
	}

	else
	{
		self->dirX = self->targetX < self->x ? -player.speed * 3 : player.speed * 3;
	}

	checkToMap(self);
}

static void dropOnPlayer()
{
	int i;
	long onGround;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags &= ~FLY;

		onGround = (self->flags & ON_GROUND);

		checkToMap(self);

		if (onGround == 0 && (self->flags & ON_GROUND))
		{
			playSoundToMap("sound/common/crash", BOSS_CHANNEL, self->x, self->y, 0);

			shakeScreen(LIGHT, 15);

			self->thinkTime = 15;

			self->action = &dropWait;

			for (i=0;i<20;i++)
			{
				addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
			}
		}
	}
}

static void dropWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags |= FLY;

		if (self->y < self->startY)
		{
			self->y = self->startY;

			self->dirY = 0;

			self->action = &followPlayer;
		}

		else
		{
			self->dirY = -4;
		}
	}

	checkToMap(self);
}
