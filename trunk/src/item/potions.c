#include "../headers.h"

#include "../system/properties.h"
#include "../graphics/animation.h"
#include "../entity.h"
#include "../player.h"
#include "../item/item.h"
#include "../item/key_items.h"

extern Entity *self, player;

static void useHealthPotion(int);

Entity *addHealthPotion(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Health Potion\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->thinkTime = 0;
	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;
	e->touch = &keyItemTouch;
	e->draw = &drawLoopingAnimationToMap;
	e->activate = &useHealthPotion;

	setEntityAnimation(e, STAND);

	return e;
}

static void useHealthPotion(int val)
{
	if (player.health != player.maxHealth)
	{
		player.health = player.maxHealth;

		self->inUse = FALSE;
	}
}
