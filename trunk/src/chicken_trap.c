#include "chicken_trap.h"

extern void setEntityAnimation(Entity *, int);
extern void drawLoopingAnimationToMap(void);
extern void loadProperties(char *, Entity *);
extern Entity *getFreeEntity(void);
extern void drawLoopingAnimationToMap(void);
extern int collision(int, int, int, int, int, int, int, int);
extern void keyItemRespawn(void);
extern void playSound(char *, int, int, int);

static void trapWait(void);
static void trapEntity(void);
static void resetTrap(void);
static void activateTrap(void);
static void touch(Entity *);

Entity *addChickenTrap(int x, int y)
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

	return e;
}

static void trapWait()
{
	self->health = self->maxHealth;

	self->thinkTime = 180;
}

static void trapEntity()
{
	setEntityAnimation(self, ATTACK_3);

	if (self->target == NULL)
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
	int i;

	self->thinkTime--;

	self->health = self->maxHealth;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;

		if (self->target == NULL)
		{
			for (i=0;i<MAX_ENTITIES;i++)
			{
				if (entity[i].active == ACTIVE && entity[i].type == ENEMY && strcmpignorecase(entity[i].name, "enemy/chicken") == 0)
				{
					if (collision(self->x, self->y, self->w, self->h, entity[i].x, entity[i].y, entity[i].w, entity[i].h) == 1)
					{
						self->target = &entity[i];

						self->target->flags |= HELPLESS;

						self->target->animationCallback = NULL;

						setEntityAnimation(self->target, STAND);

						self->target->x = self->x + abs(self->target->w - self->w) / 2;
						
						playSound("item/trap_close.wav", -1, self->x, self->y);

						break;
					}
				}
			}
		}

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
