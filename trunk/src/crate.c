#include "headers.h"



void addWoodenCrate(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a wooden crate\n");

		exit(1);
	}

	loadProperties("item/wooden_crate", e);

	e->x = x;
	e->y = y;

	e->face = RIGHT;

	e->action = &doNothing;
	e->touch = &pushEntity;
	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);
}

void addMetalCrate(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a metal crate\n");

		exit(1);
	}

	loadProperties("item/metal_crate", e);

	e->x = x;
	e->y = y;

	e->face = RIGHT;

	e->action = &doNothing;
	e->touch = &pushEntity;
	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);
}
