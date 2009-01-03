#include "crate.h"

extern void setEntityAnimation(Entity *, int);
extern void loadProperties(char *, Entity *);
extern Entity *getFreeEntity(void);
extern void doNothing(void);
extern void pushEntity(Entity *);
extern void drawLoopingEntityAnimation(void);

void addWoodenCrate(int x, int y)
{
	Entity *e = getFreeEntity();
	
	if (e == NULL)
	{
		printf("No free slots to add a wooden crate\n");
		
		exit(1);
	}
	
	loadProperties("wooden_crate", e);
	
	e->x = x;
	e->y = y;
	
	e->face = RIGHT;
	
	e->action = &doNothing;
	e->touch = &pushEntity;
	e->draw = &drawLoopingEntityAnimation;
	
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
	
	loadProperties("metal_crate", e);
	
	e->x = x;
	e->y = y;
	
	e->face = RIGHT;
	
	e->action = &doNothing;
	e->touch = &pushEntity;
	e->draw = &drawLoopingEntityAnimation;
	
	setEntityAnimation(e, STAND);
}
