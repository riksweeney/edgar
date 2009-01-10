#include "decoration.h"

extern void drawLoopingAnimationToMap(void);
extern void loadProperties(char *, Entity *);

static void move(void);

void clearDecorations()
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
		if (decoration[i].active == INACTIVE)
		{
			memset(&decoration[i], 0, sizeof(Entity));

			decoration[i].active = ACTIVE;

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

		if (self->active == ACTIVE)
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

		if (self->active == ACTIVE)
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

static void move()
{
	self->x += self->dirX;
	self->y += self->dirY;
	
	self->thinkTime--;
	
	if (self->thinkTime == 0)
	{
		self->active = INACTIVE;
	}
}
