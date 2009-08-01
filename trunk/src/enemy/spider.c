/*
Copyright (C) 2009 Parallel Realities

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

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
static int draw(void);
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
	e->die = &entityDie;

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
	self->weight = 1;

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

				self->weight = (self->targetY == self->endY ? 2 : 3);
			}
		}

		else
		{
			self->y += self->dirY;

			self->weight = (self->dirY > 0 ? 2 : 3);
		}
	}
}

static int draw()
{
	if (self->health > 0)
	{
		drawBoxToMap(self->startX + self->w / 2, self->startY, 1, (self->y - self->startY) + self->h / 2, 255, 255, 255);
	}

	drawLoopingAnimationToMap();
	
	return TRUE;
}

static void init()
{
	if (self->y == self->startY)
	{
		self->targetY = self->endY;
	}

	else
	{
		self->targetY = self->weight == 2 ? self->endY : self->startY;
	}

	self->action = &move;

	move();
}
