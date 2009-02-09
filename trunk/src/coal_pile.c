#include "headers.h"

#include "properties.h"
#include "animation.h"
#include "entity.h"
#include "random.h"
#include "decoration.h"
#include "item.h"

extern Entity *self;
extern Entity player;

static void wait(void);
static void touch(Entity *);

Entity *addCoalPile(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add Coal Pile\n");

		exit(1);
	}

	loadProperties("item/coal_pile", e);

	e->x = x;
	e->y = y;

	e->thinkTime = 0;
	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &wait;
	e->touch = &touch;
	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);
	
	return e;
}

static void wait()
{
	if (prand() % 90 == 0)
	{
		addSparkle(self->x + (prand() % self->w), self->y + (prand() % self->h));
	}
}

static void touch(Entity *other)
{
	if (other->parent != NULL && strcmpignorecase(other->name, self->requires) == 0)
	{
		if (prand() % 5 == 0)
		{
			addTemporaryItem("item/coal", self->x + self->w / 2, self->y + self->h / 2, RIGHT, 3, ITEM_JUMP_HEIGHT);
		}
	}
}
