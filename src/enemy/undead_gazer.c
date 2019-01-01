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
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void fly(void);
static void gazeInit(void);
static void gaze(void);
static void die(void);
static void gazeFinish(void);
static void addDeathTimer(void);
static void timerWait(void);
static void creditsMove(void);

Entity *addUndeadGazer(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Undead Gazer");
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

		if (self->endX != -1 && player.health > 0 && prand() % 60 == 0 && !(player.flags & HELPLESS))
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

static void die()
{
	playSoundToMap("sound/enemy/gazer/gazer_die", -1, self->x, self->y, 0);

	entityDie();
}

static void gazeInit()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &gaze;
	}
}

static void gaze()
{
	playSoundToMap("sound/enemy/gazer/flash", -1, self->x, self->y, 0);

	fadeFromColour(255, 0, 0, 60);

	addDeathTimer();

	self->endX = -1;

	self->thinkTime = 30;

	self->action = &gazeFinish;
}

static void addDeathTimer()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Death Timer");
	}

	loadProperties("enemy/death_timer", e);

	e->x = player.x;
	e->y = player.y;

	e->draw = &drawLoopingAnimationToMap;

	e->action = &timerWait;

	e->thinkTime = 16 * 60;

	e->type = ENEMY;

	e->flags |= DO_NOT_PERSIST;

	e->head = self;

	e->target = &player;

	setEntityAnimation(e, "STAND");
}

static void timerWait()
{
	Entity *temp;

	if (self->head->health <= 0 || self->target->health <= 0)
	{
		self->inUse = FALSE;
	}

	else
	{
		self->x = self->target->x + self->target->w / 2 - self->w / 2;
		self->startY = self->target->y - self->h - 8;

		self->startX++;

		if (self->startX >= 360)
		{
			self->startX = 0;
		}

		self->y = self->startY + sin(DEG_TO_RAD(self->startX)) * 8;

		if (self->thinkTime > 0)
		{
			self->thinkTime--;

			freeMessageQueue();

			setInfoBoxMessage(5, 255, 255, 255, "%d", self->thinkTime / 60);
		}

		else
		{
			temp = self;

			self = self->target;

			self->takeDamage(temp, 1);

			self = temp;
		}
	}
}

static void gazeFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirX = self->face == RIGHT ? self->speed : -self->speed;

		self->action = &fly;

		/* Gaze recharge time is about 3 seconds */

		self->thinkTime = self->endX == -1 ? 60 * 16 : 180;

		self->endX = 0;
	}
}

static void creditsMove()
{
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
			if (self->thinkTime >= 300)
			{
				playSoundToMap("sound/enemy/gazer/flash", -1, self->x, self->y, 0);

				fadeFromColour(255, 0, 0, 60);

				self->inUse = FALSE;
			}
		}
	}

	checkToMap(self);
}
