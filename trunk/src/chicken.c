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
extern int isAtEdge(Entity *);
extern void playSound(char *, int, int, int);
extern int getDistance(int, int, int, int);

static void lookForFood(void);
static void wander(void);
static void moveToFood(void);
static void finishEating(void);

Entity *addChicken(int x, int y)
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

	e->action = &wander;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void lookForFood()
{
	int i, distance, target, newDistance;
	
	newDistance = distance = target = -1;

	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].active == INACTIVE || (strcmpignorecase(entity[i].name, "item/chicken_feed") != 0 && strcmpignorecase(entity[i].name, "item/chicken_trap") != 0))
		{
			continue;
		}

		if (entity[i].health > 3 && collision(self->x + (self->face == RIGHT ? 0: -200), self->y, 200, self->h, entity[i].x, entity[i].y, entity[i].w, entity[i].h) == 1)
		{
			newDistance = getDistance(self->x, self->y, entity[i].x, entity[i].y);
			
			if (target == -1 || newDistance < distance)
			{
				distance = newDistance;
				
				target = i;
			}
		}
	}
	
	if (target != -1)
	{
		self->target = &entity[target];
	
		self->action = &moveToFood;
	
		printf("Spotted chicken feed\n");
	
		return;
	}

	self->action = &wander;
}

static void wander()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->dirX == 0)
		{
			self->dirX = self->speed * (prand() % 2 == 0 ? -1 : 1);

			setEntityAnimation(self, WALK);
		}

		else
		{
			self->dirX = 0;

			setEntityAnimation(self, STAND);
		}

		self->thinkTime = 180 + prand() % 120;
	}

	if (prand() % 2400 == 0)
	{
		playSound("enemy/chicken/cluck.wav", -1, self->x, self->y);
	}

	if (prand() % 30 == 0)
	{
		lookForFood();
	}

	if (self->dirX < 0)
	{
		self->face = LEFT;
	}

	else if (self->dirX > 0)
	{
		self->face = RIGHT;
	}

	if (self->dirX != 0 && isAtEdge(self) == 1)
	{
		self->dirX = 0;

		setEntityAnimation(self, STAND);
	}

	checkToMap(self);
}

static void moveToFood()
{
	if (self->target->health > 3 && abs(self->x + (self->face == RIGHT ? self->w : 0) - self->target->x) > self->speed)
	{
		self->dirX = self->target->x < self->x ? -self->speed : self->speed;

		if (self->dirX != 0 && isAtEdge(self) == 1)
		{
			self->dirX = 0;

			setEntityAnimation(self, STAND);
		}

		else
		{
			setEntityAnimation(self, WALK);
		}
	}

	else
	{
		if (strcmpignorecase(self->target->name, "item/chicken_feed") == 0)
		{
			self->target->thinkTime = 600;
		}

		self->action = &doNothing;

		setEntityAnimation(self, ATTACK_1);

		self->animationCallback = &finishEating;
	}

	checkToMap(self);
}

static void finishEating()
{
	self->target->health--;

	printf("Health is %d\n", self->target->health);

	if (self->target->health <= 0)
	{
		self->target->active = INACTIVE;

		self->target = NULL;

		self->action = &lookForFood;

		self->thinkTime = 0;
	}

	else
	{
		if (strcmpignorecase(self->target->name, "item/chicken_feed") == 0)
		{
			self->target->thinkTime = 600;
		}

		self->action = &doNothing;

		setEntityAnimation(self, ATTACK_1);

		self->animationCallback = &finishEating;
	}
}
