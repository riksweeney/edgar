/*
Copyright (C) 2009-2019 Parallel Realities

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

#include "../entity.h"
#include "../event/script.h"
#include "../event/trigger.h"
#include "../graphics/animation.h"
#include "../system/error.h"

extern Entity *self;

static void standardTouch(Entity *);
static void scriptTouch(Entity *);
static void entityWait(void);
static void initialise(void);

Entity *addLineDef(char *type, char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Line def");
	}

	STRNCPY(e->name, name, sizeof(e->name));

	e->flags |= (NO_DRAW|PLAYER_TOUCH_ONLY);

	e->action = &initialise;

	if (strcmpignorecase(type, "SCRIPT_LINE_DEF") == 0)
	{
		e->type = SCRIPT_LINE_DEF;

		e->touch = &scriptTouch;
	}

	else
	{
		e->type = LINE_DEF;

		e->touch = &standardTouch;
	}

	e->x = x;
	e->y = y;

	e->draw = &drawLineDefToMap;

	return e;
}

static void standardTouch(Entity *other)
{
	if (other->type == PLAYER && self->active == TRUE)
	{
		if (self->mental == -1)
		{
			activateEntitiesWithObjectiveName(self->objectiveName, (self->health >= 0 ? TRUE : FALSE));
		}

		else
		{
			activateEntitiesWithRequiredName(self->objectiveName, (self->health >= 0 ? TRUE : FALSE));
		}

		fireTrigger(self->objectiveName);

		self->inUse = FALSE;
	}
}

static void scriptTouch(Entity *other)
{
	if (other->type == PLAYER && self->active == TRUE)
	{
		runScript(self->objectiveName);

		self->inUse = FALSE;
	}
}

static void entityWait()
{

}

static void initialise()
{
	self->x = self->startX;
	self->y = self->startY;

	self->w = abs(self->startX - self->endX) + 1;
	self->h = abs(self->startY - self->endY) + 1;

	self->box.x = 0;
	self->box.y = 0;
	self->box.w = self->w;
	self->box.h = self->h;

	self->action = &entityWait;
}
