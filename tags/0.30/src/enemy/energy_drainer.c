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
#include "../collisions.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "enemies.h"
#include "../custom_actions.h"
#include "../hud.h"
#include "../player.h"
#include "../geometry.h"
#include "../projectile.h"

extern Entity *self, player;
extern Game game;

static void lookForPlayer(void);
static void attackWait(void);
static void fireEnergy(void);
static void moveToRecharge(void);
static void recharge(void);
static void init(void);
static void takeDamage(Entity *, int);

Entity *addEnergyDrainer(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add an Energy Drainer\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &drawLoopingAnimationToMap;
	e->die = &entityDieNoDrop;
	e->takeDamage = &takeDamage;
	e->reactToBlock = NULL;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void lookForPlayer()
{
	checkToMap(self);

	if (self->dirX == 0)
	{
		self->dirX = self->face == LEFT ? self->speed : -self->speed;
	}

	if (self->dirX < 0)
	{
		self->face = LEFT;
	}

	else if (self->dirX > 0)
	{
		self->face = RIGHT;
	}

	if (self->maxThinkTime > 0)
	{
		if (player.health > 0 && prand() % 30 == 0)
		{
			if (collision(self->x + (self->face == RIGHT ? self->w : -160), self->y, 160, 200, player.x, player.y, player.w, player.h) == 1)
			{
				self->action = &attackWait;

				setEntityAnimation(self, ATTACK_1);

				self->animationCallback = &fireEnergy;

				self->dirX = 0;
			}
		}
	}
}

static void attackWait()
{
	checkToMap(self);
}

static void fireEnergy()
{
	Entity *e;

	e = addProjectile("enemy/energy_drainer_shot", self, self->x + self->w / 2, self->y + self->h / 2, (self->face == RIGHT ? 7 : -7), 0);

	calculatePath(e->x, e->y, player.x, player.y, &e->dirX, &e->dirY);

	e->dirX *= e->speed;
	e->dirY *= e->speed;

	self->maxThinkTime--;

	if (self->maxThinkTime <= 0)
	{
		self->action = &moveToRecharge;
	}
}

static void moveToRecharge()
{
	if (fabs(self->targetX - self->x) <= fabs(self->dirX) && fabs(self->targetY - self->y) <= fabs(self->dirY))
	{
		self->dirX = 0;
		self->dirY = 0;

		self->thinkTime = 60;

		self->action = &recharge;

		self->takeDamage = &entityTakeDamageNoFlinch;
	}

	else
	{
		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX = self->speed;
		self->dirY = self->speed;
	}

	self->face = self->dirX < 0 ? LEFT : RIGHT;

	checkToMap(self);
}

static void recharge()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->maxThinkTime++;

		if (self->maxThinkTime == 3)
		{
			self->action = &lookForPlayer;

			self->takeDamage = &takeDamage;
		}

		else
		{
			self->thinkTime = 60;
		}
	}

	checkToMap(self);
}

static void init()
{
	Entity *e;

	/* Set target to power generator */

	e = getEntityByObjectiveName(self->requires);

	if (e == NULL)
	{
		printf("Energy Drainer could not find Entity %s\n", self->requires);

		exit(1);
	}

	self->targetX = e->x + e->w / 2;
	self->targetY = e->y + e->h / 2;

	self->targetX -= self->w;
	self->targetY -= self->h;

	self->action = &lookForPlayer;
}

static void takeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
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
			setCustomAction(self, &flashWhite, 6, 0);
			setCustomAction(self, &invulnerableNoFlash, 20, 0);

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
