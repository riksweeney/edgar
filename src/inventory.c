#include "headers.h"

#include "animation.h"
#include "player.h"
#include "item.h"

static Inventory inventory;
extern Entity *self;

static void sortInventory(void);

int addToInventory(Entity *e)
{
	int i;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == NOT_IN_USE)
		{
			inventory.item[i] = *e;

			inventory.item[i].face = RIGHT;

			inventory.item[i].thinkTime = 0;

			setEntityAnimation(&inventory.item[i], STAND);

			e->inUse = NOT_IN_USE;

			if (inventory.item[i].type == WEAPON)
			{
				autoSetPlayerWeapon(&inventory.item[i]);
			}

			else if (inventory.item[i].type == SHIELD)
			{
				autoSetPlayerShield(&inventory.item[i]);
			}

			return 1;
		}
	}

	return 0;
}

void selectNextInventoryItem(int index)
{
	int i = 0;
	int itemCount = 0;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == IN_USE)
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

	while (inventory.item[i].inUse == NOT_IN_USE);

	inventory.selectedIndex = i;
}

int inventoryHasItem(char *name)
{
	int i;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == IN_USE && strcmpignorecase(inventory.item[i].activates, name) == 0)
		{
			return 1;
		}
	}

	return 0;
}

int removeInventoryItem(char *name)
{
	int i;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].inUse == IN_USE && strcmpignorecase(inventory.item[i].activates, name) == 0)
		{
			inventory.item[i].inUse = NOT_IN_USE;

			return 1;
		}
	}

	return 0;
}

void dropInventoryItem()
{
	if (inventory.item[inventory.selectedIndex].inUse == IN_USE)
	{
		dropItem(&inventory.item[inventory.selectedIndex]);

		inventory.item[inventory.selectedIndex].inUse = NOT_IN_USE;

		sortInventory();
	}
}

void useInventoryItem()
{
	Entity *temp;

	if (inventory.item[inventory.selectedIndex].inUse == IN_USE)
	{
		temp = self;

		self = &inventory.item[inventory.selectedIndex];

		self->activate(0);

		if (inventory.item[inventory.selectedIndex].inUse == NOT_IN_USE)
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
		if (inventory.item[i].inUse == NOT_IN_USE)
		{
			for (j=i;j<MAX_INVENTORY_ITEMS;j++)
			{
				if (inventory.item[j].inUse == IN_USE)
				{
					inventory.item[i] = inventory.item[j];

					inventory.item[j].inUse = NOT_IN_USE;

					break;
				}
			}
		}
	}

	while (inventory.item[inventory.selectedIndex].inUse == NOT_IN_USE)
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
		if (inventory.item[i].inUse == IN_USE)
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

	if (self->inUse == IN_USE)
	{
		drawLoopingAnimation(self, x, y, w, h, 1);
	}
}
