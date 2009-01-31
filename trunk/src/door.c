#include "headers.h"

#include "animation.h"
#include "entity.h"
#include "properties.h"
#include "inventory.h"

extern Entity *self;

static void touch(Entity *);
static void wait(void);
static void setStart(void);
static void moveToTarget(void);
static void pressureMove(int);

Entity *addDoor(char *name, int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Door\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->action = &setStart;
	e->activate = &pressureMove;

	e->type = DOOR;

	setEntityAnimation(e, STAND);

	return e;
}

static void pressureMove(int val)
{
	if (val == 1)
	{
		self->targetX = self->endX;
		self->targetY = self->endY;
	}
	
	else
	{
		self->targetX = self->startX;
		self->targetY = self->startY;
	}
	
	self->action = &moveToTarget;
}

static void setStart()
{
	self->targetX = self->endX;
	self->targetY = self->endY;

	self->action = &wait;
}

static void wait()
{

}

static void touch(Entity *other)
{
	pushEntity(other);

	if (other->type == PLAYER && self->active == INACTIVE)
	{
		/* Look through the player's inventory */

		if (removeInventoryItem(self->objectiveName) == 1)
		{
			self->action = &moveToTarget;

			self->active = ACTIVE;
		}

		else
		{
			printf("%s is needed to open this door\n", self->objectiveName);
		}
	}
}

static void moveToTarget(void)
{	
	if (abs(self->x - self->targetX) > self->speed)
	{
		self->dirX = (self->x < self->targetX ? self->speed : -self->speed);
	}

	else
	{
		self->x = self->targetX;
	}

	if (abs(self->y - self->targetY) > self->speed)
	{
		self->dirY = (self->y < self->targetY ? self->speed : -self->speed);
	}

	else
	{
		self->y = self->targetY;
	}

	if (self->x == self->targetX && self->y == self->targetY)
	{
		self->action = &wait;
		
		self->dirX = 0;
		self->dirY = 0;
	}
	
	else
	{
		self->x += self->dirX;
		self->y += self->dirY;
	}
}
