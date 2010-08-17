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

#include "../audio/audio.h"
#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../player.h"
#include "../inventory.h"
#include "../world/target.h"
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
	int i, j, size;
	Entity *e;
	Target *t;
	char targetName[MAX_VALUE_LENGTH];
	char *items[] = {
		"item/tuning_fork",
		"item/invisibility_potion",
		"item/repellent",
		"item/water_purifier",
		"item/extend_o_grab",
		"item/bomb",
		"item/sludge_tentacle",
		"item/gazer_eye"
	};
	
	size = sizeof(items) / sizeof(char *);
	
	resetInventoryIndex();
	
	/* Remove all the inventory items */
	
	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		e = removeInventoryItemAtCursor();
		
		if (e != NULL)
		{
			for (j=0;j<size;j++)
			{
				printf("%s == %s\n", e->name, items[j]);
				
				if (strcmpignorecase(e->name, items[j]) == 0)
				{
					printf("Removing %s\n", e->name);
					
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
				STRNCPY(e->name, "item/health_potion_removed", sizeof(e->name));
			}
			
			j = prand() % self->mental;
			
			snprintf(targetName, MAX_VALUE_LENGTH, "REMOVER_TARGET_%d", j);
			
			t = getTargetByName(targetName);
			
			if (t == NULL)
			{
				showErrorAndExit("Item Remover cannot find target");
			}
			
			addEntity(*e, t->x, t->y);
			
			e->inUse = FALSE;
		}
	}
	
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
