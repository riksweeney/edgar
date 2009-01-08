#include "bat.h"

extern void drawLoopingAnimationToMap(void);
extern void loadProperties(char *, Entity *);
extern Entity *getFreeEntity(void);
extern void setEntityAnimation(Entity *, int);
extern void checkToMap(Entity *);
extern void setCustomAction(Entity *, void (*)(int *), int);
extern void helpless(int *);
extern void invulnerable(int *);
extern long prand(void);
extern void addStarExplosion(int, int);

static void fly(void);
static void touch(Entity *);
static void takeDamage(Entity *, int);

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
	
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->takeDamage = &takeDamage;
	
	e->type = ENEMY;
	
	setEntityAnimation(e, STAND);
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
				self->dirX = 0.1f + 0.1f * (prand() % 15) * (prand() % 2 == 0 ? 1 : -1);
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
static void touch(Entity *other)
{
	Entity *temp;
	
	if (other->type == PLAYER)
	{
		temp = self;
		
		self = other;
		
		self->takeDamage(temp, 1);
		
		self = temp;
	}
	
	else if (other->type == WEAPON)
	{
		takeDamage(other, other->health);
	}
}

static void takeDamage(Entity *other, int damage)
{
	self->health -= damage;
	
	setCustomAction(self, &helpless, 10);
	setCustomAction(self, &invulnerable, 10);
	
	if (self->health > 0)
	{
		if (self->dirX == 0)
		{
			self->dirX = other->dirX < 0 ? -12 : 12;
		}
		
		else if (other->dirX == 0)
		{
			self->dirX = other->face == RIGHT ? 12 : -12;
		}
		
		else
		{
			self->dirX = self->dirX < 0 ? 12 : -12;
		}
	}
	
	else
	{
		self->active = INACTIVE;
		
		addStarExplosion(self->x, self->y);
	}
}
