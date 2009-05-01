#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../audio/audio.h"
#include "../enemy/enemies.h"
#include "../item/item.h"

extern Entity *self, player;

static void wait(void);
static void hatch(void);

Entity *addEgg(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add an Egg\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->startX = x;
	e->startY = y;

	e->action = &wait;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &entityDieNoDrop;
	e->pain = NULL;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	self->thinkTime--;

	if (self->thinkTime <= 60 && self->thinkTime > 0)
	{
		if (self->thinkTime % 4 == 0)
		{
			self->x = self->startX + (2 * (self->x < self->startX ? 1 : -1));
		}
	}

	if (self->thinkTime <= 0)
	{
		self->x = self->startX;

		self->action = &hatch;
	}

	checkToMap(self);
}

static void hatch()
{
	Entity *e;

	e = addTemporaryItem(self->name, self->x, self->y, self->face, 0, 0);

	e->dirX = -3;
	e->dirY = -2;
	e->face = RIGHT;

	setEntityAnimation(e, WALK);

	e = addTemporaryItem(self->name, self->x, self->y, self->face, 0, 0);

	e->dirX = 3;
	e->dirY = -2;
	e->face = RIGHT;

	setEntityAnimation(e, JUMP);

	e = addEnemy(self->objectiveName, 0, 0);

	e->x = self->x + (self->w - e->w) / 2;
	e->y = self->y;

	self->inUse = FALSE;
}
