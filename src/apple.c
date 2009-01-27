#include "headers.h"

void addApple(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add an apple\n");

		exit(1);
	}

	loadProperties("item/apple", e);

	e->x = x;
	e->y = y;

	e->dirY = ITEM_JUMP_HEIGHT;

	e->thinkTime = 300;
	e->type = HEALTH;

	e->face = RIGHT;

	e->action = &generalItemAction;
	e->touch = &healthTouch;
	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);
}
