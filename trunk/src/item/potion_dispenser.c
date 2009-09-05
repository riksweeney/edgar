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
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../hud.h"
#include "../inventory.h"
#include "../collisions.h"
#include "key_items.h"

extern Entity *self;

static void activate(int);
static void wait(void);
static void touch(Entity *);

Entity *addPotionDispenser(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Potion Dispenser\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &wait;
	e->activate = &activate;
	e->touch = &touch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	e->thinkTime = 0;

	return e;
}

static void wait()
{
	checkToMap(self);
}

static void touch(Entity *other)
{
	if (other->type == PLAYER)
	{
		setInfoBoxMessage(0,  _("Press Action to interact"));
	}
}

static void activate(int val)
{
	Entity *e;
	
	if (removeInventoryItem(self->requires) == TRUE)
	{
		e = addKeyItem(self->objectiveName, 0, 0);

		addToInventory(e);
	}

	else
	{
		setInfoBoxMessage(60,  _("%s is required"), self->requires);
	}
}
