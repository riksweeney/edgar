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
#include "../item/key_items.h"
#include "../item/item.h"
#include "../system/error.h"

extern Entity *self, player;
extern Game game;

static void dropChickenFeed(int);

Entity *addChickenFeedBag(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add chicken feed bag");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->dirY = ITEM_JUMP_HEIGHT;

	e->type = KEY_ITEM;

	e->face = LEFT;

	e->action = &doNothing;
	e->touch = &keyItemTouch;
	e->activate = &dropChickenFeed;
	e->die = &keyItemRespawn;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void dropChickenFeed(int val)
{
	Entity *e;

	if (self->thinkTime <= 0 && game.status == IN_GAME)
	{
		e = addTemporaryItem("item/chicken_feed", player.x + (player.face == RIGHT ? player.w : 0), player.y + player.h / 2, player.face, player.face == LEFT ? -5 : 5, ITEM_JUMP_HEIGHT);

		e->touch = &entityTouch;

		playSoundToMap("sound/common/throw.ogg", -1, player.x, player.y, 0);

		self->thinkTime = self->maxThinkTime;
	}
}
