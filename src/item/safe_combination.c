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

#include "../audio/audio.h"
#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../collisions.h"
#include "../entity.h"
#include "../player.h"
#include "../inventory.h"
#include "../world/target.h"
#include "../event/global_trigger.h"
#include "../event/objective.h"
#include "../system/error.h"
#include "../system/random.h"
#include "../hud.h"
#include "../item/key_items.h"
#include "../event/script.h"

extern Entity *self;

static void entityWait(void);
static void touch(Entity *);

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
			self->description[0] = '\0';
			
			dir = prand() % 2;

			for (i=0;i<3;i++)
			{
				unit = 1 + prand() % 20;

				snprintf(combination, sizeof(combination), "%s%d%s", self->requires, unit, dir == -1 ? "L" : "R");

				STRNCPY(self->requires, combination, sizeof(self->requires));
				
				if (i == 0)
				{
					snprintf(combination, sizeof(combination), "%d %s", unit, dir == -1 ? _("Left") : _("Right"));
				}
				
				else
				{
					snprintf(combination, sizeof(combination), "%s, %d %s", self->description, unit, dir == -1 ? _("Left") : _("Right"));
				}

				STRNCPY(self->description, combination, sizeof(self->description));
				
				dir = dir == -1 ? 0 : -1;
			}

			snprintf(self->description, sizeof(self->description), "A scrap of paper. %s is written on it", combination);
		}
	}
	
	keyItemTouch(other);
}
