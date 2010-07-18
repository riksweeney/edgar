/*
Copyright (C) 2009-2010 Parallel Realities

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
#include "../system/properties.h"
#include "../entity.h"
#include "../event/script.h"
#include "../hud.h"
#include "../graphics/decoration.h"
#include "../collisions.h"
#include "../system/error.h"
#include "../system/random.h"

extern Entity *self, player;

static void entityWait(void);

Entity *addCellBars(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Cell Bar");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &entityWait;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	e->thinkTime = 0;

	return e;
}

static void entityWait()
{
	if (self->active == FALSE)
	{
		self->y -= self->speed;
		
		if (self->y < self->endY)
		{
			self->touch = &pushEntity;
			
			self->y = self->endY;
			
			self->mental = 1;
		}
	}
	
	else if (self->mental == 0)
	{
		player.x = self->x + self->w / 2 - player.w / 2;
	}
}
