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

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../system/error.h"

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
	int i, note;
	Entity *e;

	self->head = getEntityByObjectiveName("BELL_ROPE_HEAD");

	if (self->head == NULL)
	{
		showErrorAndExit("Music Sheet cannot find BELL_ROPE_HEAD");
	}

	for (i=0;i<strlen(self->head->requires);i++)
	{
		e = getFreeEntity();

		if (e == NULL)
		{
			showErrorAndExit("No free slots to add a Music Note");
		}

		loadProperties("item/music_note", e);

		note = (int)self->head->requires[i];

		setEntityAnimationByID(e, note);

		e->x = self->x + e->offsetX * (i + 1);
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
