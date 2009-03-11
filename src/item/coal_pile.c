#include "../headers.h"

#include "../properties.h"
#include "../animation.h"
#include "../entity.h"
#include "../random.h"
#include "../decoration.h"
#include "../inventory.h"
#include "../hud.h"
#include "../item.h"
#include "../trigger.h"
#include "../custom_actions.h"

extern Entity *self;

static void wait(void);
static void touch(Entity *);
static Entity *addCoal(int, int);

Entity *addCoalPile(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add Coal Pile\n");

		exit(1);
	}

	loadProperties(name, e);

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

static Entity *addCoal(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add Coal\n");

		exit(1);
	}

	loadProperties("item/coal", e);

	e->x = x;
	e->y = y;

	e->dirY = ITEM_JUMP_HEIGHT;

	e->thinkTime = 600;
	e->type = ITEM;

	e->face = RIGHT;

	e->action = &generalItemAction;
	e->touch = &addRequiredToInventory;
	e->draw = &drawLoopingAnimationToMap;

	setCustomAction(e, &invulnerableNoFlash, 60);

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
	Entity *e;

	pushEntity(other);

	if ((other->flags & ATTACKING))
	{
		if (strcmpignorecase(other->name, self->requires) == 0 && (prand() % 4) == 0)
		{
			e = addCoal(self->x + self->w / 2, self->y);

			e->y -= e->h + 1;

			e->dirX = (4 + (prand() % 2)) * (prand() % 2 == 0 ? -1 : 1);
		}

		else
		{
			printf("Dink\n");
		}
	}
}
