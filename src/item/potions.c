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

#include "../system/properties.h"
#include "../graphics/animation.h"
#include "../entity.h"
#include "../player.h"
#include "../game.h"
#include "../inventory.h"
#include "../item/item.h"
#include "../item/key_items.h"

extern Entity *self, player;
extern Game game;

static void useHealthPotion(int);

Entity *addHealthPotion(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Health Potion\n");

		exit(1);
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
			setInventoryDialogMessage(_("Used %s"), self->objectiveName);
		}

		player.health = player.maxHealth;

		self->health--;

		if (self->health <= 0)
		{
			self->inUse = FALSE;
		}
	}
}
