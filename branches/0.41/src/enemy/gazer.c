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
#include "../game.h"
#include "../item/item.h"
#include "../system/error.h"

extern Entity *self, player;

static void fly(void);
static void gazeInit(void);
static void gaze(void);
static void gazeFinish(void);
static void die(void);

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

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

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

	entityDie();
}

static void fly()
{
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
				playSoundToMap("sound/enemy/gazer/growl.ogg", -1, self->x, self->y, 0);

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
		setEntityAnimation(self, ATTACK_1);

		self->action = &gaze;
	}
}

static void gaze()
{
	playSoundToMap("sound/enemy/gazer/flash.ogg", -1, self->x, self->y, 0);

	fadeFromWhite();

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

		setEntityAnimation(self, STAND);

		self->action = &fly;

		/* Gaze recharge time is about 3 seconds */

		self->thinkTime = 180;
	}
}
