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

#include "../custom_actions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../inventory.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self, player;
extern Game game;

static void useHealthPotion(int);
static void useSlimePotion(int);
static void useInvisibilityPotion(int);
static void useFlamingArrowPotion(int);

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

	setEntityAnimation(e, "STAND");

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

		player.health += 5;

		if (player.health > player.maxHealth)
		{
			player.health = player.maxHealth;
		}

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

	setEntityAnimation(e, "STAND");

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

	setEntityAnimation(e, "STAND");

	return e;
}

static void useInvisibilityPotion(int val)
{
	if (game.status == IN_GAME && player.element != WATER)
	{
		setCustomAction(&player, &invisible, 60 * 5, 0, 0);
	}
}

Entity *addFlamingArrowPotion(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Flaming Arrow Potion");
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
	e->activate = &useFlamingArrowPotion;

	setEntityAnimation(e, "STAND");

	return e;
}

static void useFlamingArrowPotion(int val)
{
	int addNew, maxArrows;
	Entity *arrow, *fireArrow;

	arrow = getInventoryItemByName("weapon/normal_arrow");

	if (arrow == NULL)
	{
		if (game.status == IN_INVENTORY)
		{
			setInventoryDialogMessage(_("Normal Arrow is required"));
		}

		else
		{
			setInfoBoxMessage(60, 255, 255, 255, _("Normal Arrow is required"));
		}

		return;
	}

	addNew = FALSE;

	fireArrow = getInventoryItemByName("weapon/flaming_arrow");

	if (fireArrow == NULL)
	{
		addNew = TRUE;

		fireArrow = addPermanentItem("weapon/flaming_arrow", 0, 0);

		fireArrow->health = 0;
	}

	maxArrows = MAX_STACKABLES - fireArrow->health;

	maxArrows = arrow->health > maxArrows ? maxArrows : arrow->health;

	fireArrow->health += maxArrows;

	arrow->health -= maxArrows;

	if (game.status == IN_INVENTORY)
	{
		setInventoryDialogMessage(_("Created %d Flaming Arrows"), maxArrows);
	}

	else
	{
		setInfoBoxMessage(60, 255, 255, 255, _("Created %d Flaming Arrows"), maxArrows);
	}

	if (arrow->health <= 0)
	{
		removeInventoryItemByName("weapon/normal_arrow");
	}

	if (addNew == TRUE)
	{
		addToInventory(fireArrow);
	}
}
