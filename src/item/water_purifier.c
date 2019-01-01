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
#include "../item/key_items.h"
#include "../map.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self, player;
extern Game game;

static void dropPurifierCapsule(int);
static void capsuleFallout(void);
static void purifyWater(void);
static void fill(int, int);

Entity *addWaterPurifier(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Water Purifier");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &doNothing;
	e->touch = &keyItemTouch;
	e->activate = &dropPurifierCapsule;
	e->die = &keyItemRespawn;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void dropPurifierCapsule(int val)
{
	Entity *e;

	if (self->thinkTime <= 0 && game.status == IN_GAME && player.element != WATER)
	{
		e = addTemporaryItem("item/purifier_capsule", player.x + (player.face == RIGHT ? player.w : 0), player.y + player.h / 2, player.face, player.face == LEFT ? -5 : 5, ITEM_JUMP_HEIGHT);

		e->touch = &entityTouch;

		e->fallout = &capsuleFallout;

		playSoundToMap("sound/common/throw", EDGAR_CHANNEL, player.x, player.y, 0);

		self->thinkTime = self->maxThinkTime;
	}
}

static void capsuleFallout()
{
	if (self->environment == SLIME)
	{
		self->action = &purifyWater;

		self->thinkTime = 0;

		self->flags |= NO_DRAW;
	}
}

static void purifyWater()
{
	fill(self->x / TILE_SIZE, self->y / TILE_SIZE);

	resetBlendTime();

	self->inUse = FALSE;
}

static void fill(int x, int y)
{
	if (mapTileAt(x, y) == SLIME_TILE_START)
	{
		setTileAt(x, y, SLIME_TILE_BLEND);

		fill(x - 1, y);
		fill(x + 1, y);
		fill(x, y - 1);
		fill(x, y + 1);
	}
}
