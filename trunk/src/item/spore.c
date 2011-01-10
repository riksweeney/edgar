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

#include "../audio/audio.h"
#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "item.h"
#include "key_items.h"
#include "../hud.h"
#include "../system/error.h"

extern Entity *self, player;
extern Game game;

static void throwSpore(int);

Entity *addSpore(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Spore");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;
	e->touch = &keyItemTouch;
	e->activate = &throwSpore;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void throwSpore(int val)
{
	Entity *e;

	if (self->thinkTime <= 0 && game.status == IN_GAME)
	{
		e = addTemporaryItem("item/spore", player.x + (player.face == RIGHT ? player.w : 0), player.y + player.h / 2, player.face, player.face == LEFT ? -5 : 5, ITEM_JUMP_HEIGHT);

		e->touch = &entityTouch;

		playSoundToMap("sound/common/throw.ogg", EDGAR_CHANNEL, player.x, player.y, 0);

		self->thinkTime = self->maxThinkTime;

		self->health--;

		if (self->health <= 0)
		{
			self->inUse = FALSE;
		}
	}
}
