#include "headers.h"

#include "entity.h"
#include "animation.h"
#include "properties.h"
#include "custom_actions.h"
#include "rock.h"

extern Entity *self;

static void touch(Entity *);
static void takeDamage(Entity *, int);
static void die(void);

Entity *addWeakWall(char *name, int x, int y)
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

	e->action = &doNothing;
	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->takeDamage = &takeDamage;
	e->die = die;

	setEntityAnimation(e, STAND);

	return e;
}

static void touch(Entity *other)
{
	if (self->active == TRUE && (other->flags & ATTACKING))
	{
		takeDamage(other, other->damage);
	}

	if (self->inUse == TRUE)
	{
		pushEntity(other);
	}
}

static void takeDamage(Entity *other, int damage)
{
	if (damage > 100)
	{
		self->die();
	}

	if (strcmpignorecase(self->requires, other->name) == 0)
	{
		self->health -= damage;

		if (self->health <= 0)
		{
			self->die();
		}
	}

	else
	{
		printf("Dink from %s\n", other->name);
	}
}

static void die()
{
	Entity *e;

	e = addSmallRock(self->x, self->y, "common/small_rock");

	e->x += (self->w - e->w) / 2;
	e->y += (self->h - e->h) / 2;

	e->dirX = -3;
	e->dirY = -8;

	e = addSmallRock(self->x, self->y, "common/small_rock");

	e->x += (self->w - e->w) / 2;
	e->y += (self->h - e->h) / 2;

	e->dirX = 3;
	e->dirY = -8;

	self->inUse = FALSE;
}
