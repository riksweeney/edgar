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
#include "../item/item.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void fly(void);
static void gazeInit(void);
static void gaze(void);
static void gazeFinish(void);
static void die(void);
static void creditsMove(void);

Entity *addGazer(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Gazer");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &die;

	e->takeDamage = &entityTakeDamageNoFlinch;

	e->reactToBlock = &changeDirection;

	e->action = &fly;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void die()
{
	Entity *e;

	if (prand() % 3 == 0)
	{
		e = dropCollectableItem("item/gazer_eye", self->x + self->w / 2, self->y, self->face);

		e->x -= e->w / 2;
	}

	playSoundToMap("sound/enemy/gazer/gazer_die", -1, self->x, self->y, 0);

	entityDie();
}

static void fly()
{
	if (self->currentFrame == 3 || self->currentFrame == 7)
	{
		if (self->mental == 0)
		{
			playSoundToMap("sound/enemy/gazer/flap", -1, self->x, self->y, 0);

			self->mental = 1;
		}
	}

	else
	{
		self->mental = 0;
	}

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->dirX = self->face == LEFT ? self->speed : -self->speed;
	}

	if (self->dirX < 0)
	{
		self->face = LEFT;
	}

	else if (self->dirX > 0)
	{
		self->face = RIGHT;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;

		if (player.health > 0 && prand() % 60 == 0 && !(player.flags & HELPLESS))
		{
			if (collision(self->x + (self->face == RIGHT ? self->w : -160), self->y, 160, self->h, player.x, player.y, player.w, player.h) == 1)
			{
				playSoundToMap("sound/enemy/gazer/growl", -1, self->x, self->y, 0);

				self->action = &gazeInit;

				self->thinkTime = 30;

				self->dirX = 0;
			}
		}
	}
}

static void gazeInit()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, "ATTACK_1");

		self->action = &gaze;
	}
}

static void gaze()
{
	playSoundToMap("sound/enemy/gazer/flash", -1, self->x, self->y, 0);

	fadeFromColour(255, 255, 255, 60);

	if ((player.x < self->x && player.face == RIGHT) || (player.x > self->x && player.face == LEFT))
	{
		if (collision(self->x + (self->face == RIGHT ? self->w : -160), self->y, 160, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			setPlayerStunned(120);

			self->thinkTime = 30;
		}
	}

	else
	{
		self->thinkTime = 90;
	}

	self->action = &gazeFinish;
}

static void gazeFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirX = self->face == RIGHT ? self->speed : -self->speed;

		setEntityAnimation(self, "STAND");

		self->action = &fly;

		/* Gaze recharge time is about 3 seconds */

		self->thinkTime = 180;
	}
}

static void creditsMove()
{
	int currentFrame;
	float frameTimer;

	self->dirX = self->speed;

	self->thinkTime++;

	if (self->thinkTime >= 180)
	{
		if (self->thinkTime == 180)
		{
			playSoundToMap("sound/enemy/gazer/growl", -1, self->x, self->y, 0);
		}

		self->dirX = 0;

		if (self->thinkTime >= 240)
		{
			currentFrame = self->currentFrame;

			frameTimer = self->frameTimer;

			setEntityAnimation(self, "ATTACK_1");

			self->currentFrame = currentFrame;

			self->frameTimer = frameTimer;

			if (self->thinkTime >= 300)
			{
				playSoundToMap("sound/enemy/gazer/flash", -1, self->x, self->y, 0);

				fadeFromColour(255, 255, 255, 60);

				self->inUse = FALSE;
			}
		}

		else
		{
			setEntityAnimation(self, "STAND");
		}
	}

	else
	{
		setEntityAnimation(self, "STAND");
	}

	checkToMap(self);
}
