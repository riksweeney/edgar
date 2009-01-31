#include "headers.h"

#include "animation.h"
#include "enemies.h"
#include "entity.h"
#include "properties.h"

extern Entity *self;

static void spawn(void);
static void init(void);

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
	e->action = &init;

	e->type = SPAWNER;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	self->health = self->thinkTime;
	
	self->action = &spawn;
	
	self->action();
}

static void spawn()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = addEnemy(self->activates, self->x, self->y);

		e->x += (self->w - e->w) / 2;
		e->y += (self->h - e->h) / 2;

		self->thinkTime = self->health;
	}
}
