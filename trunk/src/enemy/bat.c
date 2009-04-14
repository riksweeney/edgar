#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../collisions.h"

extern Entity *self;

static void fly(void);
static void die(void);
static void pain(void);

Entity *addBat(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Bat\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &fly;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &die;
	e->pain = &pain;
	e->takeDamage = &entityTakeDamage;
	e->reactToBlock = &changeDirection;

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

static void fly()
{
	int speed;

	if (!(self->flags & HELPLESS))
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			switch (prand() % 5)
			{
				case 0:
				case 1:
					self->dirX = self->speed;
				break;

				case 2:
				case 3:
					self->dirX = -self->speed;
				break;

				default:
					self->dirX = 0;
				break;
			}

			self->thinkTime = 180 + prand() % 120;
		}

		if (self->dirX < 0)
		{
			self->face = LEFT;
		}

		else if (self->dirX > 0)
		{
			self->face = RIGHT;
		}
	}

	speed = self->dirX;

	checkToMap(self);

	if (self->dirX == 0 && speed != 0)
	{
		self->dirX = speed * -1;
	}
}
