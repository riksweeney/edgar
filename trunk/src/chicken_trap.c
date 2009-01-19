#include "chicken_trap.h"

extern void setEntityAnimation(Entity *, int);
extern void drawLoopingAnimationToMap(void);
extern void loadProperties(char *, Entity *);
extern Entity *getFreeEntity(void);
extern void drawLoopingAnimationToMap(void);
extern int collision(int, int, int, int, int, int, int, int);
extern void keyItemRespawn(void);

static void trapWait(void);
static void trapEntity(void);
static void resetTrap(void);
static void activateTrap(void);
static void touch(Entity *);

void addChickenTrap(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add chicken trap\n");

		exit(1);
	}

	loadProperties("item/chicken_trap", e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = LEFT;

	e->action = &trapWait;
	e->touch = &touch;
	e->die = &keyItemRespawn;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);
}

static void trapWait()
{
	self->health = self->maxHealth;
	
	self->thinkTime = 180;
}

static void trapEntity()
{
	int i, trapped;
	
	trapped = 0;
	
	setEntityAnimation(self, ATTACK_3);
	
	for (i=0;i<MAX_ENTITIES;i++)
	{
		if (entity[i].active == ACTIVE && entity[i].type == ENEMY && strcmpignorecase(entity[i].name, "enemy/chicken") == 0)
		{
			if (collision(self->x, self->y, self->w, self->h, entity[i].x, entity[i].y, entity[i].w, entity[i].h) == 1)
			{
				entity[i].flags |= HELPLESS;
				
				entity[i].animationCallback = NULL;
				
				setEntityAnimation(&entity[i], STAND);
				
				entity[i].x = self->x + abs(entity[i].w - self->w) / 2;
				
				trapped = 1;
				
				break;
			}
		}
	}
	
	if (trapped == 0)
	{
		self->thinkTime = 180;
		
		self->action = &resetTrap;
	}
	
	else
	{
		self->action = &trapWait;
	}
}

static void resetTrap()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
		
		setEntityAnimation(self, ATTACK_2);
		
		self->animationCallback = &trapWait;
	}
}

static void activateTrap()
{
	self->thinkTime--;
	
	self->health = self->maxHealth;
	
	if (self->thinkTime <= 0)
	{	
		self->thinkTime = 0;
		
		setEntityAnimation(self, ATTACK_1);
		
		self->animationCallback = &trapEntity;
	}
}

static void touch(Entity *other)
{
	if (other->type == ENEMY && strcmpignorecase(other->name, "enemy/chicken") == 0)
	{
		self->action = &activateTrap;
		
		self->touch = NULL;
	}
}
