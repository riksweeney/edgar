#include "headers.h"

#include "animation.h"
#include "properties.h"
#include "entity.h"
#include "random.h"
#include "collisions.h"

extern Entity *self;

static void fly(void);

Entity *addBat(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Bat\n");

		exit(1);
	}

	loadProperties("enemy/bat", e);

	e->x = x;
	e->y = y;

	e->action = &fly;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->takeDamage = &entityTakeDamage;
	e->die = &entityDie;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void fly()
{
	if (!(self->flags & HELPLESS))
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			if (self->dirX == 0)
			{
				self->dirX = self->speed * (prand() % 2 == 0 ? -1 : 1);
			}

			else
			{
				self->dirX = 0;
			}

			self->thinkTime = 60 * prand() % 120;
		}

		if (self->dirX < 0)
		{
			self->face = LEFT;
		}

		else if (self->dirX > 0)
		{
			self->face = RIGHT;
		}
	}

	checkToMap(self);
}
