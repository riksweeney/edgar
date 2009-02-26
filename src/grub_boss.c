#include "headers.h"

#include "animation.h"
#include "properties.h"
#include "entity.h"
#include "collisions.h"
#include "game.h"
#include "audio.h"
#include "music.h"
#include "map.h"
#include "random.h"

extern Entity *self, player;

static void wait(void);
static void spit(void);
static void fire(void);
static void pain(void);
static void initialise(void);
static void spinAttack(void);
static void finishSpin(void);
static void takeDamage(Entity *, int);
static void die(void);
static void finishSpin(void);
static void attackFinished(void);
static void spinAttackStart(void);

Entity *addGrubBoss(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add the Grub Boss\n");

		exit(1);
	}

	loadProperties("boss/grub_boss", e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;
	e->die = &die;
	e->pain = &pain;
	e->takeDamage = &takeDamage;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void takeDamage(Entity *other, int damage)
{
	entityTakeDamage(other, damage);
}

static void pain()
{

}

static void wait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		switch (prand() % 2)
		{
			case 0:
				self->action = &spit;

				self->thinkTime = 3;
			break;

			default:
				self->action = &spinAttack;
			break;
		}
	}

	checkToMap(self);
}

static void spit()
{
	if (self->thinkTime > 0)
	{
		setEntityAnimation(self, ATTACK_1);

		self->animationCallback = &fire;
	}

	else
	{
		attackFinished();
	}

	checkToMap(self);
}

static void fire()
{
	printf("Firing shot\n");

	self->thinkTime--;
}

static void initialise()
{
	if (self->active == TRUE)
	{
		adjustMusicVolume(-1);
	}

	if (getMinMapX() < self->endX)
	{
		setMinMapX(getMinMapX() + 1);
	}

	else
	{
		self->touch = &entityTouch;

		self->action = &wait;
	}
}

static void startSpin()
{
	setEntityAnimation(self, ATTACK_2);

	self->animationCallback = &spinAttackStart;

	self->thinkTime = 60;
}

static void spinAttackStart()
{
	setEntityAnimation(self, ATTACK_3);

	self->action = &spinAttack;

	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			self->face = (player.x > self->x ? RIGHT : LEFT);

			self->dirY = -8;
		}
	}

	else if (self->thinkTime == 0 && self->flags & ON_GROUND)
	{
		self->dirX = (self->face == RIGHT ? self->speed : -self->speed);

		self->action = &spinAttack;
	}

	checkToMap(self);
}

static void spinAttack()
{
	checkToMap(self);

	if (self->dirX == 0)
	{
		shakeScreen(LIGHT, 5);

		self->face = (player.x > self->x ? RIGHT : LEFT);

		self->frameSpeed *= -1;

		self->dirX = 2;

		self->dirY = -6;

		self->action = &finishSpin;

		self->thinkTime = 0;
	}
}

static void finishSpin()
{
	checkToMap(self);

	if (self->flags & ON_GROUND && self->thinkTime == 0)
	{
		self->frameSpeed *= -1;

		setEntityAnimation(self, ATTACK_2);

		self->animationCallback = &attackFinished;
	}
}

static void attackFinished()
{
	self->thinkTime = 180;

	self->action = &wait;
}

static void die()
{
	setMinMapX(0);
	setMinMapX(0);

	entityDie();
}
