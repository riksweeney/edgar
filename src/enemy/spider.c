/*
Copyright (C) 2009-2019 Parallel Realities

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
Foundation, 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
*/

#include "../headers.h"

#include "../audio/audio.h"
#include "../custom_actions.h"
#include "../entity.h"
#include "../game.h"
#include "../graphics/animation.h"
#include "../graphics/graphics.h"
#include "../system/error.h"
#include "../system/properties.h"

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
		showErrorAndExit("No free slots to add a Spider");
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

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	if (strcmpignorecase(self->name, "enemy/red_spider") == 0)
	{
		if (self->health > 20)
		{
			setEntityAnimation(self, "PURPLE");
		}

		else if (self->health > 10)
		{
			setEntityAnimation(self, "BLUE");
		}
	}

	if (self->y == self->startY)
	{
		self->targetY = self->endY;
	}

	else
	{
		self->targetY = self->weight == 2 ? self->endY : self->startY;
	}

	self->action = &move;
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE))
	{
		if (damage < self->health)
		{
			self->targetY = self->startY;

			setCustomAction(self, &flashWhite, 6, 0, 0);

			self->action = &retreat;

			setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
		}

		else
		{
			self->flags &= ~FLY;

			self->dirY = ITEM_JUMP_HEIGHT;

			self->damage = 0;

			if (other->type == WEAPON || other->type == PROJECTILE)
			{
				increaseKillCount();
			}

			self->die();
		}

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}
	}
}

static void redTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (!(self->flags & INVULNERABLE))
	{
		if (damage >= self->health)
		{
			self->flags &= ~FLY;

			self->dirY = ITEM_JUMP_HEIGHT;

			self->damage = 0;

			if (other->type == WEAPON || other->type == PROJECTILE)
			{
				increaseKillCount();
			}

			self->die();
		}

		else if (self->maxThinkTime < 0)
		{
			self->targetY = 1800;

			setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
		}

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
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

static void entityWait()
{
	if (self->targetY > 0)
	{
		self->targetY -= 20;

		if (self->targetY < 0)
		{
			self->targetY = 0;
		}

		self->x = self->startX + sin(DEG_TO_RAD(self->targetY)) * 10;
	}
}

static void move()
{
	self->weight = 1;

	self->originalWeight = self->weight;

	if (self->thinkTime > 0)
	{
		self->thinkTime--;

		if (self->thinkTime == 0)
		{
			playSoundToMap("sound/enemy/spider/spider", -1, self->x, self->y, 0);
		}
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
				self->targetY = 0;

				self->action = &entityWait;
			}

			else
			{
				self->thinkTime = self->y == self->endY ? 0 : self->maxThinkTime;

				self->targetY = (self->targetY == self->endY ? self->startY : self->endY);

				self->weight = (self->targetY == self->endY ? 2 : 3);

				self->originalWeight = self->weight;
			}
		}

		else
		{
			self->y += self->dirY;

			self->weight = (self->dirY > 0 ? 2 : 3);

			self->originalWeight = self->weight;
		}
	}
}

static int draw()
{
	if (self->health > 0)
	{
		drawLine(self->startX + self->w / 2, self->startY, self->x + self->w / 2, self->y, 255, 255, 255);
	}

	drawLoopingAnimationToMap();

	return TRUE;
}
