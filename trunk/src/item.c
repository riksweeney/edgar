#include "item.h"

extern void checkToMap(Entity *);
extern Entity *getFreeEntity(void);
extern void loadProperties(char *, Entity *);
extern void drawLoopingAnimationToMap(void);
extern void setEntityAnimation(Entity *, int);
extern void doNothing(void);
extern void invulnerable(int *);
extern void setCustomAction(Entity *, void (*)(int *), int);
extern void pushEntity(Entity *);
extern int addToInventory(Entity *);
extern int addEntity(Entity, int, int);
extern void setPlayerWeapon(void);
extern void setPlayerShield(void);

void healthTouch(Entity *);
void generalItemAction(void);
void keyItemTouch(Entity *);

Entity *addPermanentItem(char *name, int x, int y)
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
		e->activate = (e->type == WEAPON ? &setPlayerWeapon : &setPlayerShield);
	}

	else if (e->flags & PUSHABLE)
	{
		e->touch = &pushEntity;
	}

	setEntityAnimation(e, STAND);

	return e;
}

void addTemporaryItem(char *name, int x, int y, int face, float dirX, float dirY)
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

	if (e->thinkTime <= 0)
	{
		printf("No valid thinkTime defined for %s\n", name);

		exit(1);
	}

	e->dirX = dirX;
	e->dirY = dirY;

	e->face = face;

	e->action = &generalItemAction;
	e->draw = &drawLoopingAnimationToMap;

	if (e->type == HEALTH)
	{
		e->touch = &healthTouch;
	}

	setEntityAnimation(e, STAND);
}

void dropRandomItem(int x, int y)
{
	addTemporaryItem("item/heart", x, y, LEFT, 0, ITEM_JUMP_HEIGHT);
}

void generalItemAction()
{
	self->thinkTime--;

	if (self->flags & ON_GROUND)
	{
		self->dirX = 0;
	}

	if (self->thinkTime < 90)
	{
		if (self->thinkTime % 3 == 0)
		{
			self->flags ^= NO_DRAW;
		}
	}

	if (self->thinkTime <= 0)
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

void dropItem(Entity *e)
{
	if (e->type == SHIELD)
	{
		if (strcmpignorecase(playerShield.name, e->name) == 0)
		{
			playerShield.active = INACTIVE;
		}
	}

	else if (e->type == WEAPON)
	{
		if (strcmpignorecase(playerWeapon.name, e->name) == 0)
		{
			playerWeapon.active = INACTIVE;
		}
	}

	e->dirY = ITEM_JUMP_HEIGHT;

	e->action = &doNothing;

	setCustomAction(e, &invulnerable, 180);

	addEntity(*e, player.x, player.y);
}
