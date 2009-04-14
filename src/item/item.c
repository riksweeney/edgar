#include "../headers.h"

#include "../graphics/animation.h"
#include "../entity.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../system/properties.h"
#include "../player.h"
#include "../custom_actions.h"
#include "../collisions.h"
#include "../system/random.h"
#include "../inventory.h"

extern Entity *self;
extern Entity player, playerShield, playerWeapon;

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

	else if ((e->flags & PUSHABLE) || (e->flags & OBSTACLE))
	{
		e->touch = &pushEntity;

		e->frameSpeed = 0;
	}

	else
	{
		e->touch = &keyItemTouch;
	}

	setEntityAnimation(e, STAND);

	return e;
}

Entity *addTemporaryItem(char *name, int x, int y, int face, float dirX, float dirY)
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
	
	e->touch = NULL;

	if (e->type == HEALTH)
	{
		e->touch = &healthTouch;
	}

	else
	{
		e->type = TEMP_ITEM;
	}

	setEntityAnimation(e, STAND);

	return e;
}

void dropRandomItem(int x, int y)
{
	if (prand() % 4 == 0)
	{
		addTemporaryItem("item/heart", x, y, RIGHT, 0, ITEM_JUMP_HEIGHT);
	}
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
		self->inUse = FALSE;
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

		self->inUse = FALSE;
	}
}

void dropItem(Entity *e)
{
	if (e->type == SHIELD)
	{
		if (strcmpignorecase(playerShield.name, e->name) == 0)
		{
			playerShield.name[0] = '\0';

			playerShield.inUse = FALSE;
		}
	}

	else if (e->type == WEAPON)
	{
		if (strcmpignorecase(playerWeapon.name, e->name) == 0)
		{
			playerWeapon.name[0] = '\0';

			playerWeapon.inUse = FALSE;
		}
	}

	e->dirY = ITEM_JUMP_HEIGHT;

	e->action = &doNothing;

	setCustomAction(e, &invulnerable, 180);

	addEntity(*e, player.x, player.y);
}

void throwItem(int val)
{
	Entity *e;

	e = addTemporaryItem(self->name, player.x + (player.face == RIGHT ? player.w : 0), player.y + player.h / 2, player.face, player.face == LEFT ? -7 : 7, 0);

	self->inUse = FALSE;

	e->type = PROJECTILE;

	e->flags |= FLY;

	e->touch = self->touch;

	e->damage = self->damage;

	e->parent = &player;

	e->thinkTime = 600;
}
