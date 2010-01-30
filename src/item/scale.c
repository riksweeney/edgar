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
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/error.h"
#include "../hud.h"

extern Entity *self;

static void init(void);
static void wait(void);
static void touch(Entity *);

Entity *addScale(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Scale");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;
	e->touch = &touch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	e->thinkTime = 0;

	return e;
}

static void init()
{
	EntityList *list = getEntitiesByObjectiveName(self->objectiveName);
	EntityList *l;
	Entity *e;
	int i;

	i = 0;

	for (l=list->next;l!=NULL;l=l->next)
	{
		e = l->entity;

		if (e != self)
		{
			self->target = e;

			break;
		}
	}

	freeEntityList(list);

	self->targetY = self->endY - self->startY;

	self->action = &wait;
}

static void wait()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		self->y += self->speed;

		self->target->y -= self->speed;

		if (self->y >= self->endY)
		{
			self->y = self->endY;

			self->target->y = self->startY;
		}
	}

	else
	{
		self->y -= 2;

		if (self->y <= self->targetY)
		{
			self->y = self->targetY;

			self->target->y = self->targetY;
		}
	}
}

static void touch(Entity *other)
{
	pushEntity(other);

	if (other->standingOn != NULL && other->standingOn->type == PLAYER)
	{
		self->thinkTime = 3;
	}
}
