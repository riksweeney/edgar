#include "../headers.h"

#include "../decoration.h"
#include "../system/random.h"
#include "../item/item.h"
#include "../entity.h"
#include "../graphics/animation.h"

extern Entity *self;

static void drawGib(void);

void throwGibs(char *name, int gibs)
{
	int i;
	Entity *e;

	for (i=0;i<gibs;i++)
	{
		e = addTemporaryItem(name, self->x, self->y, RIGHT, 0, 0);

		e->x += (self->x - e->x) / 2;
		e->y += (self->y - e->y) / 2;

		e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = -12 - (prand() % 4);

		setEntityAnimation(e, i);

		e->thinkTime = 180 + (prand() % 120);

		e->draw = &drawGib;
	}

	self->inUse = FALSE;
}

static void drawGib()
{
	if (!(self->flags & ON_GROUND))
	{
		if (prand() % 6 == 0)
		{
			addBlood(self->x + self->w / 2, self->y + self->h / 2);
		}
	}
	
	else
	{
		self->frameSpeed = 0;
	}
	
	drawLoopingAnimationToMap();
}
