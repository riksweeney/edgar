#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../collisions.h"
#include "../projectile.h"

extern Entity *self, player;

static void wait(void);
static void attack(void);

Entity *addJumpingPlant(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Jumping Plant\n");

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
	if (prand() % 10 == 0)
	{
		if (collision(self->x - 150, self->y, 300 + self->w, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->action = &attack;
		}
	}

	checkToMap(self);
}

static void attack()
{
	if (self->flags & ON_GROUND)
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			self->dirY = -11;

			self->dirX = player.x < self->x ? -self->speed : self->speed;
		}
	}

	checkToMap(self);

	if ((self->flags & ON_GROUND) && self->thinkTime == 0)
	{
		self->thinkTime = self->maxThinkTime;
	}
}
