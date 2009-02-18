#include "headers.h"

#include "animation.h"
#include "properties.h"
#include "entity.h"
#include "collisions.h"
#include "audio.h"
#include "graphics.h"

extern Entity *self;

static void move(void);
static void die(void);
static void pain(void);
static void draw(void);
static void init(void);

Entity *addSpider(int x, int y)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Spider\n");

		exit(1);
	}

	loadProperties("enemy/spider", e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &draw;
	e->touch = &entityTouch;
	e->die = &die;
	e->pain = &pain;
	e->takeDamage = &entityTakeDamage;

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

static void move()
{
	if (self->thinkTime > 0)
	{
		self->thinkTime--;
	}

	else
	{
		if (abs(self->x - self->targetX) > self->speed)
		{
			self->dirX = (self->x < self->targetX ? self->speed : -self->speed);
		}

		else
		{
			self->x = self->targetX;
		}

		if (abs(self->y - self->targetY) > self->speed)
		{
			self->dirY = (self->y < self->targetY ? self->speed * 5 : -self->speed);
		}

		else
		{
			self->y = self->targetY;
		}

		if (self->x == self->targetX && self->y == self->targetY)
		{
			self->dirX = self->dirY = 0;

			self->thinkTime = self->y == self->endY ? 0 : self->maxHealth;

			self->targetX = (self->targetX == self->endX ? self->startX : self->endX);
			self->targetY = (self->targetY == self->endY ? self->startY : self->endY);
		}

		else
		{
			self->x += self->dirX;
			self->y += self->dirY;
		}
	}
}

static void draw()
{
	drawBoxToMap(self->startX + self->w / 2, self->startY, 1, (self->y - self->startY) + self->h / 2, 255, 255, 255);

	drawLoopingAnimationToMap();
}

static void init()
{
	self->targetX = self->endX;
	self->targetY = self->endY;

	self->maxHealth = self->thinkTime;

	self->action = &move;

	move();
}
