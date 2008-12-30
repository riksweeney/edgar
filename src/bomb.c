#include "bomb.h"

void addBomb(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("Could not get a free slot for a Bomb!\n");
		
		exit(1);
	}
	
	e->x = x;
	e->y = y;
	
	e->flags = NO_TOUCH;
	
	e->action = &doNothing;
	e->activate = &dropBomb;
	e->draw = &drawLoopingAnimation;
	
	setAnimation("BOMB_INACTIVE", e);
}

static void dropBomb()
{
	setAnimation("BOMB_ACTIVE", self);
	
	e->x = 
}
