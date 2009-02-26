#include "headers.h"

#include "animation.h"
#include "properties.h"
#include "entity.h"
#include "hud.h"

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
}

static void touch(Entity *other)
{
	int frame = self->currentFrame;
	
	if (other->type == PLAYER)
	{
		self->thinkTime = 60;
		
		setEntityAnimation(self, WALK);
		
		self->currentFrame = frame;

		setInfoBoxMessage(0, "Press Action to save your game");
	}
}

static void activate(int val)
{
	printf("Saving game\n");
}
