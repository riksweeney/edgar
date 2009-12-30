/*
Copyright (C) 2009-2010 Parallel Realities

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
#include "../collisions.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../custom_actions.h"
#include "../system/error.h"

extern Entity *self;

static void walk(void);
static void wait(void);
static void init(void);
static void changeWalkDirectionStart(void);
static void changeWalkDirection(void);
static void changeWalkDirectionFinish(void);
static void changeHeadStart(void);
static void changeHead(void);
static void changeHeadFinish(void);
static void electrifyStart(void);
static void electrify(void);
static void electrifyFinish(void);
static void createElectricity(void);
static void takeDamage(Entity *, int);
static void doElectricity(void);

Entity *addTortoise(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Tortoise");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &entityDie;
	e->takeDamage = &takeDamage;
	e->reactToBlock = &changeDirection;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void walk()
{
	moveLeftToRight();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirX = 0;

		/* Good Head */

		if (self->startX == 0)
		{
			if (prand() % 5 == 0)
			{
				self->action = &changeHeadStart;
				self->action = &changeWalkDirectionStart;
			}

			else
			{
				self->thinkTime = 60;

				self->action = &electrifyStart;
			}
		}
	}
}

static void changeWalkDirectionStart()
{
	setEntityAnimation(self, CUSTOM_1);

	self->action = &wait;

	self->animationCallback = &changeWalkDirection;

	self->thinkTime = 60;
}

static void changeWalkDirection()
{
	self->thinkTime--;

	self->action = &changeWalkDirection;

	setEntityAnimation(self, CUSTOM_3);

	if (self->thinkTime <= 0)
	{
		self->face = self->face == LEFT ? RIGHT : LEFT;

		self->frameSpeed = -1;

		setEntityAnimation(self, CUSTOM_1);

		self->animationCallback = &changeWalkDirectionFinish;

		self->action = &wait;
	}
}

static void changeWalkDirectionFinish()
{
	self->frameSpeed = 1;

	setEntityAnimation(self, STAND);

	self->action = &walk;

	self->dirX = self->face == LEFT ? -self->speed : self->speed;

	self->thinkTime = 120 + prand() % 120;
}

static void wait()
{
	checkToMap(self);
}

static void changeHeadStart()
{
	setEntityAnimation(self, self->startX == 0 ? CUSTOM_1 : CUSTOM_2);

	self->action = &wait;

	self->animationCallback = &changeHead;

	self->thinkTime = 60;
}

static void changeHead()
{
	self->thinkTime--;

	self->action = &changeHead;

	setEntityAnimation(self, CUSTOM_3);

	if (self->thinkTime <= 0)
	{
		self->startX = self->startX == 0 ? 1 : 0;

		self->frameSpeed = -1;

		setEntityAnimation(self, self->startX == 0 ? CUSTOM_1 : CUSTOM_2);

		self->animationCallback = &changeHeadFinish;

		self->action = &wait;
	}
}

static void changeHeadFinish()
{
	self->frameSpeed = 1;

	setEntityAnimation(self, self->startX == 0 ? STAND : WALK);

	self->action = &walk;

	self->dirX = self->face == LEFT ? -self->speed : self->speed;

	self->thinkTime = 120 + prand() % 120;
}

static void init()
{
	if (strcmpignorecase("enemy/tortoise", self->name) == 0)
	{
		setEntityAnimation(self, STAND);

		self->startX = 0;
	}

	else
	{
		setEntityAnimation(self, WALK);

		self->startX = 1;
	}

	self->action = &walk;

	self->thinkTime = 120 + prand() % 120;
}

static void electrifyStart()
{
	self->dirX = 0;

	self->frameSpeed = 0;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->frameSpeed = 1;

		setEntityAnimation(self, CUSTOM_1);

		self->animationCallback = &createElectricity;
	}

	checkToMap(self);
}

static void createElectricity()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add Tortoise electricity");
	}

	loadProperties("enemy/tortoise_electricity", e);

	playSoundToMap("sound/enemy/tortoise/tortoise_electric.ogg", -1, self->x, self->y, 0);

	setEntityAnimation(e, STAND);

	e->action = &doElectricity;

	e->touch = &entityTouch;

	e->takeDamage = &takeDamage;

	e->draw = &drawLoopingAnimationToMap;

	e->target = self;

	e->face = self->face;

	e->x = self->x;

	e->y = self->y;

	self->target = e;

	self->frameSpeed = 1;

	setEntityAnimation(self, ATTACK_1);

	self->action = &electrify;

	self->thinkTime = 120;
}

static void electrify()
{
	self->thinkTime--;

	self->element = LIGHTNING;

	if (self->thinkTime <= 0)
	{
		self->frameSpeed = -1;

		setEntityAnimation(self, self->startX == 0 ? CUSTOM_1 : CUSTOM_2);

		self->animationCallback = &electrifyFinish;

		self->action = &wait;

		self->element = NO_ELEMENT;

		self->target->inUse = FALSE;
	}

	checkToMap(self);
}

static void electrifyFinish()
{
	setEntityAnimation(self, self->startX == 0 ? STAND : WALK);

	self->frameSpeed = 1;

	self->action = &walk;

	self->dirX = self->face == LEFT ? -self->speed : self->speed;

	self->thinkTime = 120 + prand() % 120;

	checkToMap(self);
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (damage != 0)
	{
		if (self->element == NO_ELEMENT)
		{
			entityTakeDamageNoFlinch(other, damage);
		}

		else
		{
			if (other->type == WEAPON)
			{
				/* Damage the player instead */

				temp = self;

				self = other->parent;

				self->takeDamage(temp, temp->damage);

				self = temp;

				return;
			}

			else if (other->type == PROJECTILE)
			{
				self->health -= damage;

				other->target = self;
			}

			if (self->health > 0)
			{
				setCustomAction(self, &flashWhite, 6, 0, 0);
				setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);

				if (self->pain != NULL)
				{
					self->pain();
				}
			}

			else
			{
				self->damage = 0;

				self->die();
			}
		}
	}
}

static void doElectricity()
{
	self->x = self->target->x + self->target->w / 2 - self->w / 2;
	self->y = self->target->y + self->target->h - self->h;
}
