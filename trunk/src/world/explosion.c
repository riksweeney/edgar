#include "../headers.h"

#include "../graphics/animation.h"
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"

extern Entity *self;

static void finish(void);

Entity *addExplosion(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add an Explosion\n");

		exit(1);
	}

	loadProperties("common/explosion", e);

	e->x = x;
	e->y = y;

	e->type = PROJECTILE;

	e->flags |= FLY|ATTACKING;

	e->face = RIGHT;

	e->action = &doNothing;
	e->touch = &entityTouch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	e->animationCallback = &finish;

	return e;
}

static void finish()
{
	self->inUse = FALSE;
}
