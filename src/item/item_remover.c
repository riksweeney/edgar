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
#include "../entity.h"
#include "../player.h"
#include "../inventory.h"
#include "../world/target.h"
#include "../event/global_trigger.h"
#include "../event/objective.h"
#include "../system/error.h"
#include "../system/random.h"

extern Entity *self;

static void removeItems(void);
static void entityWait(void);

Entity *addItemRemover(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Item Remover");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &entityWait;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void entityWait()
{
	if (self->active == TRUE)
	{
		self->action = &removeItems;
	}
}

static void removeItems()
{
	int i, j, size, count, x, allTargets;
	Entity *e;
	Target *t;
	char targetName[MAX_LINE_LENGTH];
	char *items[] = {
		"item/tuning_fork",
		"item/invisibility_potion",
		"item/repellent",
		"item/water_purifier",
		"item/extend_o_grab",
		"item/bomb",
		"item/sludge_tentacle",
		"item/gazer_eye",
		"weapon/normal_arrow"
	};

	size = sizeof(items) / sizeof(char *);

	resetInventoryIndex();

	count = 0;

	allTargets = 0;

	/* Remove all the inventory items */

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		e = removeInventoryItemAtCursor();

		if (e != NULL)
		{
			for (j=0;j<size;j++)
			{
				if (strcmpignorecase(e->name, items[j]) == 0)
				{
					e->inUse = FALSE;

					break;
				}
			}

			if (e->inUse == FALSE)
			{
				continue;
			}

			if (strcmpignorecase(e->name, "item/health_potion") == 0)
			{
				e->mental = -1;
			}

			/* Ensure that at least 1 item appears at each target */

			j = allTargets < self->mental ? allTargets : prand() % self->mental;

			allTargets++;

			snprintf(targetName, MAX_VALUE_LENGTH, "REMOVER_TARGET_%d", j);

			t = getTargetByName(targetName);

			if (t == NULL)
			{
				showErrorAndExit("Item Remover cannot find target");
			}

			x = t->x;

			x += (prand() % 32) * (prand() % 2 == 0 ? -1 : 1);

			addEntity(*e, x, t->y);

			e->inUse = FALSE;

			snprintf(targetName, MAX_LINE_LENGTH, "\"%s\" 1 UPDATE_TRIGGER \"ITEMS\"", e->objectiveName);

			count++;

			addGlobalTriggerFromScript(targetName);
		}
	}

	freeInventory();

	snprintf(targetName, MAX_LINE_LENGTH, "\"ITEMS\" %d UPDATE_OBJECTIVE \"Retrieve items\"", count);

	addGlobalTriggerFromScript(targetName);

	e = removePlayerWeapon();

	if (e != NULL)
	{
		e->inUse = FALSE;
	}

	e = removePlayerShield();

	if (e != NULL)
	{
		e->inUse = FALSE;
	}

	self->inUse = FALSE;
}
