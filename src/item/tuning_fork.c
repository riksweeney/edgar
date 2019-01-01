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
#include "../item/key_items.h"
#include "../player.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Game game;
extern Entity *self;

static void activate(int);

Entity *addTuningFork(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Tuning Fork");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = LEFT;

	e->action = &doNothing;
	e->touch = &keyItemTouch;
	e->activate = &activate;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void activate(int val)
{
	EntityList *el, *entities;
	Entity *temp;

	entities = getEntities();

	if (game.status == IN_GAME)
	{
		playSoundToMap("sound/item/tuning_fork", EDGAR_CHANNEL, self->x, self->y, 0);

		for (el=entities->next;el!=NULL;el=el->next)
		{
			if (el->entity->inUse == TRUE)
			{
				if ((strcmpignorecase(el->entity->requires, "TUNING_FORK") == 0 && getDistanceFromPlayer(el->entity) < 128)
					|| (strcmpignorecase(el->entity->requires, "BOSS_TUNING_FORK") == 0))
				{
					temp = self;

					self = el->entity;

					self->activate(100);

					self = temp;
				}
			}
		}
	}
}
