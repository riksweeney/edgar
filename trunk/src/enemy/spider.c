#include "../headers.h"

#include "../graphics/animation.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../graphics/graphics.h"
#include "../custom_actions.h"

extern Entity *self;

static void move(void);
static void die(void);
static void draw(void);
static void init(void);
static void takeDamage(Entity *, int);
static void redTakeDamage(Entity *, int);
static void retreat(void);

Entity *addSpider(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Spider\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &draw;
	e->touch = &entityTouch;
	e->die = &die;
	
	if (strcmpignorecase(name, "enemy/red_spider") == 0)
	{
		e->takeDamage = &redTakeDamage;
	}
	
	else
	{
		e->takeDamage = &takeDamage;
	}

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void die()
{
	entityDie();
}

static void takeDamage(Entity *other, int damage)
{
	if (!(self->flags & INVULNERABLE))
	{
		if (damage < self->health)
		{
			self->targetY = self->startY;

			setCustomAction(self, &flashWhite, 6, 0);
			setCustomAction(self, &invulnerableNoFlash, 20, 0);

			self->action = &retreat;
		}

		else
		{
			self->health -= damage;

			self->action = self->die;
		}
	}
}

static void redTakeDamage(Entity *other, int damage)
{
	if (!(self->flags & INVULNERABLE))
	{
		if (damage >= self->health)
		{
			self->health -= damage;

			self->action = self->die;
		}
	}
}

static void retreat()
{
	if (abs(self->y - self->targetY) <= self->speed * 3)
	{
		self->y = self->targetY;
	}

	if (self->y == self->targetY)
	{
		self->dirX = self->dirY = 0;

		self->thinkTime = 600;

		self->targetY = self->endY;

		self->action = &move;
	}

	else
	{
		self->y -= self->speed * 3;
	}
}

static void wait()
{

}

static void move()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;
	}

	else
	{
		if (abs(self->y - self->targetY) > self->speed)
		{
			self->dirY = (self->y < self->targetY ? self->speed * 5 : -self->speed);
		}

		else
		{
			self->y = self->targetY;
		}

		if (self->y == self->targetY)
		{
			self->dirY = 0;

			if (self->maxThinkTime < 0 && self->endY == self->targetY)
			{
				self->action = &wait;
			}

			else
			{
				self->thinkTime = self->y == self->endY ? 0 : self->maxThinkTime;

				self->targetY = (self->targetY == self->endY ? self->startY : self->endY);
			}
		}

		else
		{
			self->y += self->dirY;
		}
	}
}

static void draw()
{
	if (self->health > 0)
	{
		drawBoxToMap(self->startX + self->w / 2, self->startY, 1, (self->y - self->startY) + self->h / 2, 255, 255, 255);
	}

	drawLoopingAnimationToMap();
}

static void init()
{
	self->targetY = self->endY;

	self->action = &move;

	move();
}
