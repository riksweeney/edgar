#include "../headers.h"

#include "../animation.h"
#include "../properties.h"
#include "../entity.h"

extern Entity *self;

static void wait(void);
static void activate(int);
static void touch(Entity *);

Entity *addPressurePlate(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("Couldn't get a free slot for a Pressure Plate!\n");

		exit(1);
	}

	loadProperties(name, e);

	e->touch = &touch;

	e->action = &wait;

	e->draw = &drawLoopingAnimationToMap;

	e->activate = &activate;

	e->x = x;
	e->y = y;

	e->health = 0;

	e->maxHealth = e->health;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	if (self->health != self->maxHealth)
	{
		setEntityAnimation(self, STAND);

		activateEntitiesWithName(self->objectiveName, FALSE);

		self->maxHealth = self->health;
	}

	self->health = 0;
}

static void touch(Entity *other)
{
	pushEntity(other);

	if (other->standingOn == self)
	{
		if (strlen(self->requires) == 0 || strcmpignorecase(self->requires, other->objectiveName) == 0)
		{
			activate(1);
		}
	}
}

static void activate(int val)
{
	self->health = val;

	if (self->health != self->maxHealth)
	{
		setEntityAnimation(self, WALK);

		activateEntitiesWithName(self->objectiveName, TRUE);

		self->maxHealth = self->health;
	}
}
