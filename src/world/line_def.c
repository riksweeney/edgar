#include "../headers.h"

#include "../animation.h"
#include "../properties.h"
#include "../entity.h"

extern Entity *self;

static void touch(Entity *);
static void wait(void);
static void initialise(void);

Entity *addLineDef(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("Couldn't get a free slot for a Line def!\n");

		exit(1);
	}

	STRNCPY(e->name, name, sizeof(e->name));

	e->flags |= NO_DRAW;

	e->type = LINE_DEF;

	e->action = &initialise;

	e->touch = &touch;

	e->x = x;
	e->y = y;

	e->draw = &drawLineDefToMap;

	return e;
}

static void touch(Entity *other)
{
	if (other->type == PLAYER && self->active == TRUE)
	{
		activateEntitiesWithName(self->objectiveName, (self->health >= 0 ? TRUE : FALSE));

		self->inUse = FALSE;
	}
}

static void wait()
{

}

static void initialise()
{
	self->x = self->startX;
	self->y = self->startY;

	self->w = abs(self->startX - self->endX) + 1;
	self->h = abs(self->startY - self->endY) + 1;

	self->action = &wait;
}
