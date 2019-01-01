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
#include "../system/error.h"
#include "../system/properties.h"
#include "item.h"
#include "key_items.h"

extern Entity *self, player;
extern Game game;

static void throwMeat(int);
static void meatFallout(void);
static void touch(Entity *);

Entity *addPoisonMeat(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Poison Meat");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;

	e->touch = &keyItemTouch;

	e->fallout = &entityDieNoDrop;

	e->activate = &throwMeat;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void throwMeat(int val)
{
	Entity *e;

	if (self->thinkTime <= 0 && game.status == IN_GAME && player.element != WATER)
	{
		e = addEntity(*self, player.x + (player.face == RIGHT ? player.w : 0), player.y);

		e->flags &= ~ON_GROUND;

		e->health = 5;

		e->dirX = player.face == LEFT ? -6 : 6;

		e->dirY = ITEM_JUMP_HEIGHT;

		e->action = &generalItemAction;

		e->touch = &touch;

		e->thinkTime = 1200;

		e->fallout = &meatFallout;

		e->flags |= DO_NOT_PERSIST;

		setCustomAction(e, &invulnerableNoFlash, 60, 0, 0);

		playSoundToMap("sound/common/throw", -1, player.x, player.y, 0);

		self->health--;

		if (self->health <= 0)
		{
			self->inUse = FALSE;
		}

		else
		{
			self->thinkTime = 120;
		}
	}
}

static void meatFallout()
{
	if (self->environment != WATER)
	{
		entityDieNoDrop();
	}

	self->layer = BACKGROUND_LAYER;

	self->dirX = 0;
}

static void touch(Entity *other)
{

}
