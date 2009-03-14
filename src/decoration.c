#include "headers.h"

#include "decoration.h"
#include "graphics/animation.h"
#include "system/properties.h"

static Entity decoration[MAX_DECORATIONS];
extern Entity *self;

static void move(void);
static void wait(void);
static void finish(void);
static void timeout(void);

void freeDecorations()
{
	/* Clear the list */

	memset(decoration, 0, sizeof(Entity) * MAX_DECORATIONS);
}

Entity *getFreeDecoration()
{
	int i;

	/* Loop through all the Decorations and find a free slot */

	for (i=0;i<MAX_DECORATIONS;i++)
	{
		if (decoration[i].inUse == FALSE)
		{
			memset(&decoration[i], 0, sizeof(Entity));

			decoration[i].inUse = TRUE;

			decoration[i].active = TRUE;

			decoration[i].frameSpeed = 1;

			return &decoration[i];
		}
	}

	/* Return NULL if you couldn't any free slots */

	return NULL;
}

void doDecorations()
{
	int i;

	/* Loop through the Decorations and perform their action */

	for (i=0;i<MAX_DECORATIONS;i++)
	{
		self = &decoration[i];

		if (self->inUse == TRUE)
		{
			self->action();
		}
	}
}

void drawDecorations()
{
	int i;

	/* Loop through the Decorations and perform their action */

	for (i=0;i<MAX_DECORATIONS;i++)
	{
		self = &decoration[i];

		if (self->inUse == TRUE)
		{
			self->draw();
		}
	}
}

void addStarExplosion(int x, int y)
{
	int i;
	Entity *e;

	for (i=0;i<8;i++)
	{
		e = getFreeDecoration();

		if (e == NULL)
		{
			return;
		}

		loadProperties("decoration/star", e);

		switch (i)
		{
			case 0:
				e->dirX = -3;
				e->dirY = -3;
			break;

			case 1:
				e->dirX = 3;
				e->dirY = -3;
			break;

			case 2:
				e->dirX = 3;
				e->dirY = 3;
			break;

			case 3:
				e->dirX = 0;
				e->dirY = 3;
			break;

			case 4:
				e->dirX = 0;
				e->dirY = -3;
			break;

			case 5:
				e->dirX = 3;
				e->dirY = 0;
			break;

			default:
				e->dirX = -3;
				e->dirY = 0;
			break;
		}

		e->thinkTime = 15;

		e->x = x;
		e->y = y;

		e->action = &move;
		e->draw = &drawLoopingAnimationToMap;
	}
}

void addSparkle(int x, int y)
{
	Entity *e = getFreeDecoration();

	if (e == NULL)
	{
		return;
	}

	loadProperties("decoration/sparkle", e);

	e->thinkTime = 15;

	e->x = x;
	e->y = y;

	e->action = &wait;
	e->draw = &drawLoopingAnimationToMap;
	e->animationCallback = &finish;
}

Entity *addTrail(int x, int y, char *name, int thinkTime)
{
	Entity *e = getFreeDecoration();

	if (e == NULL)
	{
		return NULL;
	}

	loadProperties(name, e);

	e->thinkTime = thinkTime;

	e->x = x;
	e->y = y;

	e->action = &timeout;
	e->draw = &drawLoopingAnimationToMap;

	return e;
}

static void finish()
{
	self->inUse = FALSE;
}

static void timeout()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		finish();
	}
}

static void wait()
{

}

static void move()
{
	self->x += self->dirX;
	self->y += self->dirY;

	self->thinkTime--;

	if (self->thinkTime == 0)
	{
		self->inUse = FALSE;
	}
}
