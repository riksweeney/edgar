#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../audio/music.h"
#include "../collisions.h"
#include "../game.h"

extern Entity *self;

static void wait(void);
static void drop(void);
static void touch(Entity *);
static void initialise(void);
static void chasePlayer(void);
static void idle(void);

Entity *addBoulderBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add the Boulder\n");

		exit(1);
	}

	loadProperties("boss/boulder_boss", e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->die = &entityDie;

	e->type = ENEMY;

	e->flags |= NO_DRAW|FLY|ATTACKING;

	e->damage = 500;

	setEntityAnimation(e, STAND);

	return e;
}

static void initialise()
{
	if (self->active == TRUE)
	{
		self->thinkTime--;

		adjustMusicVolume(-1);

		if (self->thinkTime <= 0)
		{
			self->x = self->startX;
			self->y = self->startY;

			self->touch = &touch;

			self->flags &= ~NO_DRAW;
			self->flags &= ~FLY;

			self->action = &drop;
		}
	}

	else
	{
		self->thinkTime = 120;
	}
}

static void drop()
{
	setMusicVolume(-1);

	if (self->flags & ON_GROUND)
	{
		self->thinkTime = 120;

		playSound("sound/boss/boulder_boss/boulder_crash.wav", BOSS_CHANNEL, BOSS_CHANNEL, self->x, self->y);

		shakeScreen(STRONG, self->thinkTime / 2);

		self->action = &wait;
	}

	doNothing();
}

static void wait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, WALK);

		self->endX = -2.0f;

		self->thinkTime = 120;

		self->action = &chasePlayer;
	}
}

static void chasePlayer()
{
	long onGround = self->flags & ON_GROUND;

	self->dirX -= 0.01f;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->thinkTime = 150;

		self->endX -= 0.04f;

		self->frameSpeed++;
	}

	if (self->dirX <= self->endX)
	{
		self->dirX = self->endX;
	}

	checkToMap(self);

	if (onGround == 0 && (self->flags && ON_GROUND))
	{
		shakeScreen(LIGHT, 5);
	}

	if (self->dirX == 0)
	{
		self->action = &idle;

		self->touch = &pushEntity;

		self->frameSpeed = 0;

		self->active = FALSE;

		playSound("sound/boss/boulder_boss/boulder_crash.wav", BOSS_CHANNEL, BOSS_CHANNEL, self->x, self->y);

		shakeScreen(STRONG, 90);

		self->thinkTime = 90;
	}
}

static void idle()
{
	adjustMusicVolume(1);
}

static void touch(Entity *other)
{
	Entity *temp = self;

	if (other->die != NULL)
	{
		self = other;

		self->die();

		self = temp;
	}
}
