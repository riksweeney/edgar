#include "headers.h"

#include "graphics/animation.h"
#include "system/properties.h"
#include "entity.h"
#include "collisions.h"

static void projectileMove(void);

extern Entity *self;

Entity *addProjectile(char *name, Entity *owner, int x, int y, float dirX, float dirY)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a projectile\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->dirX = dirX;
	e->dirY = dirY;

	e->action = &projectileMove;

	e->draw = &drawLoopingAnimationToMap;

	e->touch = &entityTouch;

	e->type = PROJECTILE;

	e->parent = owner;

	setEntityAnimation(e, STAND);

	return e;
}

static void projectileMove()
{
	self->face = self->dirX > 0 ? RIGHT : LEFT;
	
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->inUse = FALSE;
	}

	checkToMap(self);
}

void bounceOffShield()
{
	self->dirX *= -1;

	self->flags &= ~FLY;

	self->touch = NULL;
}
