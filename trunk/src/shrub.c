#include "headers.h"

#include "animation.h"
#include "properties.h"
#include "entity.h"
#include "audio.h"

extern Entity *self;

static void touch(Entity *);
static void die(void);

Entity *addShrub(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Shrub\n");

		exit(1);
	}

	loadProperties("misc/small_tree", e);

	e->x = x;
	e->y = y;

	e->action = &doNothing;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->die = &die;

	e->type = KEY_ITEM;

	setEntityAnimation(e, STAND);

	return e;
}

static void touch(Entity *other)
{
	pushEntity(other);

	if ((other->flags & ATTACKING) && strcmpignorecase(other->name, self->requires) == 0)
	{
		self->health--;

		if (self->health <= 0)
		{
			self->action = &die;
		}
	}
}

static void die()
{
	self->inUse = FALSE;
}
