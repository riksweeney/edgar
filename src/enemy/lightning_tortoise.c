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
#include "../collisions.h"
#include "../custom_actions.h"
#include "../entity.h"
#include "../game.h"
#include "../graphics/animation.h"
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void walk(void);
static void entityWait(void);
static void changeWalkDirectionStart(void);
static void changeWalkDirection(void);
static void changeWalkDirectionFinish(void);
static void electrifyStart(void);
static void electrify(void);
static void electrifyFinish(void);
static void createElectricity(void);
static void takeDamage(Entity *, int);
static void doElectricity(void);
static void creditsMove(void);
static void die(void);

Entity *addLightningTortoise(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Lightning Tortoise");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &walk;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &die;
	e->takeDamage = &takeDamage;
	e->reactToBlock = &changeDirection;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void walk()
{
	moveLeftToRight();

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirX = 0;

		if (prand() % 5 == 0)
		{
			self->action = &changeWalkDirectionStart;
		}

		else
		{
			self->thinkTime = 60;

			self->action = &electrifyStart;
		}
	}
}

static void changeWalkDirectionStart()
{
	setEntityAnimation(self, "CUSTOM_1");

	self->action = &entityWait;

	self->animationCallback = &changeWalkDirection;

	self->thinkTime = 60;

	checkToMap(self);
}

static void changeWalkDirection()
{
	self->thinkTime--;

	self->action = &changeWalkDirection;

	setEntityAnimation(self, "CUSTOM_3");

	if (self->thinkTime <= 0)
	{
		self->face = self->face == LEFT ? RIGHT : LEFT;

		self->frameSpeed = -1;

		setEntityAnimation(self, "CUSTOM_1");

		self->animationCallback = &changeWalkDirectionFinish;

		self->action = &entityWait;
	}

	checkToMap(self);
}

static void changeWalkDirectionFinish()
{
	self->frameSpeed = 1;

	setEntityAnimation(self, "STAND");

	self->action = &walk;

	self->dirX = self->face == LEFT ? -self->speed : self->speed;

	self->thinkTime = 120 + prand() % 120;

	checkToMap(self);
}

static void entityWait()
{
	checkToMap(self);
}

static void electrifyStart()
{
	self->dirX = 0;

	self->frameSpeed = 0;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->frameSpeed = 1;

		setEntityAnimation(self, "CUSTOM_1");

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

	playSoundToMap("sound/enemy/tortoise/tortoise_electric", -1, self->x, self->y, 0);

	setEntityAnimation(e, "STAND");

	e->action = &doElectricity;

	e->creditsAction = &doElectricity;

	e->touch = &entityTouch;

	e->takeDamage = &takeDamage;

	e->draw = &drawLoopingAnimationToMap;

	e->target = self;

	e->face = self->face;

	e->x = self->x;

	e->y = self->y;

	self->target = e;

	self->frameSpeed = 1;

	setEntityAnimation(self, "ATTACK_1");

	self->action = &electrify;

	self->creditsAction = &electrify;

	self->thinkTime = 120;
}

static void electrify()
{
	self->thinkTime--;

	self->element = LIGHTNING;

	if (self->thinkTime <= 0)
	{
		self->frameSpeed = -1;

		setEntityAnimation(self, "CUSTOM_1");

		self->animationCallback = &electrifyFinish;

		self->action = &entityWait;

		self->creditsAction = &entityWait;

		self->element = NO_ELEMENT;

		self->target->inUse = FALSE;
	}

	checkToMap(self);
}

static void electrifyFinish()
{
	setEntityAnimation(self, "STAND");

	self->frameSpeed = 1;

	self->action = &walk;

	self->creditsAction = &creditsMove;

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
			if (other->element == LIGHTNING)
			{
				if (self->flags & INVULNERABLE)
				{
					return;
				}

				if (damage != 0)
				{
					self->health += damage;

					if (other->type == PROJECTILE)
					{
						other->target = self;
					}

					setCustomAction(self, &flashWhite, 6, 0, 0);

					/* Don't make an enemy invulnerable from a projectile hit, allows multiple hits */

					if (other->type != PROJECTILE)
					{
						setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);
					}

					if (self->pain != NULL)
					{
						self->pain();
					}

					if (prand() % 5 == 0)
					{
						setInfoBoxMessage(90, 255, 255, 255, _("The damage from this weapon is being absorbed..."));
					}
				}
			}

			else
			{
				entityTakeDamageNoFlinch(other, damage);
			}
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
				setCustomAction(self, &invulnerableNoFlash, HIT_INVULNERABLE_TIME, 0, 0);

				if (self->pain != NULL)
				{
					self->pain();
				}
			}

			else
			{
				self->damage = 0;

				increaseKillCount();

				self->die();
			}
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

static void die()
{
	playSoundToMap("sound/enemy/tortoise/tortoise_die", -1, self->x, self->y, 0);

	entityDie();
}

static void doElectricity()
{
	if (self->target->health <= 0)
	{
		self->inUse = FALSE;
	}

	self->x = self->target->x + self->target->w / 2 - self->w / 2;
	self->y = self->target->y + self->target->h - self->h;
}

static void creditsMove()
{
	self->mental++;

	setEntityAnimation(self, "STAND");

	self->dirX = self->speed;

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}

	if (self->mental != 0 && (self->mental % 300) == 0)
	{
		self->thinkTime = 60;

		self->creditsAction = &electrifyStart;
	}
}
