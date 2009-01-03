#include "coal.h"

extern void addItemToInventory(Entity *e);
extern int inventoryHasItem(char *);

static void touch(Entity *);

void addCoal(int x, int y)
{
	Entity *e = getFreeEntity();
	
	if (e == NULL)
	{
		printf("No free slots to add Coal\n");
		
		exit(1);
	}
	
	loadProperties("coal", e);
	
	e->x = x;
	e->y = y;
	
	e->dirY = -6;
	
	e->thinkTime = 300;
	e->type = ITEM;
	
	e->face = RIGHT;
	
	e->action = &generalItemAction;
	e->touch = &touch;
	e->draw = &drawLoopingEntityAnimation;
	
	setEntityAnimation(e, STAND_RIGHT);
}

static void touch(Entity *other)
{
	if (other->type == PLAYER && inventoryHasItem("coal_bag") == 1)
	{
		addItemToInventory(self);
	}
}
