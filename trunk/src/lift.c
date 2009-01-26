#include "lift.h"

extern Entity *getFreeEntity(void);
extern void drawLoopingAnimationToMap(void);
extern void setEntityAnimation(Entity *, int);
extern int collision(int, int, int, int, int, int, int, int);
extern void loadProperties(char *, Entity *);
extern Target *getTargetByName(char *);
extern void playSound(char *, int, int, int, int);

static void touch(Entity *);
static void autoMove(void);
static void wait(void);
static void findTarget(int);
static void wait(void);
static void setToStart(void);
static void moveToTarget(void);

Entity *addLift(char *name, int startX, int startY, char *liftType)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("Couldn't get a free slot for a Lift!\n");

		exit(1);
	}

	loadProperties(name, e);

	if (strcmpignorecase(liftType, "AUTO_LIFT") == 0)
	{
		e->action = &autoMove;
		e->touch = &touch;

		e->type = AUTO_LIFT;
	}

	else if (strcmpignorecase(liftType, "MANUAL_LIFT") == 0)
	{
		e->activate = &findTarget;
		e->touch = &touch;

		e->type = MANUAL_LIFT;
	}

	e->action = &setToStart;

	e->draw = &drawLoopingAnimationToMap;

	e->x = e->startX = startX;
	e->y = e->startY = startY;

	setEntityAnimation(e, STAND);

	return e;
}

static void touch(Entity *other)
{
	/* Test the vertical movement */

	if (other->dirY > 0)
	{
		/* Trying to move down */

		if (collision(other->x, other->y, other->w, other->h, self->x, self->y, self->w, self->h) == 1)
		{
			/* Place the player as close to the solid tile as possible */

			other->y = self->y;
			other->y -= other->h;

			other->standingOn = self;
			other->flags |= ON_GROUND;
			other->dirY = 0;
		}
	}
}

static void findTarget(int val)
{
	char targetName[30];
	Target *t;

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
		if (self->type == AUTO_LIFT)
		{
			self->targetX = (self->targetX == self->endX ? self->startX : self->endX);
			self->targetY = (self->targetY == self->endY ? self->startY : self->endY);

			self->thinkTime = self->health;

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
	if (self->thinkTime > 0)
	{
		self->thinkTime--;
	}

	else
	{
		self->action = &moveToTarget;
	}
}

static void setToStart()
{
	char targetName[30];
	Target *t;

	if (self->type == AUTO_LIFT)
	{
		self->targetX = self->endX;
		self->targetY = self->endY;

		self->health = self->thinkTime;

		self->action = &autoMove;

		self->action();
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

			exit(1);
		}

		self->action = &wait;
	}
}
