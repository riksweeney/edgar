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

	strcpy(e->requires, entityToSpawn);

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->action = &init;

	e->type = SPAWNER;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	self->maxHealth = self->thinkTime;

	self->action = &spawn;

	if (self->active == INACTIVE)
	{
		self->thinkTime = 0;
	}

	if (self->health == 0)
	{
		self->health = -1;
	}

	self->action();
}

static void spawn()
{
	Entity *e;

	if (self->active == ACTIVE)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			e = addEnemy(self->objectiveName, self->x, self->y);

			e->x += (self->w - e->w) / 2;
			e->y += (self->h - e->h) / 2;

			self->thinkTime = self->maxHealth;

			if (self->health != -1)
			{
				self->health--;

				if (self->health == 0)
				{
					self->inUse = NOT_IN_USE;
				}
			}
		}
	}
}
