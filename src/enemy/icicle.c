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
#include "../item/item.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self, player;

static void entityWait(void);
static void init(void);
static void fall(void);
static void dieWait(void);

Entity *addIcicle(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Icicle");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	switch (self->mental)
	{
		case 0:
			self->action = &entityWait;
		break;

		case 1:
			self->action = &fall;
		break;

		default:
			self->action = &dieWait;
		break;
	}
}

static void entityWait()
{
	if (player.health > 0)
	{
		/* Must be within a certain range */

		if (collision(self->x - 32, self->y, 64, 320, player.x, player.y, player.w, player.h) == 1)
		{
			self->thinkTime = 30;

			self->mental = 1;

			self->action = &fall;
		}
	}

	checkToMap(self);
}

static void fall()
{
	int i;
	Entity *e;

	self->flags &= ~FLY;

	checkToMap(self);

	if ((self->flags & ON_GROUND) || self->standingOn != NULL)
	{
		self->flags |= NO_DRAW;

		for (i=0;i<8;i++)
		{
			e = addTemporaryItem("misc/icicle_piece", self->x, self->y, RIGHT, 0, 0);

			e->x += self->w / 2 - e->w / 2;
			e->y += self->h / 2 - e->h / 2;

			e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
			e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

			setEntityAnimationByID(e, i);

			e->thinkTime = 60 + (prand() % 60);
		}

		playSoundToMap("sound/enemy/icicle/smash", -1, self->x, self->y, 0);

		self->touch = NULL;

		self->action = &dieWait;

		self->mental = 2;

		self->thinkTime = 300;
	}
}

static void dieWait()
{
	self->flags |= NO_DRAW;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental = 0;

		self->flags |= FLY;

		self->flags &= ~NO_DRAW;

		self->dirX = 0;

		self->dirY = 0;

		self->y = self->startY;

		self->touch = &entityTouch;

		self->action = &entityWait;

		setCustomAction(self, &invulnerable, 180, 0, 0);
	}
}
