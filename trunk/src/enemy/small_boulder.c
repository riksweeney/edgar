#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../collisions.h"
#include "../item/item.h"

extern Entity *self;

static void roll(void);
static void die(void);

Entity *addSmallBoulder(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Small Boulder\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;

	e->touch = &entityTouch;

	e->action = &roll;

	e->type = ENEMY;

	e->dirY = 0;

	setEntityAnimation(e, STAND);

	return e;
}

static void roll()
{
	float dirX = self->dirX;

	self->thinkTime--;

	if (!(self->flags & ON_GROUND))
	{
		self->frameSpeed = 0;

		self->dirX = 0;
	}

	else if (self->thinkTime <= 0)
	{
		self->dirX = (self->face == LEFT ? -self->speed : self->speed);

		self->frameSpeed = 1;
	}

	checkToMap(self);

	if (self->dirX == 0 && dirX != 0)
	{
		die();
	}
}

static void die()
{
	int i;

	Entity *e;

	for (i=0;i<4;i++)
	{
		e = addTemporaryItem("misc/small_boulder_piece", self->x, self->y, self->face, 0, 0);

		e->x += (self->x - e->x) / 2;
		e->y += (self->y - e->y) / 2;

		e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimation(e, i);

		e->thinkTime = 60 + (prand() % 60);
	}

	self->inUse = FALSE;
}
