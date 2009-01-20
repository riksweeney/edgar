#include "key_items.h"

extern void addToInventory(Entity *);
extern void setCustomAction(Entity *, void (*)(int *), int);

extern Entity *addChickenFeedBag(int, int);
extern Entity *addChickenTrap(int, int);
extern void invulnerable(int *);

static Constructor items[] = {
{"item/chicken_feed_bag", &addChickenFeedBag},
{"item/chicken_trap", &addChickenTrap}
};

static int length = sizeof(items) / sizeof(Constructor);

Entity *addKeyItem(char *name, int x, int y)
{
	int i;

	for (i=0;i<length;i++)
	{
		if (strcmpignorecase(items[i].name, name) == 0)
		{
			printf("Adding Key Item %s to %d %d\n", items[i].name, x, y);

			return items[i].construct(x, y);
		}
	}

	printf("Could not find key item %s\n", name);

	exit(1);
}

void keyItemTouch(Entity *other)
{
	if (!(self->flags & INVULNERABLE) && other->type == PLAYER)
	{
		addToInventory(self);
	}
}

void keyItemRespawn()
{
	self->x = player.x + (player.w - self->w) / 2;
	self->y = player.y + player.h - self->h;

	self->dirY = ITEM_JUMP_HEIGHT;

	setCustomAction(self, &invulnerable, 180);
}
