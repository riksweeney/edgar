#include "chicken_feed.h"

extern void setEntityAnimation(Entity *, int);
extern void drawLoopingAnimationToMap(void);
extern void loadProperties(char *, Entity *);
extern Entity *getFreeEntity(void);
extern void doNothing(void);
extern void keyItemRespawn(void);
extern void drawLoopingAnimationToMap(void);
extern void keyItemTouch(Entity *);
extern void addTemporaryItem(char *, int, int);

static void dropChickenFeed(void);

void addChickenFeedBag(int x, int y)
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
}

void dropChickenFeed()
{
	if (self->thinkTime <= 0)
	{
		addTemporaryItem("item/chicken_feed", player.x + player.h / 2, player.y + player.h / 2);

		self->thinkTime = self->health;
	}
}
