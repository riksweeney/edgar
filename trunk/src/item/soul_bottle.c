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
#include "key_items.h"
#include "../inventory.h"
#include "../hud.h"
#include "../system/error.h"

extern Entity *self, player;
extern Game game;

static void useBottle(int);
static void soulActivate(void);
static void soulTouch(Entity *);

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

	setEntityAnimation(e, STAND);

	return e;
}

static void useBottle(int val)
{
	Entity *e;

	if (game.status == IN_GAME)
	{
		e = addEntity(*self, player.x + (player.face == LEFT ? 0 : player.w), self->y);

		e->dirX = player.face == LEFT ? -5 : 5;

		e->dirY = ITEM_JUMP_HEIGHT;

		e->face = player.face;

		e->action = &soulActivate;

		e->touch = &soulTouch;

		e->thinkTime = 300;

		removeInventoryItem(self->objectiveName);
	}
}

static void soulActivate()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &doNothing;

		self->touch = &keyItemTouch;
	}
}

static void soulTouch(Entity *other)
{
	if (strcmpignorecase(other->name, "enemy/spirit") == 0)
	{
		loadProperties("full_soul_bottle", self);

		self->touch = &keyItemTouch;

		self->activate = NULL;
	}
}
