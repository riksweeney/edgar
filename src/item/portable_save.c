/*
Copyright (C) 2009-2011 Parallel Realities

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
#include "key_items.h"
#include "../custom_actions.h"
#include "../collisions.h"
#include "../system/error.h"
#include "../hud.h"
#include "../map.h"
#include "../world/save_point.h"
#include "../inventory.h"

extern Entity *self, player;
extern Game game;

static void createSave(int);

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

	setEntityAnimation(e, "STAND");

	return e;
}

static void createSave(int val)
{
	Entity *e;

	if (game.status == IN_GAME)
	{
		if (strcmpignorecase(getMapFilename(), self->requires) != 0)
		{
			setInfoBoxMessage(120, 255, 255, 255, _("You cannot use this item here"));
		}
		
		else
		{
			e = addSavePoint(player.x, player.y);
			
			e->x += (player.w - e->w) / 2;
			
			removeInventoryItemByObjectiveName(self->objectiveName);
		}
	}
}
