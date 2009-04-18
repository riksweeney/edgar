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
static void takeDamage(Entity *, int);
static void die(void);
static void explode(void);

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
	e->die = &die;
	e->pain = NULL;
	e->takeDamage = &takeDamage;
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
		setEntityAnimation(self, ATTACK_1);

		self->animationCallback = &spit;
	}

	self->thinkTime = self->maxThinkTime;
}

static void spit()
{
	Entity *e;

	e = addProjectile("common/green_blob", self, self->x, self->y, -2, 0);

	e->flags |= FLY;

	e = addProjectile("common/green_blob", self, self->x, self->y, -2, -2);

	e->flags |= FLY;

	e = addProjectile("common/green_blob", self, self->x, self->y, 0, -2);

	e->flags |= FLY;

	e = addProjectile("common/green_blob", self, self->x, self->y, 2, -2);

	e->flags |= FLY;

	e = addProjectile("common/green_blob", self, self->x, self->y, 2, 0);

	e->flags |= FLY;

	setEntityAnimation(self, STAND);
}

static void takeDamage(Entity *other, int damage)
{
	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

		setCustomAction(self, &helpless, 10);
		setCustomAction(self, &invulnerable, 15);

		if (self->health <= 0)
		{
			self->damage = 0;

			self->die();
		}
	}
}

static void die()
{
	if (prand() % 5 == 0)
	{
		self->touch = NULL;

		setEntityAnimation(self, DIE);

		self->animationCallback = &explode;
	}

	else
	{
		entityDie();
	}
}

static void explode()
{
	Entity *e;

	e = addProjectile("common/green_blob", self, self->x, self->y, -2, -6);

	e->weight = 0.5;

	e = addProjectile("common/green_blob", self, self->x, self->y, -2, -8);

	e->weight = 0.5;

	e = addProjectile("common/green_blob", self, self->x, self->y, 2, 6);

	e->weight = 0.5;

	e = addProjectile("common/green_blob", self, self->x, self->y, 2, 8);

	e->weight = 0.5;

	entityDie();
}
