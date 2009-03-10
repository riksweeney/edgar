#include "headers.h"

#include "collisions.h"
#include "animation.h"
#include "properties.h"
#include "entity.h"
#include "decoration.h"

extern Entity *self;

static void jump(void);
static void draw(void);
static void move(void);

Entity *addJumpingFireball(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Fireball\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &jump;

	e->draw = &draw;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

Entity *addFireball(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Fireball\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &move;

	e->draw = &draw;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void move()
{
	checkToMap(self);

	if (self->dirX == 0 && self->dirY == 0)
	{
		self->inUse = FALSE;
	}
}

static void jump()
{
	if (self->flags & ON_GROUND)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->dirY = -self->speed;

			self->thinkTime = self->maxThinkTime;

			self->health = 5;
		}
	}

	else
	{
		self->health--;

		setEntityAnimation(self, self->dirY >= 0 ? WALK : STAND);
	}

	checkToMap(self);
}

static void draw()
{
	drawLoopingAnimationToMap();
}
