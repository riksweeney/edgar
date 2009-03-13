#include "../headers.h"

#include "../animation.h"
#include "../properties.h"
#include "../entity.h"
#include "../audio.h"
#include "../custom_actions.h"
#include "../item.h"
#include "../random.h"

extern Entity *self;

static void touch(Entity *);
static void die(void);

Entity *addShrub(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Shrub\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &doNothing;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->die = &die;

	e->type = KEY_ITEM;

	setEntityAnimation(e, STAND);

	return e;
}

static void touch(Entity *other)
{
	pushEntity(other);

	if ((other->flags & ATTACKING) && strcmpignorecase(other->name, self->requires) == 0)
	{
		self->health--;

		setCustomAction(self, &flashWhite, 6);
		setCustomAction(self, &invulnerableNoFlash, 20);

		if (self->health <= 0)
		{
			self->action = &die;
		}
	}
}

static void die()
{
	int i;

	Entity *e = addPermanentItem("misc/chopped_log", self->x, self->y);

	e->x += (self->x - e->x) / 2;
	e->y += (self->y - e->y) / 2;

	e->dirY = ITEM_JUMP_HEIGHT;

	for (i=0;i<8;i++)
	{
		e = addTemporaryItem("misc/small_tree_piece", self->x, self->y, RIGHT, 0, 0);

		e->x += (self->x - e->x) / 2;
		e->y += (self->y - e->y) / 2;

		e->dirX = (prand() % 10) * (prand() % 2 == 0 ? -1 : 1);
		e->dirY = ITEM_JUMP_HEIGHT + (prand() % ITEM_JUMP_HEIGHT);

		setEntityAnimation(e, i);

		e->thinkTime = 60 + (prand() % 60);
	}

	self->inUse = FALSE;
}
