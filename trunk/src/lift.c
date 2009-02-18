#include "headers.h"

#include "animation.h"
#include "audio.h"
#include "properties.h"
#include "target.h"
#include "collisions.h"
#include "entity.h"
#include "hud.h"

extern Entity *self, entity[MAX_ENTITIES];

static void autoMove(void);
static void wait(void);
static void findTarget(int);
static void wait(void);
static void setToStart(void);
static void moveToTarget(void);
static void touch(Entity *);

Entity *addLift(char *name, int startX, int startY, int type)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("Couldn't get a free slot for a Lift!\n");

		exit(1);
	}

	loadProperties(name, e);

	e->type = type;

	if (type == AUTO_LIFT)
	{
		e->action = &autoMove;
	}

	else
	{
		e->activate = &findTarget;
	}

	e->touch = &touch;

	e->action = &setToStart;

	e->draw = &drawLoopingAnimationToMap;

	e->x = e->startX = startX;
	e->y = e->startY = startY;

	setEntityAnimation(e, STAND);

	return e;
}

static void touch(Entity *other)
{
	other->x -= other->dirX;
	other->y -= other->dirY;

	/* Test the horizontal movement */

	if (other->dirY > 0)
	{
		/* Trying to move down */

		if (collision(other->x, other->y + other->dirY, other->w, other->h, self->x, self->y, self->w, self->h) == 1)
		{
			/* Place the player as close to the solid tile as possible */

			other->y = self->y;
			other->y -= other->h;

			other->standingOn = self;
			other->dirY = 0;
			other->flags |= ON_GROUND;
		}
	}

	other->x += other->dirX;
	other->y += other->dirY;
}

static void findTarget(int val)
{
	char targetName[MAX_VALUE_LENGTH];
	Target *t;

	if (self->active == TRUE)
	{
		if (self->action == &moveToTarget)
		{
			return;
		}

		self->health += val;

		if (self->health < 0)
		{
			self->health = 0;
		}

		sprintf(targetName, "%s_TARGET_%d", self->objectiveName, self->health);

		/* Search for the lift's target */

		t = getTargetByName(targetName);

		if (t != NULL)
		{
			if (t->x == (int)self->x && t->y == (int)self->y)
			{
				self->action = &wait;
			}

			else
			{
				self->targetX = t->x;
				self->targetY = t->y;

				self->action = &moveToTarget;

				playSound("sound/common/mine_lift.wav", OBJECT_CHANNEL_1, OBJECT_CHANNEL_2, self->x, self->y);
			}
		}

		else
		{
			printf("Failed to find target\n");

			self->health -= val;
		}
	}

	else
	{
		addHudMessage(BAD_MESSAGE, "This lift is not active");
	}
}

static void moveToTarget()
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
		self->dirX = self->dirY = 0;

		if (self->type == AUTO_LIFT)
		{
			self->targetX = (self->targetX == self->endX ? self->startX : self->endX);
			self->targetY = (self->targetY == self->endY ? self->startY : self->endY);

			self->thinkTime = self->maxThinkTime;

			self->action = &autoMove;
		}

		else
		{
			self->action = &wait;
		}
	}

	else
	{
		self->x += self->dirX;
		self->y += self->dirY;
	}
}

static void wait()
{

}

static void autoMove()
{
	if (self->active == TRUE)
	{
		if (self->thinkTime > 0)
		{
			self->thinkTime--;
		}

		else
		{
			self->action = &moveToTarget;
		}
	}
}

static void setToStart()
{
	char targetName[MAX_VALUE_LENGTH];
	Target *t;

	if (self->type == AUTO_LIFT)
	{
		sprintf(targetName, "%s_START", self->objectiveName);

		/* Search for the lift's target */

		t = getTargetByName(targetName);

		if (t == NULL)
		{
			printf("Could not find target %s for lift %s!\n", targetName, self->objectiveName);

			t = addTarget(self->x, self->y, targetName);
		}

		self->x = t->x;
		self->y = t->y;

		self->startX = self->x;
		self->startY = self->y;

		sprintf(targetName, "%s_END", self->objectiveName);

		/* Search for the lift's target */

		t = getTargetByName(targetName);

		if (t == NULL)
		{
			printf("Could not find target %s for lift %s!\n", targetName, self->objectiveName);

			t = addTarget(self->x, self->y, targetName);
		}

		self->endX = t->x;
		self->endY = t->y;

		self->targetX = self->endX;
		self->targetY = self->endY;

		self->action = &autoMove;

		if (self->active == FALSE)
		{
			self->thinkTime = 0;
		}
	}

	else
	{
		sprintf(targetName, "%s_TARGET_%d", self->objectiveName, self->health);

		/* Search for the lift's target */

		t = getTargetByName(targetName);

		if (t != NULL)
		{
			self->x = t->x;
			self->y = t->y;
		}

		else
		{
			printf("Could not find target %s for lift %s!\n", targetName, self->objectiveName);

			addTarget(self->x, self->y, targetName);
		}

		self->action = &wait;
	}
}
