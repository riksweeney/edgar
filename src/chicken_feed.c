#include "chicken_feed.h"

void addChickenFeed(int x, int y)
{
	Entity *e = getFreeEntity();
	
	if (e == NULL)
	{
		printf("No free slots to add %s\n", name);
		
		exit(1);
	}
	
	loadProperties(name, e);
	
	e->x = x;
	e->y = y;
	
	e->dirY = ITEM_JUMP_HEIGHT;
	
	e->type = ITEM;
	
	e->face = LEFT;
	
	e->action = &doNothing;
	e->touch = NULL;
	e->activate = &dropChickenFeed;
	e->die = &keyItemRespawn;
	
	e->draw = &drawLoopingAnimationToMap;
	
	setEntityAnimation(e, STAND);
}

static void pickup()
{
	self->action = &reload;
}

static void reload()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}
}

void dropChickenFeed()
{
	Entity *player = getPlayer();
	
	if (self->thinkTime <= 0)
	{
		addItem(CHICKE_FEED, player->x + player->h / w, player->y + player->h / 2);
	}
}
