#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../event/script.h"

extern Entity *self;

static void standardTouch(Entity *);
static void scriptTouch(Entity *);
static void wait(void);
static void initialise(void);

Entity *addLineDef(char *type, char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("Couldn't get a free slot for a Line def!\n");

		exit(1);
	}

	STRNCPY(e->name, name, sizeof(e->name));

	e->flags |= NO_DRAW;

	e->action = &initialise;

	if (strcmpignorecase(type, "SCRIPT_LINE_DEF") == 0)
	{
		e->type = SCRIPT_LINE_DEF;

		e->touch = &scriptTouch;
	}

	else
	{
		e->type = LINE_DEF;

		e->touch = &standardTouch;
	}

	e->x = x;
	e->y = y;

	e->draw = &drawLineDefToMap;

	return e;
}

static void standardTouch(Entity *other)
{
	if (other->type == PLAYER && self->active == TRUE)
	{
		activateEntitiesWithName(self->objectiveName, (self->health >= 0 ? TRUE : FALSE));

		self->inUse = FALSE;
	}
}

static void scriptTouch(Entity *other)
{
	if (other->type == PLAYER && self->active == TRUE)
	{
		loadScript(self->objectiveName);

		readNextScriptLine();

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
