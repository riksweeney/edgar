#include "inventory.h"

extern void addEntity(Entity, int, int);

static void sortInventory(void);

int addToInventory(Entity *e)
{
	int i;
	
	for (i=0;i<MAX_INVENTORY_ITEMS;i++)
	{
		if (inventory.item[i].active == INACTIVE)
		{
			inventory.item[i] = *e;
			
			e->active = INACTIVE;
			
			return 1;
		}
	}
	
	return 0;
}

void selectNextItem(int index)
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
}

void dropInventoryItem(int x, int y)
{
	if (inventory.item[inventory.selectedIndex].active == ACTIVE)
	{
		addEntity(inventory.item[inventory.selectedIndex], x, y);
		
		inventory.item[inventory.selectedIndex].active = INACTIVE;
	}
}

void useInventoryItem(int x, int y)
{
	if (inventory.item[inventory.selectedIndex].active == ACTIVE)
	{
		inventory.item[inventory.selectedIndex].activate(&player);
		
		inventory.item[inventory.selectedIndex].active = INACTIVE;
		
		sortInventory();
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
					
					break;
				}
			}
		}
	}
}
