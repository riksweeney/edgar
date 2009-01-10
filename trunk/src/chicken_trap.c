#include "chicken_trap.h"

void addChickenTrap(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add chicken_feed bag\n");

		exit(1);
	}

	loadProperties("item/chicken_feed_bag", e);

	e->x = x;
	e->y = y;

	e->dirY = ITEM_JUMP_HEIGHT;

	e->type = ITEM;

	e->face = LEFT;

	e->action = &doNothing;
	e->touch = keyItemTouch;
	e->activate = &dropChickenFeed;
	e->die = &keyItemRespawn;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);
}
