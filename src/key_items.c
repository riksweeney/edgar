#include "headers.h"

#include "chicken_feed.h"
#include "chicken_trap.h"
#include "coal_pile.h"
#include "custom_actions.h"
#include "inventory.h"
#include "rock_pile.h"
#include "shrub.h"

extern Entity *self, player;

static Constructor items[] = {
{"item/chicken_feed_bag", &addChickenFeedBag},
{"item/chicken_trap", &addChickenTrap},
{"item/coal_pile", &addCoalPile},
{"item/rock_pile", &addRockPile},
{"misc/small_tree", &addShrub}
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

			return items[i].construct(x, y, name);
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
