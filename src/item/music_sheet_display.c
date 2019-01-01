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

#include "../collisions.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void init(void);
static void sheetWait(void);
static void noteWait(void);

Entity *addMusicSheetDisplay(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Music Sheet Display");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	int i, note, length;
	Entity *e;

	self->head = getEntityByObjectiveName("BELL_ROPE_HEAD");

	if (self->head == NULL)
	{
		showErrorAndExit("Music Sheet cannot find BELL_ROPE_HEAD");
	}

	length = strlen(self->head->requires);

	for (i=0;i<length;i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Music Note");
		}

		loadProperties("item/music_note", e);

		note = self->head->requires[i] - '0';

		setEntityAnimationByID(e, note);

		e->x = self->x + self->offsetX + e->offsetX * (i + 1);
		e->y = self->y + e->offsetY;

		e->type = KEY_ITEM;

		e->face = RIGHT;

		e->action = &noteWait;

		e->draw = &drawLoopingAnimationToMap;

		e->head = self;
	}

	self->action = &sheetWait;
}

static void sheetWait()
{
	if (self->mental == 1)
	{
		self->inUse = FALSE;
	}
}

static void noteWait()
{
	if (self->head->inUse == FALSE)
	{
		self->inUse = FALSE;
	}
}
