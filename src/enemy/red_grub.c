#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../collisions.h"

extern Entity *self, player;

static void die(void);
static void pain(void);
static void reactToBlock(void);
static void lookForPlayer(void);
static void spinAttackStart(void);
static void spinAttack(void);
static void spinAttackEnd(void);

Entity *addRedGrub(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Red Grub\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &lookForPlayer;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &die;
	e->pain = &pain;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &reactToBlock;

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

static void lookForPlayer()
{
	checkToMap(self);

	if (self->dirX == 0 || isAtEdge(self) == TRUE)
	{
		self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

		self->face = (self->face == RIGHT ? LEFT : RIGHT);
	}

	if (prand() % 60 == 0)
	{
		if (collision(self->x + (self->face == RIGHT ? self->w : -320), self->y, 320, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->action = &spinAttackStart;

			self->thinkTime = 60;
		}
	}
}

static void spinAttackStart()
{
	setEntityAnimation(self, ATTACK_2);

	self->flags |= INVULNERABLE;

	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			self->face = (player.x > self->x ? RIGHT : LEFT);

			self->frameSpeed = 2;

			self->dirY = -8;
		}
	}

	else if (self->thinkTime == 0 && self->flags & ON_GROUND)
	{
		self->dirX = (self->face == RIGHT ? self->speed * 4 : -self->speed * 4);

		self->action = &spinAttack;

		self->thinkTime = 180;

		self->flags |= ATTACKING;
	}

	checkToMap(self);
}

static void spinAttack()
{
	float speed = self->dirX;

	self->thinkTime--;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->face = (player.x > self->x ? RIGHT : LEFT);

		self->dirX = speed < 0 ? 3 : -3;

		self->dirY = -6;

		self->action = &spinAttackEnd;

		self->thinkTime = 0;
	}

	else if (self->thinkTime <= 0)
	{
		self->action = &spinAttackEnd;

		self->thinkTime = 0;
	}
}

static void spinAttackEnd()
{
	checkToMap(self);

	if ((self->flags & ON_GROUND) && self->thinkTime == 0)
	{
		self->face = (player.x > self->x ? RIGHT : LEFT);

		setEntityAnimation(self, STAND);

		self->dirX = 0;

		self->flags &= ~(ATTACKING|INVULNERABLE);

		self->action = &lookForPlayer;

		self->frameSpeed = 1;
	}
}

static void reactToBlock()
{
	if (player.face == LEFT)
	{
		self->x = player.x - self->w;
	}

	else
	{
		self->x = player.x + player.w;
	}

	if (self->action == &spinAttack)
	{
		self->dirX = player.face == LEFT ? -5 : 5;

		self->dirY = -6;

		self->action = &spinAttackEnd;

		self->thinkTime = 0;
	}

	else
	{
		changeDirection();
	}
}
