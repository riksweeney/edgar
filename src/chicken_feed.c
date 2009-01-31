#include "headers.h"

#include "animation.h"
#include "audio.h"
#include "properties.h"
#include "entity.h"
#include "key_items.h"
#include "item.h"

extern Entity *self, player;

static void dropChickenFeed(int);

Entity *addChickenFeedBag(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add chicken feed bag\n");

		exit(1);
	}

	loadProperties("item/chicken_feed_bag", e);

	e->x = x;
	e->y = y;

	e->dirY = ITEM_JUMP_HEIGHT;

	e->type = KEY_ITEM;

	e->face = LEFT;

	e->health = e->thinkTime;

	e->action = &doNothing;
	e->touch = &keyItemTouch;
	e->activate = &dropChickenFeed;
	e->die = &keyItemRespawn;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void dropChickenFeed(int val)
{
	if (self->thinkTime <= 0)
	{
		addTemporaryItem("item/chicken_feed", player.x + (player.face == RIGHT ? player.w : 0), player.y + player.h / 2, player.face, player.face == LEFT ? -10 : 10, ITEM_JUMP_HEIGHT);

		playSound("sound/common/throw.ogg", OBJECT_CHANNEL_1, OBJECT_CHANNEL_2, 0, 0);

		self->thinkTime = self->health;
	}
}
