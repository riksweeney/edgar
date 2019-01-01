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

#include "../collisions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../inventory.h"
#include "../map.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../world/save_point.h"
#include "key_items.h"

extern Entity *self, player;
extern Game game;

static void createSave(int);
static void savePointWait(void);

Entity *addPortableSave(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Portable Save");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;

	e->touch = &keyItemTouch;

	e->fallout = &keyItemFallout;

	e->activate = &createSave;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "LITTLE");

	return e;
}

static void createSave(int val)
{
	Entity *e;

	if (game.status == IN_GAME && player.element != WATER)
	{
		player.inUse = FALSE;

		if (game.gameType != NORMAL || strcmpignorecase(getMapFilename(), self->requires) != 0)
		{
			setInfoBoxMessage(120, 255, 255, 255, _("You cannot use this here"));
		}

		else if (player.standingOn != NULL || isSpaceEmpty(&player) != NULL)
		{
			setInfoBoxMessage(120, 255, 255, 255, _("You can only use this item in an empty space"));
		}

		else
		{
			e = addEntity(*self, player.x, player.y);

			e->touch = &entityTouch;

			e->thinkTime = 180;

			e->x += (player.w - e->w) / 2;

			e->mental = 0;

			e->action = &savePointWait;

			removeInventoryItemByObjectiveName(self->objectiveName);
		}

		player.inUse = TRUE;
	}
}

static void savePointWait()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime % 15 == 0)
	{
		setEntityAnimation(self, self->mental == 0 ? "BIG" : "LITTLE");

		self->mental = self->mental == 0 ? 1 : 0;
	}

	if (self->thinkTime <= 0)
	{
		e = addSavePoint(self->x, self->y);

		e->y = self->y + self->h;

		e->y -= e->h;

		self->inUse = FALSE;
	}

	checkToMap(self);
}
