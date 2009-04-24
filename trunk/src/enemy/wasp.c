#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"

extern Entity *self;

static void die(void);
static void pain(void);
static void init(void);

Entity *addWasp(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Wasp\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &die;
	e->pain = &pain;
	e->takeDamage = &entityTakeDamageFlinch;
	e->reactToBlock = &changeTarget;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void die()
{
	entityDie();
}

static void pain()
{
	playSound("sound/enemy/bat/squeak.wav", ENEMY_CHANNEL_1, ENEMY_CHANNEL_2, self->x, self->y);
}

static void init()
{
	self->targetX = self->endX;

	self->action = &flyToTarget;

	self->action();
}
