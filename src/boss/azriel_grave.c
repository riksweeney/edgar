/*
Copyright (C) 2009-2012 Parallel Realities

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
#include "../audio/music.h"
#include "../boss/sorceror.h"
#include "../collisions.h"
#include "../credits.h"
#include "../custom_actions.h"
#include "../dialog.h"
#include "../enemy/enemies.h"
#include "../enemy/rock.h"
#include "../entity.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../graphics/graphics.h"
#include "../hud.h"
#include "../inventory.h"
#include "../item/key_items.h"
#include "../item/item.h"
#include "../map.h"
#include "../player.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../weather.h"
#include "../world/target.h"

extern Entity *self;

static void entityWait(void);

Entity *addAzrielGrave(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Azriel's Grave");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &entityWait;

	e->draw = &drawLoopingAnimationToMap;

	e->type = KEY_ITEM;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	int i;
	Entity *e;
	
	if (self->mental == 1)
	{
		for (i=0;i<4;i++)
		{
			e = addTemporaryItem("boss/azriel_grave_piece", self->x, self->y, RIGHT, 0, 0);

			e->x += self->w / 2 - e->w / 2;
			e->y += self->h / 2 - e->h / 2;

			e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
			e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

			setEntityAnimationByID(e, i);

			e->thinkTime = 60 + (prand() % 60);
		}
		
		playSoundToMap("sound/common/crumble.ogg", -1, self->x, self->y, 0);

		self->inUse = FALSE;
	}
	
	checkToMap(self);
}
