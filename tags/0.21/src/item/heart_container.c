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
#include "key_items.h"
#include "item.h"
#include "../player.h"
#include "../system/random.h"
#include "../graphics/decoration.h"

extern Entity *self;

static void touch(Entity *);
static void draw(void);

Entity *addHeartContainer(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a heart container\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;
	e->touch = &touch;

	e->draw = &draw;

	setEntityAnimation(e, STAND);

	e->thinkTime = 0;

	return e;
}

static void touch(Entity *other)
{
	if (other->type == PLAYER)
	{
		increasePlayerMaxHealth();
		
		self->inUse = FALSE;
	}
}

static void draw()
{
	Entity *e;
	
	if (prand() % 10 == 0)
	{
		e = addBasicDecoration(self->x + prand() % self->w, self->y + prand() % 25, "decoration/blue_particle");
		
		if (e != NULL)
		{
			e->thinkTime = 60 + prand() % 30;
			
			e->dirY = -0.3;
		}
	}
	
	drawLoopingAnimationToMap();
}
