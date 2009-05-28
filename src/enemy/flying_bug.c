#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../system/random.h"
#include "../graphics/graphics.h"

extern Entity *self;

static void wait(void);
static void init(void);
static void moveToTarget(void);
static void changeRandomTarget(void);

Entity *addFlyingBug(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Flying Bug\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &entityDie;
	e->pain = NULL;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeTarget;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	self->thinkTime = 0;
	
	self->action = &wait;
	
	self->action();
}

static void wait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		changeRandomTarget();

		self->action = &moveToTarget;
	}
}

static void changeRandomTarget()
{
	float x, y, t;

	self->targetX = self->x + (prand() % 64) * (prand() % 2 == 0 ? -1 : 1);
	self->targetY = self->y + (prand() % 64) * (prand() % 2 == 0 ? -1 : 1);

	x = self->targetX - self->x;
	y = self->targetY - self->y;

	t = fabs(x) + fabs(y);

	self->dirX = x / t * self->speed;
	self->dirY = y / t * self->speed;
}

static void moveToTarget()
{
	float x, y;
	
	if (self->dirX == 0 || self->dirY == 0)
	{
		changeRandomTarget();
	}

	x = self->dirX;
	y = self->dirY;

	checkToMap(self);

	if (fabs(self->targetX - self->x) < fabs(self->dirX) && fabs(self->targetY - self->y) < fabs(self->dirY))
	{
		self->thinkTime = 120 + (prand() % self->maxThinkTime);

		self->action = &wait;
	}

	else if (self->dirX != x || self->dirY != y)
	{
		changeRandomTarget();
	}
}
