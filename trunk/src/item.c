#include "item.h"

extern void checkToMap(Entity *);
extern Entity *getFreeEntity(void);
extern void loadProperties(char *, Entity *);
extern void drawLoopingEntityAnimation(void);
extern void setEntityAnimation(Entity *, int);
extern void doNothing(void);

void healthTouch(Entity *other);
void generalItemAction(void);
void touchKeyItem(Entity *);

void addGenericItem(char *name, int x, int y, int type)
{
	Entity *e = getFreeEntity();
	
	if (e == NULL)
	{
		printf("No free slots to add %s\n", name);
		
		exit(1);
	}
	
	loadProperties(name, e);
	
	e->x = x;
	e->y = y;
	
	e->dirY = -6;
	
	e->thinkTime = 300;
	e->type = ITEM;
	
	e->face = RIGHT;
	
	e->action = &generalItemAction;
	
	switch (type)
	{
		case HEALTH:
			e->touch = &healthTouch;
		break;
	}
	
	e->draw = &drawLoopingEntityAnimation;
	
	setEntityAnimation(e, STAND);
}

void addKeyItem(char *name, int x, int y)
{
	Entity *e = getFreeEntity();
	
	if (e == NULL)
	{
		printf("No free slots to add %s\n", name);
		
		exit(1);
	}
	
	loadProperties(name, e);
	
	e->x = x;
	e->y = y;
	e->dirY = -6;
	
	e->action = &doNothing;
	
	e->touch = &touchKeyItem;
	
	e->draw = &drawLoopingEntityAnimation;
	
	setEntityAnimation(e, STAND);
}

void generalItemAction()
{
	self->dirY += GRAVITY_SPEED;

	if (self->dirY >= MAX_FALL_SPEED)
	{
		self->dirY = MAX_FALL_SPEED;
	}
	
	self->thinkTime--;
	
	if (self->thinkTime < 90)
	{
		if (self->thinkTime % 3 == 0)
		{
			self->flags ^= NO_DRAW;
		}
	}
	
	if (self->thinkTime == 0)
	{
		self->active = INACTIVE;
	}
	
	checkToMap(self);
	
	self->standingOn = NULL;
}

void healthTouch(Entity *other)
{
	if (other->type == PLAYER)
	{
		other->health += self->health;
		
		if (other->health > other->maxHealth)
		{
			other->health = other->maxHealth;
		}
		
		self->active = INACTIVE;
	}
}

void touchKeyItem(Entity *other)
{
}
