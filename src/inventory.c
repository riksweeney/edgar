#include "headers.h"

static Inventory inventory;
extern Entity *self;

static void sortInventory(void);

int addToInventory(Entity *e)
{
	int i;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].active == INACTIVE)
		{
			inventory.item[i] = *e;

			inventory.item[i].face = RIGHT;

			inventory.item[i].thinkTime = 0;

			setEntityAnimation(&inventory.item[i], STAND);

			e->active = INACTIVE;

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
		if (inventory.item[i].active == ACTIVE)
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

	while (inventory.item[i].active == INACTIVE);

	inventory.selectedIndex = i;
}

int inventoryHasItem(char *name)
{
	int i;

	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].active == ACTIVE && strcmpignorecase(inventory.item[i].name, name) == 0)
		{
			return 1;
		}
	}

	return 0;
}

void dropInventoryItem()
{
	if (inventory.item[inventory.selectedIndex].active == ACTIVE)
	{
		dropItem(&inventory.item[inventory.selectedIndex]);

		inventory.item[inventory.selectedIndex].active = INACTIVE;

		sortInventory();
	}
}

void useInventoryItem()
{
	Entity *temp;

	if (inventory.item[inventory.selectedIndex].active == ACTIVE)
	{
		temp = self;

		self = &inventory.item[inventory.selectedIndex];

		self->activate(0);

		if (inventory.item[inventory.selectedIndex].active == INACTIVE)
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
		if (inventory.item[i].active == INACTIVE)
		{
			for (j=i;j<MAX_INVENTORY_ITEMS;j++)
			{
				if (inventory.item[j].active == ACTIVE)
				{
					inventory.item[i] = inventory.item[j];

					inventory.item[j].active = INACTIVE;

					break;
				}
			}
		}
	}

	while (inventory.item[inventory.selectedIndex].active == INACTIVE)
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
		if (inventory.item[i].active == ACTIVE)
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

	if (self->active == ACTIVE)
	{
		drawLoopingAnimation(self, x, y, w, h, 1);
	}
}
