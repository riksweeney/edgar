#include "../headers.h"

#include "../graphics/animation.h"
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "key_items.h"
#include "item.h"
#include "../inventory.h"
#include "../item/bomb.h"

extern Entity *self, player;

static void collectBomb(Entity *);

Entity *addBombPile(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add Bomb Pile\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &doNothing;
	e->touch = &collectBomb;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	e->thinkTime = 0;

	return e;
}

static void collectBomb(Entity *other)
{
	Entity *bomb = NULL;

	if (other->type == PLAYER)
	{
		if (self->thinkTime <= 0 && getInventoryItem(_("Bomb")) == NULL)
		{
			bomb = addBomb(other->x, other->y, "item/bomb");

			STRNCPY(bomb->objectiveName, _("Bomb"), sizeof(bomb->objectiveName));

			addToInventory(bomb);

			self->thinkTime = self->maxThinkTime;
		}
	}
}
