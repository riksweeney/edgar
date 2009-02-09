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
	int speed;
	
	if (!(self->flags & HELPLESS))
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			switch (prand() % 5)
			{
				case 0:
				case 1:
					self->dirX = self->speed;
				break;
				
				case 2:
				case 3:
					self->dirX = -self->speed;
				break;
				
				default:
					self->dirX = 0;
				break;
			}
			
			self->thinkTime = 180 + prand() % 120;
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
	
	speed = self->dirX;

	checkToMap(self);
	
	if (self->dirX == 0 && speed != 0)
	{
		self->dirX = speed * -1;
	}
}
