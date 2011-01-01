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

#include "../graphics/decoration.h"
#include "../system/random.h"
#include "../item/item.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../audio/audio.h"
#include "../system/error.h"

extern Entity *self;

static int drawGib(void);

EntityList *throwGibs(char *name, int gibs)
{
	int i;
	Entity *e;
	EntityList *list;

	list = (EntityList *)malloc(sizeof(EntityList));

	if (list == NULL)
	{
		showErrorAndExit("Failed to allocate a whole %d bytes for Entity List", (int)sizeof(EntityList));
	}

	list->next = NULL;

	for (i=0;i<gibs;i++)
	{
		e = addTemporaryItem(name, self->x, self->y, RIGHT, 0, 0);

		e->x += (self->w - e->w) / 2;
		e->y += (self->h - e->h) / 2;

		e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = -12 - (prand() % 4);

		setEntityAnimation(e, i);

		e->thinkTime = 180 + (prand() % 120);

		e->draw = &drawGib;
		
		addEntityToList(list, e);
	}

	playSoundToMap("sound/common/gib.ogg", -1, self->x, self->y, 0);

	self->inUse = FALSE;
	
	return list;
}

static int drawGib()
{
	if (drawLoopingAnimationToMap() == TRUE)
	{
		if (!(self->flags & ON_GROUND))
		{
			if (prand() % 6 == 0)
			{
				addBlood(self->x + self->w / 2, self->y + self->h / 2);
			}
		}

		else
		{
			self->frameSpeed = 0;
		}
	}

	return TRUE;
}
