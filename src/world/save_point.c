#include "headers.h"

#include "animation.h"
#include "properties.h"
#include "entity.h"
#include "hud.h"
#include "load_save.h"

extern Entity *self;

static void wait(void);
static void touch(Entity *);
static void activate(int);

Entity *addSavePoint(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Save Point\n");

		exit(1);
	}

	loadProperties("common/save_point", e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->activate = &activate;
	e->action = &wait;

	e->type = SAVE_POINT;

	e->health = 0;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	int frame = self->currentFrame;

	self->thinkTime--;

	if (self->thinkTime < 0)
	{
		self->thinkTime = 0;

		setEntityAnimation(self, STAND);

		self->currentFrame = frame;
	}

	self->health--;

	if (self->health < 0)
	{
		self->health = 0;
	}
}

static void touch(Entity *other)
{
	int frame = self->currentFrame;

	if (other->type == PLAYER)
	{
		self->thinkTime = 5;

		setEntityAnimation(self, WALK);

		self->currentFrame = frame;

		if (self->health == 0)
		{
			setInfoBoxMessage(5, "Press Action to save your game");
		}

		else
		{
			setInfoBoxMessage(5, "Game saved");
		}
	}
}

static void activate(int val)
{
	if (self->health == 0)
	{
		self->health = 120;

		saveGame(0);
	}
}
