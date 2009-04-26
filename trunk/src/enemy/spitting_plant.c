#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../collisions.h"
#include "../projectile.h"
#include "../custom_actions.h"

extern Entity *self;

static void wait(void);
static void spit(void);
static void spitFinish(void);

Entity *addSpittingPlant(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Spitting Plant\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &wait;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &entityDie;
	e->pain = NULL;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void wait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->action = &doNothing;

		setEntityAnimation(self, ATTACK_1);

		self->animationCallback = &spit;
	}

	checkToMap(self);
}

static void spit()
{
	Entity *e;
	int x, y;

	x = self->x + self->w / 2;
	y = self->y + 5;

	e = addProjectile("common/green_blob", self, x, y, -6, 0);

	e->flags |= FLY;

	e->reactToBlock = &bounceOffShield;

	e = addProjectile("common/green_blob", self, x, y, -6, -6);

	e->flags |= FLY;

	e->reactToBlock = &bounceOffShield;

	e = addProjectile("common/green_blob", self, x, y, 0, -6);

	e->flags |= FLY;

	e->reactToBlock = &bounceOffShield;

	e = addProjectile("common/green_blob", self, x, y, 6, -6);

	e->flags |= FLY;

	e->reactToBlock = &bounceOffShield;

	e = addProjectile("common/green_blob", self, x, y, 6, 0);

	e->flags |= FLY;

	e->reactToBlock = &bounceOffShield;

	setEntityAnimation(self, ATTACK_2);

	self->animationCallback = &spitFinish;
}


static void spitFinish()
{
	setEntityAnimation(self, STAND);

	self->thinkTime = self->maxThinkTime;

	self->action = &wait;
}
