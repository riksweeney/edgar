#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../system/random.h"
#include "../graphics/graphics.h"
#include "../player.h"
#include "enemies.h"

extern Entity *self;

static void wait(void);
static void init(void);

Entity *addEnemyGenerator(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add an Enemy Generator\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

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

static void init()
{
	if (strlen(self->objectiveName) == 0)
	{
		printf("Enemy Generator at %f %f is not set correctly\n", self->x, self->y);

		exit(1);
	}

	self->action = &wait;

	self->action();
}

static void wait()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		/* Only spawn enemies when the player's close enough */

		if (getDistanceFromPlayer(self) < (SCREEN_WIDTH / 2))
		{
			e = addEnemy(self->objectiveName, self->x, self->y);

			e->x += (self->w - e->w) / 2;
			e->y += (self->h - e->h) / 2;

			e->startX = self->startX;
			e->startY = self->startY;

			e->endX = self->endX;
			e->endY = self->endY;

			e->face = self->face;
			
			self->thinkTime = (rand() % self->maxThinkTime);
		}
	}

	checkToMap(self);
}
