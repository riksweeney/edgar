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

#include "../audio/audio.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../map.h"
#include "../system/error.h"

extern Entity *self;

static void touch(Entity *);
static void entityWait(void);
static void initialise(void);

Entity *addPhaseDoor(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Phase Door");
	}

	STRNCPY(e->name, name, sizeof(e->name));

	e->flags |= (NO_DRAW|PLAYER_TOUCH_ONLY);

	e->action = &initialise;

	e->touch = &touch;

	e->x = x;
	e->y = y;

	e->draw = &drawPhaseDoorToMap;

	return e;
}

static void touch(Entity *other)
{
	if (self->health == 0)
	{
		other->x = self->endX;

		cameraSnapToTargetEntity();
	}

	else if (self->health == -1)
	{
		other->y = self->endY;

		cameraSnapToTargetEntity();
	}

	else if (self->thinkTime <= 0)
	{
		/*playSoundToMap("sound/item/charge_beep", -1, self->x, self->y, 0);*/

		self->thinkTime = 300;
	}
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}
}

static void initialise()
{
	self->x = self->startX;
	self->y = self->startY;

	self->w = self->health == 0 ? 1 : self->mental;
	self->h = self->health == -1 ? 1 : self->mental;

	self->box.x = 0;
	self->box.y = 0;
	self->box.w = self->w;
	self->box.h = self->h;

	self->action = &entityWait;
}
