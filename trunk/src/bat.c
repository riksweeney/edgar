#include "bat.h"

extern void drawLoopingEntityAnimation(void);
extern void loadProperties(char *, Entity *);
extern Entity *getFreeEntity(void);
extern void setEntityAnimation(Entity *, int);
extern void checkToMap(Entity *);
extern void setCustomAction(Entity *, void (*)(int *), int);
extern void pushBack(int *);
extern void invulnerable(int *);

static void fly(void);
static void touch(Entity *);

void addBat(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Bat\n");
		
		exit(1);
	}
	
	loadProperties("bat", e);
	
	e->x = x;
	e->y = y;
	
	e->action = &fly;
	
	e->draw = &drawLoopingEntityAnimation;
	e->touch = &touch;
	e->type = ENEMY;
	
	setEntityAnimation(e, STAND);
}

static void fly()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		if (self->dirX == 0)
		{
			self->dirX = 0.1f + 0.1f * (rand() % 15) * (rand() % 2 == 0 ? 1 : -1);
		}
		
		else
		{
			self->dirX = 0;
		}
		
		self->thinkTime = 60 * rand() % 120;
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

static void touch(Entity *other)
{
	if (other->type == PLAYER)
	{
		other->health--;
		
		setCustomAction(other, &pushBack, 4);
		
		setCustomAction(other, &invulnerable, 60);
		
		if (other->dirX == 0)
		{
			other->dirX = self->dirX < 0 ? -30 : 30;
		}
		
		else
		{
			other->dirX = other->dirX < 0 ? 30 : -30;
		}
	}
}
