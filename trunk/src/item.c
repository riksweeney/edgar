#include "item.h"

extern void checkToMap(Entity *);
extern Entity *getFreeEntity(void);
extern void loadProperties(char *, Entity *);
extern void drawLoopingAnimationToMap(void);
extern void setEntityAnimation(Entity *, int);
extern void doNothing(void);
extern Entity *getPlayer(void);
extern void invulnerable(int *);
extern void setCustomAction(Entity *, void (*)(int *), int);
extern void pushEntity(Entity *);
extern int addToInventory(Entity *);
extern int addEntity(Entity, int, int);
extern Entity *getPlayerShield(void);
extern Entity *getPlayerWeapon(void);

void healthTouch(Entity *);
void generalItemAction(void);
void keyItemTouch(Entity *);

void addPermanentItem(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add item %s\n", name);
		
		exit(1);
	}
	
	loadProperties(name, e);
	
	e->x = x;
	e->y = y;
	
	e->action = &doNothing;
	e->draw = &drawLoopingAnimationToMap;
	
	if (e->type == HEALTH)
	{
		e->touch = &healthTouch;
	}
	
	else if (e->type == WEAPON || e->type == SHIELD)
	{
		e->touch = &keyItemTouch;
	}
	
	else if (e->flags & PUSHABLE)
	{
		e->touch = &pushEntity;
	}
	
	setEntityAnimation(e, STAND);
}

void addTemporaryItem(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add item %s\n", name);
		
		exit(1);
	}
	
	loadProperties(name, e);
	
	e->x = x;
	e->y = y;
	
	e->dirY = ITEM_JUMP_HEIGHT;
	
	e->action = &generalItemAction;
	e->draw = &drawLoopingAnimationToMap;
	
	if (e->type == HEALTH)
	{
		e->touch = &healthTouch;
	}
	
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

void keyItemTouch(Entity *other)
{
	if (other->type == PLAYER)
	{
		addToInventory(self);
	}
}

void dropItem(Entity *e)
{
	Entity *player = getPlayer();
	Entity *w;
	
	if (e->type == SHIELD)
	{
		w = getPlayerShield();
		
		if (strcmpignorecase(w->name, e->name) == 0)
		{
			w->active = 0;
		}
	}
	
	else if (e->type == WEAPON)
	{
		w = getPlayerWeapon();
		
		if (strcmpignorecase(w->name, e->name) == 0)
		{
			w->active = 0;
		}
	}
	
	e->dirY = ITEM_JUMP_HEIGHT;
	
	e->flags |= INVULNERABLE;
	
	e->action = &doNothing;
	
	setCustomAction(e, &invulnerable, 180);
	
	addEntity(*e, player->x, player->y);
}

void keyItemRespawn()
{
	Entity *player = getPlayer();
	
	self->x = player->x + (player->w - self->w) / 2;
	self->y = player->y + player->h - self->h;
	
	self->dirY = ITEM_JUMP_HEIGHT;
	
	self->flags |= INVULNERABLE;
	
	setCustomAction(self, &invulnerable, 180);
}
