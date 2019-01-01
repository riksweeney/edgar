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
#include "../item/key_items.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void entityWait(void);
static void touch(Entity *);
static void activate(int);

Entity *addSafeCombination(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Safe Combination");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &entityWait;

	e->draw = &drawLoopingAnimationToMap;

	e->touch = &touch;

	e->activate = &activate;

	e->active = FALSE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	checkToMap(self);
}

static void touch(Entity *other)
{
	int i, unit, dir;
	char combination[MAX_VALUE_LENGTH];

	if (other->type == PLAYER)
	{
		if (strlen(self->requires) == 0)
		{
			dir = prand() % 2;

			for (i=0;i<3;i++)
			{
				unit = 1 + prand() % 20;

				snprintf(combination, sizeof(combination), "%s%d%s", self->requires, unit, dir == 0 ? "L" : "R");

				STRNCPY(self->requires, combination, sizeof(self->requires));

				dir = dir == 1 ? 0 : 1;
			}

			self->activate(1);
		}
	}

	keyItemTouch(other);
}

static void activate(int val)
{
	char combination[MAX_VALUE_LENGTH], turns[5];
	int i, j, length;

	j = 0;

	combination[0] = '\0';

	length = strlen(self->requires);

	for (i=0;i<length;i++)
	{
		if (isdigit(self->requires[i]))
		{
			turns[j++] = self->requires[i];

			turns[j] = '\0';
		}

		else
		{
			val = atoi(turns);

			if (strlen(combination) == 0)
			{
				snprintf(self->description, sizeof(self->description), "%d %s", val, self->requires[i] == 'L' ? _("Left") : _("Right"));
			}

			else
			{
				snprintf(self->description, sizeof(self->description), ", %d %s", val, self->requires[i] == 'L' ? _("Left") : _("Right"));
			}

			strncat(combination, self->description, MAX_MESSAGE_LENGTH - strlen(combination) - 1);

			j = 0;

			turns[j] = '\0';
		}
	}

	STRNCPY(self->description, combination, sizeof(self->description));

	snprintf(self->description, sizeof(self->description), _("A scrap of paper. \"%s\" is written on it"), combination);
}
