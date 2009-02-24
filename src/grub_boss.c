#include "headers.h"

#include "animation.h"
#include "properties.h"
#include "entity.h"
#include "collisions.h"
#include "game.h"
#include "audio.h"
#include "music.h"

extern Entity *self, player;

static void wait(void);
static void spit(void);
static void fire(void);
static void pain(void);
static void initialise(void);
static void startSpin(void);
static void spinAttack(void);
static void finishSpin(void);
static void takeDamage(Entity *, int);
static void die(void);

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
		self->action = &spit;

		self->thinkTime = 3;
	}
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
		self->action = &startSpin;
	}
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

	self->touch = &entityTouch;

	self->action = &wait;
}

static void startSpin()
{
	setEntityAnimation(self, ATTACK_2);

	self->animationCallback = &spinAttack;

	self->thinkTime = 60;
}

static void spinAttack()
{
	setEntityAnimation(self, ATTACK_3);

	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			self->face = (player.x > self->x ? RIGHT : LEFT);

			self->dirX = (self->face == RIGHT ? self->speed : -self->speed);
		}
	}

	else if (self->dirX == 0)
	{
		shakeScreen(LIGHT, 5);

		self->face = (player.x > self->x ? RIGHT : LEFT);

		self->frameSpeed *= -1;

		setEntityAnimation(self, ATTACK_2);

		self->animationCallback = &finishSpin;
	}
}

static void finishSpin()
{
	self->frameSpeed *= -1;

	setEntityAnimation(self, STAND);

	self->thinkTime = 180;
}

static void die()
{
	entityDie();
}
