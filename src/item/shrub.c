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
#include "../system/properties.h"
#include "../entity.h"
#include "../audio/audio.h"
#include "../custom_actions.h"
#include "item.h"
#include "../system/random.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"
#include "../system/error.h"

extern Entity *self;

static void touch(Entity *);
static void die(void);

Entity *addShrub(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Shrub");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &doNothing;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->die = &die;

	e->type = KEY_ITEM;

	setEntityAnimation(e, STAND);

	return e;
}

static void touch(Entity *other)
{
	pushEntity(other);

	if ((other->flags & ATTACKING) && !(self->flags & INVULNERABLE))
	{
		if (strcmpignorecase(other->name, self->requires) == 0)
		{
			self->health--;

			setCustomAction(self, &flashWhite, 6, 0);
			setCustomAction(self, &invulnerableNoFlash, 20, 0);

			if (self->health <= 0)
			{
				self->action = &die;
			}
		}
	}
}

static void die()
{
	int i;

	Entity *e = addPermanentItem("misc/chopped_log", self->x, self->y);

	e->x += (self->w - e->w) / 2;
	e->y += (self->h - e->h) / 2;

	e->dirY = ITEM_JUMP_HEIGHT;

	fireTrigger(self->objectiveName);

	fireGlobalTrigger(self->objectiveName);

	for (i=0;i<8;i++)
	{
		e = addTemporaryItem("misc/small_tree_piece", self->x, self->y, RIGHT, 0, 0);

		e->x += self->w / 2 - e->w / 2;
		e->y += self->h / 2 - e->h / 2;

		e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimation(e, i);

		e->thinkTime = 60 + (prand() % 60);
	}

	self->inUse = FALSE;
}
