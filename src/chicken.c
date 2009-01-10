#include "chicken.h"

extern void setEntityAnimation(Entity *, int);
extern void drawLoopingAnimationToMap(void);
extern void loadProperties(char *, Entity *);
extern Entity *getFreeEntity(void);
extern void drawLoopingAnimationToMap(void);
extern long prand(void);
extern int collision(int, int, int, int, int, int, int, int);
extern void checkToMap(Entity *);
extern void doNothing(void);

static void lookForFood(void);
static void moveToFeed(void);
static void finishEating(void);

void addChicken(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Chicken\n");

		exit(1);
	}

	loadProperties("enemy/chicken", e);

	e->x = x;
	e->y = y;

	e->action = &lookForFood;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);
}

void lookForFood()
{
	int i;

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

		self->thinkTime = 180 * prand() % 120;
	}

	if (prand() % 20 == 0)
	{
		for (i=0;i<MAX_ENTITIES;i++)
		{
			if (entity[i].active == INACTIVE || entity[i].type != KEY_ITEM || &entity[i] == self || strcmpignorecase(entity[i].name, "chicken_feed") != 0)
			{
				continue;
			}

			if (collision(self->x, self->y, self->face == RIGHT ? 400: -400, self->h, entity[i].x, entity[i].y, entity[i].w, entity[i].h) == 1)
			{
				self->target = &entity[i];

				self->action = &moveToFeed;

				return;
			}
		}
	}

	if (self->dirX < 0)
	{
		self->face = LEFT;
	}

	else if (self->dirX > 0)
	{
		self->face = RIGHT;
	}

	checkToMap(self);
}

static void moveToFeed()
{
	if (self->target->active == ACTIVE && abs(self->x - self->target->x) > self->speed)
	{
		self->dirX += self->target->x < self->x ? -self->speed : self->speed;
	}

	else
	{
		self->target->thinkTime = 600;

		self->action = &doNothing;

		self->animationCallback = &finishEating;

		setEntityAnimation(self, ATTACK_1);
	}

	checkToMap(self);
}

static void finishEating()
{
	self->target->health--;

	if (self->target->health <= 0)
	{
		self->active = INACTIVE;

		self->target = NULL;

		self->action = &lookForFood;

		self->thinkTime = 0;
	}

	else
	{
		self->target->thinkTime = 600;

		self->animationCallback = &finishEating;
	}
}
