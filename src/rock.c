#include "headers.h"

static void largeRockFall(void);
static void smallRockFall(void);
static void wait(void);

extern Entity *self;

Entity *addLargeRock(int x, int y)
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
	e->action = &wait;

	e->type = ENEMY;
	
	e->health = e->thinkTime;
	
	e->dirY = 0;

	setEntityAnimation(e, STAND);

	return e;
}

Entity *addSmallRock(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Small Rock\n");

		exit(1);
	}

	loadProperties("common/small_rock", e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;
	
	e->health = e->thinkTime;
	
	e->dirY = 0;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->flags &= ~FLY;
		
		self->action = &largeRockFall;
	}
}

static void largeRockFall()
{
	Entity *e;
	
	checkToMap(self);
	
	if (self->dirY == 0)
	{
		e = addSmallRock(self->x, self->y);
		
		e->x += (self->w - e->w) / 2;
		e->y += (self->h - e->h) / 2;
		
		e->action = &smallRockFall;
		
		e->dirX = -3;
		e->dirY = -8;
		
		e = addSmallRock(self->x, self->y);
		
		e->x += (self->w - e->w) / 2;
		e->y += (self->h - e->h) / 2;
		
		e->action = &smallRockFall;
		
		e->dirX = 3;
		e->dirY = -8;
		
		self->active = INACTIVE;
	}
}

static void smallRockFall()
{
	checkToMap(self);
	
	if (self->flags & ON_GROUND)
	{
		self->active = INACTIVE;
	}
}
