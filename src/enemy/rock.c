#include "headers.h"

#include "animation.h"
#include "audio.h"
#include "entity.h"
#include "properties.h"
#include "collisions.h"
#include "random.h"

static void largeRockFall(void);
static void smallRockFall(void);
static void shake(void);

extern Entity *self;

Entity *addLargeRock(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Large Rock\n");

		exit(1);
	}

	loadProperties("common/large_rock", e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->action = &shake;

	e->type = ENEMY;

	e->dirX = 2 * (prand() % 2 == 0 ? -1 : 1);

	e->x += e->dirX;

	e->dirY = 0;

	setEntityAnimation(e, STAND);

	return e;
}

Entity *addSmallRock(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Small Rock\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;

	e->action = &smallRockFall;

	e->type = ENEMY;

	e->dirY = 0;

	setEntityAnimation(e, STAND);

	return e;
}

static void shake()
{
	self->dirY = 0;

	if (self->thinkTime % 4 == 0)
	{
		self->dirX *= -1;

		self->x += self->dirX * 2;
	}

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->x -= self->dirX;

		self->dirX = 0;

		self->action = &largeRockFall;
	}
}

static void largeRockFall()
{
	Entity *e;

	if (self->flags & ON_GROUND)
	{
		playSound("sound/common/rock_bounce.wav", OBJECT_CHANNEL_1, OBJECT_CHANNEL_1, self->x, self->y);

		e = addSmallRock(self->x, self->y, "common/small_rock");

		e->x += (self->w - e->w) / 2;
		e->y += (self->h - e->h) / 2;

		e->dirX = -3;
		e->dirY = -8;

		e = addSmallRock(self->x, self->y, "common/small_rock");

		e->x += (self->w - e->w) / 2;
		e->y += (self->h - e->h) / 2;

		e->dirX = 3;
		e->dirY = -8;

		self->inUse = FALSE;
	}

	checkToMap(self);
}

static void smallRockFall()
{
	checkToMap(self);

	if (self->flags & ON_GROUND || self->standingOn != NULL)
	{
		self->inUse = FALSE;
	}
}
