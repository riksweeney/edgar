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
#include "../event/trigger.h"
#include "../graphics/animation.h"
#include "../inventory.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "key_items.h"

extern Entity *self, player;
extern Game game;

static void useBottle(int);
static void soulActivate(void);
static void soulTouch(Entity *);
static void bottleFill(void);

Entity *addSoulBottle(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Soul Bottle");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;
	e->touch = &keyItemTouch;
	e->activate = &useBottle;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void useBottle(int val)
{
	Entity *e;

	if (game.status == IN_GAME && player.element != WATER)
	{
		playSoundToMap("sound/common/throw", -1, player.x, player.y, 0);

		e = addEntity(*self, player.x + (player.face == LEFT ? 0 : player.w), player.y);

		e->dirX = player.face == LEFT ? -5 : 5;

		e->flags &= ~ON_GROUND;

		e->dirY = ITEM_JUMP_HEIGHT;

		e->face = player.face;

		e->action = &soulActivate;

		e->touch = &soulTouch;

		e->health = 0;

		e->thinkTime = 300;

		setEntityAnimation(e, "WALK");

		removeInventoryItemByObjectiveName(self->objectiveName);
	}
}

static void soulActivate()
{
	long onGround;
	Entity *e;

	onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (landedOnGround(onGround) == TRUE)
	{
		if (self->health == 0)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add Bottle Magic");
			}

			loadProperties("item/bottle_magic", e);

			setEntityAnimation(e, "STAND");

			e->x = self->x + self->w / 2 - e->w / 2;
			e->y = self->y - e->h;

			e->action = &doNothing;
			e->draw = &drawLoopingAnimationToMap;

			self->target = e;

			self->health = 1;
		}

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			if (self->target != NULL)
			{
				self->target->inUse = FALSE;
			}

			setEntityAnimation(self, "STAND");

			self->touch = &keyItemTouch;
		}

		self->dirX = 0;
	}
}

static void soulTouch(Entity *other)
{
	if (self->health == 1 && other->type == PLAYER)
	{
		if (self->target != NULL)
		{
			self->target->inUse = FALSE;
		}

		keyItemTouch(other);
	}

	else if (self->health == 1 && strcmpignorecase(other->name, "enemy/spirit") == 0)
	{
		self->touch = NULL;

		self->action = &bottleFill;

		self->activate = NULL;

		fireTrigger(other->objectiveName);

		other->target = self;

		other->action = other->die;

		self->mental = 11;

		playSoundToMap("sound/enemy/spirit/spirit_explode", -1, self->x, self->y, 0);

		self->endX = playSoundToMap("sound/enemy/spirit/spirit_scream", -1, self->x, self->y, 0);
	}
}

static void bottleFill()
{
	if (self->mental <= 10)
	{
		if (self->target != NULL)
		{
			self->target->inUse = FALSE;
		}

		stopSound(self->endX);

		playSoundToMap("sound/item/cork", -1, self->x, self->y, 0);

		loadProperties("item/full_soul_bottle", self);

		self->touch = &keyItemTouch;

		self->action = &doNothing;
	}
}
