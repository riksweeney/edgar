#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../map.h"
#include "../audio/music.h"
#include "../event/trigger.h"
#include "../item/key_items.h"
#include "../collisions.h"

extern Entity *self, player;

static void initialise(void);
static void doIntro(void);
static void die(void);
static void takeDamage(Entity *, int);

Entity *addGolemBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add the Golem Boss\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;
	e->takeDamage = &takeDamage;
	e->die = &die;

	e->type = ENEMY;

	e->flags |= NO_DRAW|FLY;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void initialise()
{
	int minX, minY;

	minX = getMapStartX();
	minY = getMapStartY();
	
	if (self->active == TRUE)
	{
		adjustMusicVolume(-1);

		if (minX == self->endX && minY == self->endY)
		{
			centerMapOnEntity(&player);

			self->dirX = self->speed;

			/*setEntityAnimation(self, ATTACK_2);*/

			self->action = &doIntro;

			self->flags &= ~NO_DRAW;
			self->flags &= ~FLY;

			self->thinkTime = 300;

			printf("Starting\n");
		}
	}
}

static void doIntro()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		die();
	}
}

static void die()
{
	Entity *e;

	self->thinkTime--;

	self->takeDamage = NULL;

	printf("Dying %d\n", self->thinkTime);

	if (self->thinkTime <= 0)
	{
		setMinMapX(0);
		setMinMapX(0);

		fireTrigger(self->objectiveName);

		e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

		e->dirY = ITEM_JUMP_HEIGHT;

		self->inUse = FALSE;
	}

	checkToMap(self);
}

static void takeDamage(Entity *other, int damage)
{

}
