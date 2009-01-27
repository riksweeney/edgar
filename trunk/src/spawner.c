#include "headers.h"

extern Entity *self;

static void spawn(void);

Entity *addSpawner(int x, int y, char *entityToSpawn)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Spawner\n");

		exit(1);
	}

	loadProperties("common/spawner", e);

	e->x = x;
	e->y = y;

	strcpy(e->activates, entityToSpawn);

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->action = &spawn;

	e->type = SPAWNER;

	setEntityAnimation(e, STAND);

	return e;
}

static void spawn()
{
	Entity *e;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		printf("Spawning %s\n", self->activates);
		
		e = addEnemy(self->activates, self->x, self->y);
		
		e->x += (self->w - e->w) / 2;
		e->y += (self->h - e->h) / 2;
		
		self->thinkTime = self->health;
	}
}
