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
#include "../collisions.h"
#include "../custom_actions.h"
#include "../audio/audio.h"
#include "key_items.h"
#include "item.h"
#include "../system/error.h"
#include "../system/random.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"

extern Entity *self;

static void activate(int);
static void takeDamage(Entity *, int);
static void wait(void);
static void die(void);

Entity *addGlassWall(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Glass Wall");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &wait;
	e->touch = &pushEntity;
	e->activate = &activate;
	e->takeDamage = &takeDamage;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void activate(int val)
{
	if (val == 100)
	{
		self->active = TRUE;
	}
}

static void wait()
{
	if (self->active == TRUE)
	{
		if ((self->thinkTime % 60) == 0)
		{
			self->mental++;

			setEntityAnimation(self, self->mental);

			playSoundToMap("sound/item/crack.ogg", -1, self->x, self->y, 0);
		}

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			playSoundToMap("sound/common/shatter.ogg", -1, self->x, self->y, 0);

			self->action = &die;
		}
	}

	checkToMap(self);
}

static void die()
{
	int i;
	Entity *e;

	fireTrigger(self->objectiveName);

	fireGlobalTrigger(self->objectiveName);

	for (i=0;i<7;i++)
	{
		e = addTemporaryItem("item/glass_wall_piece", self->x, self->y, RIGHT, 0, 0);

		e->x += self->w / 2 - e->w / 2;
		e->y += self->h / 2 - e->h / 2;

		e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimation(e, i);

		e->thinkTime = 60 + (prand() % 60);
	}

	self->inUse = FALSE;
}

static void takeDamage(Entity *other, int damage)
{
	setCustomAction(self, &invulnerableNoFlash, 20, 0);

	playSoundToMap("sound/common/dink.ogg", 2, self->x, self->y, 0);
}
