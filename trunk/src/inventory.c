#include "headers.h"

#include "animation.h"
#include "player.h"
#include "item.h"
#include "hud.h"
#include "trigger.h"
#include "properties.h"

static Inventory inventory;
extern Entity *self;
extern Entity playerWeapon, playerShield;

static void sortInventory(void);

int addToInventory(Entity *e)
{
	int i;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == FALSE)
		{
			inventory.item[i] = *e;

			inventory.item[i].face = RIGHT;

			inventory.item[i].thinkTime = 0;

			setEntityAnimation(&inventory.item[i], STAND);

			e->inUse = FALSE;

			if (inventory.item[i].type == WEAPON)
			{
				autoSetPlayerWeapon(&inventory.item[i]);
			}

			else if (inventory.item[i].type == SHIELD)
			{
				autoSetPlayerShield(&inventory.item[i]);
			}

			addHudMessage(STANDARD_MESSAGE, "Picked up %s", inventory.item[i].objectiveName);
			
			fireTrigger(inventory.item[i].objectiveName);

			return TRUE;
		}
	}

	return FALSE;
}

void selectNextInventoryItem(int index)
{
	int i = 0;
	int itemCount = 0;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == TRUE)
		{
			itemCount++;
		}
	}

	if (itemCount == 0)
	{
		inventory.selectedIndex = 0;

		return;
	}

	i = inventory.selectedIndex;

	do
	{
		i += index;

		if (i == -1)
		{
			i = MAX_INVENTORY_ITEMS - 1;
		}

		else if (i == MAX_INVENTORY_ITEMS)
		{
			i = 0;
		}
	}

	while (inventory.item[i].inUse == FALSE);

	inventory.selectedIndex = i;
}

Entity *getInventoryItem(char *name)
{
	int i;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == TRUE && strcmpignorecase(inventory.item[i].objectiveName, name) == 0)
		{
			return &inventory.item[i];
		}
	}

	return NULL;
}

int removeInventoryItem(char *name)
{
	int i, found;

	found = FALSE;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == TRUE && strcmpignorecase(inventory.item[i].objectiveName, name) == 0)
		{
			inventory.item[i].inUse = FALSE;

			found = TRUE;

			break;
		}
	}

	if (found == TRUE)
	{
		sortInventory();
	}

	return found;
}

void dropInventoryItem()
{
	if (inventory.item[inventory.selectedIndex].inUse == TRUE)
	{
		dropItem(&inventory.item[inventory.selectedIndex]);

		inventory.item[inventory.selectedIndex].inUse = FALSE;

		sortInventory();
	}
}

void useInventoryItem()
{
	Entity *temp;

	if (inventory.item[inventory.selectedIndex].inUse == TRUE && inventory.item[inventory.selectedIndex].activate != NULL)
	{
		temp = self;

		self = &inventory.item[inventory.selectedIndex];

		self->activate(0);

		if (inventory.item[inventory.selectedIndex].inUse == FALSE)
		{
			sortInventory();
		}

		self = temp;
	}
}

static void sortInventory()
{
	int i, j;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == FALSE)
		{
			for (j=i;j<MAX_INVENTORY_ITEMS;j++)
			{
				if (inventory.item[j].inUse == TRUE)
				{
					inventory.item[i] = inventory.item[j];

					inventory.item[j].inUse = FALSE;

					break;
				}
			}
		}
	}

	while (inventory.item[inventory.selectedIndex].inUse == FALSE)
	{
		inventory.selectedIndex--;

		if (inventory.selectedIndex < 0)
		{
			inventory.selectedIndex = 0;

			break;
		}
	}
}

void doInventory()
{
	int i;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == TRUE)
		{
			inventory.item[i].thinkTime--;

			if (inventory.item[i].thinkTime < 0)
			{
				inventory.item[i].thinkTime = 0;
			}
		}
	}
}

void drawSelectedInventoryItem(int x, int y, int w, int h)
{
	self = &inventory.item[inventory.selectedIndex];

	if (self->inUse == TRUE)
	{
		drawLoopingAnimation(self, x, y, w, h, 1);
	}
}

void addRequiredToInventory(Entity *other)
{
	Entity *item;

	if (!(self->flags & INVULNERABLE) && other->type == PLAYER)
	{
		item = getInventoryItem(self->requires);

		if (item != NULL)
		{
			item->health++;

			self->inUse = FALSE;

			addHudMessage(STANDARD_MESSAGE, "Picked up %s", self->objectiveName);

			fireTrigger(item->objectiveName);
		}

		else
		{
			addHudMessage(BAD_MESSAGE, "%s is required to carry this item", self->requires);
		}
	}
}

void loadInventoryItems()
{
	int i, j;
	Entity e;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == TRUE)
		{
			printf("Reloading properties for %s\n", inventory.item[i].name);

			loadProperties(inventory.item[i].name, &e);

			printf("Realigning animations for %s\n", inventory.item[i].name);

			for (j=0;j<MAX_ANIMATION_TYPES;j++)
			{
				inventory.item[i].animation[j] = e.animation[j];
			}

			printf("Resetting stand animation for %s\n", inventory.item[i].name);

			setEntityAnimation(&inventory.item[i], STAND);

			if (inventory.item[i].type == WEAPON && strcmpignorecase(inventory.item[i].name, playerWeapon.name) == 0)
			{
				printf("Resetting player weapon\n");

				autoSetPlayerWeapon(&inventory.item[i]);
			}

			else if (inventory.item[i].type == SHIELD && strcmpignorecase(inventory.item[i].name, playerWeapon.name) == 0)
			{
				printf("Resetting player shield\n");

				autoSetPlayerShield(&inventory.item[i]);
			}
		}
	}

	printf("Done reloading inventory\n");
}
