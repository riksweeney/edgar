#include "bat.h"

extern void drawLoopingAnimationToMap(void);
extern void loadProperties(char *, Entity *);
extern Entity *getFreeEntity(void);
extern void setEntityAnimation(Entity *, int);
extern void checkToMap(Entity *);
extern long prand(void);
extern void playSoundAtIndex(int);
extern void entityTakeDamage(Entity *, int);
extern void entityDie(void);
extern void entityTouch(Entity *);

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

void fly()
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
