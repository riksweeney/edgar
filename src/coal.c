#include "headers.h"

#include "hud.h"
#include "properties.h"
#include "item.h"
#include "animation.h"
#include "entity.h"
#include "inventory.h"

extern Entity *self;

void addCoal(int x, int y)
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

	e->thinkTime = 300;
	e->type = ITEM;

	e->face = RIGHT;

	e->action = &generalItemAction;
	e->touch = &addRequiredToInventory;
	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);
}
