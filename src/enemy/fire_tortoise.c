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
#include "../graphics/decoration.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../collisions.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../custom_actions.h"
#include "../system/error.h"
#include "../game.h"
#include "../hud.h"
#include "../projectile.h"

extern Entity *self, player;

static void walk(void);
static void entityWait(void);
static void takeDamage(Entity *, int);
static void changeWalkDirectionStart(void);
static void changeWalkDirection(void);
static void changeWalkDirectionFinish(void);
static void breatheFireInit(void);
static void breatheFireWait(void);

Entity *addFireTortoise(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Fire Tortoise");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &walk;

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
	
	if (player.health > 0 && prand() % 60 == 0)
	{
		if (collision(self->x + (self->face == RIGHT ? self->w : -320), self->y, 320, self->h, player.x, player.y, player.w, player.h) == 1)
		{
			self->action = &breatheFireInit;

			self->dirX = 0;
		}
	}

	else if (self->thinkTime <= 0)
	{
		self->dirX = 0;

		self->action = &changeWalkDirectionStart;
	}
}

static void changeWalkDirectionStart()
{
	setEntityAnimation(self, CUSTOM_1);

	self->action = &entityWait;

	self->animationCallback = &changeWalkDirection;

	self->thinkTime = 60;
	
	checkToMap(self);
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

		self->action = &entityWait;
	}
	
	checkToMap(self);
}

static void changeWalkDirectionFinish()
{
	self->frameSpeed = 1;

	setEntityAnimation(self, STAND);

	self->action = &walk;

	self->dirX = self->face == LEFT ? -self->speed : self->speed;

	self->thinkTime = 300 + prand() % 180;
	
	checkToMap(self);
}

static void breatheFireInit()
{
	Entity *e;
	
	setEntityAnimation(self, ATTACK_1);

	e = addProjectile("enemy/fireball", self, 0, 0, (self->face == LEFT ? -6 : 6), 0);
	
	if (self->face == LEFT)
	{
		e->x = self->x + self->w - e->w - self->offsetX;
	}

	else
	{
		e->x = self->x + self->offsetX;
	}
	
	e->y = self->y + self->offsetY;

	e->flags |= FLY;
	
	playSoundToMap("sound/enemy/fireball/fireball.ogg", -1, self->x, self->y, 0);
	
	self->thinkTime = 30;
	
	self->action = &breatheFireWait;
	
	checkToMap(self);
}

static void breatheFireWait()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		setEntityAnimation(self, STAND);
		
		self->thinkTime = 300 + prand() % 180;
		
		self->dirX = self->face == LEFT ? -self->speed : self->speed;
		
		self->action = &walk;
	}
	
	checkToMap(self);
}

static void entityWait()
{
	checkToMap(self);
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;
	
	if (damage != 0)
	{
		if (other->element == FIRE)
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
					setCustomAction(self, &invulnerableNoFlash, 20, 0, 0);
				}

				if (self->pain != NULL)
				{
					self->pain();
				}
				
				if (prand() % 5 == 0)
				{
					setInfoBoxMessage(90, 255, 255, 255, _("The damage from this weapon is being absorbed..."));
				}
				
				if (other->type == PROJECTILE)
				{
					temp = self;

					self = other;

					self->die();

					self = temp;
				}
				
				addDamageScore(-damage, self);
			}
		}
		
		else
		{
			entityTakeDamageNoFlinch(other, damage);
		}
	}
}
