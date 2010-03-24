/*
Copyright (C) 2009-2010 Parallel Realities

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

#include "../system/properties.h"
#include "../graphics/animation.h"
#include "../entity.h"
#include "../player.h"
#include "../game.h"
#include "../inventory.h"
#include "../custom_actions.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../system/error.h"

extern Entity *self, player;
extern Game game;

static void useHealthPotion(int);
static void useSlimePotion(int);
static void useInvisibilityPotion(int);

Entity *addHealthPotion(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Health Potion");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->thinkTime = 0;
	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;
	e->touch = &keyItemTouch;
	e->draw = &drawLoopingAnimationToMap;
	e->activate = &useHealthPotion;

	setEntityAnimation(e, STAND);

	return e;
}

static void useHealthPotion(int val)
{
	if (player.health != player.maxHealth)
	{
		if (game.status == IN_INVENTORY)
		{
			setInventoryDialogMessage(_("Used %s"), _(self->objectiveName));
		}

		player.health = player.maxHealth;

		self->health--;

		if (self->health <= 0)
		{
			self->inUse = FALSE;
		}
	}
}

Entity *addSlimePotion(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Slime Potion");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->thinkTime = 0;
	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;
	e->touch = &keyItemTouch;
	e->draw = &drawLoopingAnimationToMap;
	e->activate = &useSlimePotion;

	setEntityAnimation(e, STAND);

	return e;
}

static void useSlimePotion(int val)
{
	if (game.status == IN_GAME)
	{
		becomeJumpingSlime(self->health);

		if (player.element == WATER)
		{
			loadProperties("item/empty_potion", self);

			self->activate = NULL;

			self->health = 0;
		}
	}
}

Entity *addInvisibilityPotion(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Invisibility Potion");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->thinkTime = 0;
	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;
	e->touch = &keyItemTouch;
	e->draw = &drawLoopingAnimationToMap;
	e->activate = &useInvisibilityPotion;

	setEntityAnimation(e, STAND);

	return e;
}

static void useInvisibilityPotion(int val)
{
	setCustomAction(&player, &invisible, 60 * 5, 0, 0);
}
