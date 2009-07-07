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

#include "chicken_feed.h"
#include "chicken_trap.h"
#include "coal_pile.h"
#include "../custom_actions.h"
#include "../inventory.h"
#include "rock_pile.h"
#include "heart_container.h"
#include "shrub.h"
#include "stalactite.h"
#include "bomb.h"
#include "bomb_pile.h"
#include "potions.h"
#include "arrow_target.h"

extern Entity *self, player;

static Constructor items[] = {
{"item/chicken_feed_bag", &addChickenFeedBag},
{"item/chicken_trap", &addChickenTrap},
{"item/coal_pile", &addCoalPile},
{"item/rock_pile", &addRockPile},
{"misc/small_tree", &addShrub},
{"item/heart_container", &addHeartContainer},
{"item/stalactite", &addStalactite},
{"item/bomb", &addBomb},
{"item/bomb_pile", &addBombPile},
{"item/health_potion", &addHealthPotion},
{"item/arrow_target", &addArrowTarget}
};

static int length = sizeof(items) / sizeof(Constructor);

Entity *addKeyItem(char *name, int x, int y)
{
	int i;

	for (i=0;i<length;i++)
	{
		if (strcmpignorecase(items[i].name, name) == 0)
		{
			printf("Adding Key Item %s to %d %d\n", items[i].name, x, y);

			return items[i].construct(x, y, name);
		}
	}

	printf("Could not find key item %s\n", name);

	exit(1);
}

void keyItemTouch(Entity *other)
{
	if (!(self->flags & INVULNERABLE) && other->type == PLAYER)
	{
		addToInventory(self);
	}
}

void keyItemRespawn()
{
	self->x = player.x + (player.w - self->w) / 2;
	self->y = player.y + player.h - self->h;

	self->dirY = ITEM_JUMP_HEIGHT;

	setCustomAction(self, &invulnerable, 180, 0);
}
