#include "../headers.h"

#include "../graphics/animation.h"
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../custom_actions.h"
#include "../collisions.h"
#include "key_items.h"
#include "../system/random.h"
#include "../world/explosion.h"

extern Entity *self, player;

static void dropBomb(int);
static void wait(void);
static void explode(void);

Entity *addBomb(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Bomb\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &wait;
	e->touch = &keyItemTouch;
	e->activate = &dropBomb;

	e->draw = &drawLoopingAnimationToMap;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	checkToMap(self);
}

static void dropBomb(int val)
{
	self->thinkTime = 0;

	self->touch = NULL;
	
	setEntityAnimation(self, WALK);

	self->animationCallback = &explode;

	self->active = TRUE;
	
	self->health = 10;

	addEntity(*self, player.x, player.y);

	self->inUse = FALSE;
}

static void explode()
{
	int x, y;
	
	self->flags |= NO_DRAW|FLY;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		x = self->x;
		y = self->y;
		
		x += (prand() % 20) * (prand() % 2 == 0 ? 1 : -1);
		y += (prand() % 20) * (prand() % 2 == 0 ? 1 : -1);
		
		addExplosion(x, y);

		self->health--;

		self->thinkTime = 15;

		if (self->health == 0)
		{
			self->inUse = FALSE;
		}
	}
	
	self->action = &explode;
}
