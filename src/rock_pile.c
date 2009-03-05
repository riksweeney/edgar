#include "headers.h"

#include "animation.h"
#include "audio.h"
#include "properties.h"
#include "entity.h"
#include "key_items.h"
#include "item.h"
#include "inventory.h"
#include "rock.h"

extern Entity *self, player;

static void collectRock(Entity *);

Entity *addRockPile(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add rock pile\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;
	e->touch = &collectRock;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	e->thinkTime = 0;

	return e;
}

static void collectRock(Entity *other)
{
	Entity *rock = NULL;

	if (other->type == PLAYER)
	{
		if (self->thinkTime <= 0 && getInventoryItem("Small Rock") == NULL)
		{
			rock = addSmallRock(other->x, other->y);

			strcpy(rock->objectiveName, "Small Rock");

			rock->activate = &throwItem;

			rock->touch = &entityTouch;

			rock->damage = 1;

			addToInventory(rock);

			self->thinkTime = self->maxThinkTime;
		}
	}
}
