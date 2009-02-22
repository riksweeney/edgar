#include "headers.h"

#include "animation.h"
#include "properties.h"
#include "entity.h"
#include "random.h"
#include "collisions.h"
#include "audio.h"

extern Entity *self;

static void die(void);
static void pain(void);

Entity *addBeetle(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Beetle\n");

		exit(1);
	}

	loadProperties("enemy/beetle", e);

	e->x = x;
	e->y = y;

	e->action = &moveLeftToRight;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &die;
	e->pain = &pain;
	e->takeDamage = &entityTakeDamage;

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
