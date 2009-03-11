#include "headers.h"

#include "animation.h"
#include "properties.h"
#include "entity.h"
#include "random.h"
#include "collisions.h"
#include "audio.h"
#include "script.h"

extern Entity *self;

static void wait(void);
static void talk(int);

Entity *addVillager(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Villager\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &wait;

	e->draw = &drawLoopingAnimationToMap;
	e->activate = &talk;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	checkToMap(self);
}

static void talk(int val)
{
	loadScript(self->requires);

	readNextScriptLine();
}
