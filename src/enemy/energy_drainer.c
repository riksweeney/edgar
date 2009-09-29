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
#include "../game.h"
#include "../player.h"
#include "../geometry.h"
#include "../projectile.h"
#include "../graphics/decoration.h"

extern Entity *self, player;
extern Game game;

static void lookForPlayer(void);
static void attackWait(void);
static void fireEnergy(void);
static void moveToRecharge(void);
static void recharge(void);
static void init(void);
static void takeDamage(Entity *, int);
static void die(void);
static void ballWait(void);
static void returnToGenerator(void);
static void hover(void);
static void createSpark(void);
static void sparkWait(void);
static void stunned(void);
static void stunFinish(void);
static void groundShockStart(void);
static void groundShock(void);
static void groundShockFinish(void);

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
	e->die = &die;
	e->takeDamage = &takeDamage;
	e->reactToBlock = NULL;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void lookForPlayer()
{
	int distance;

	checkToMap(self);

	/* Don't stray too far from the generator */

	distance = getDistance(self->x, self->y, self->targetX, self->targetY);

	if (self->dirX == 0 || distance > 480)
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

	if (self->mental > 0)
	{
		if (player.health > 0 && prand() % 5 == 0)
		{
			if (collision(self->x + (self->face == RIGHT ? self->w : -160), self->y - 64, 160, 128, player.x, player.y, player.w, player.h) == 1)
			{
				self->action = prand() % 2 == 0 ? &fireEnergy : &groundShockStart;

				self->dirX = 0;

				self->thinkTime = 30;
			}
		}
	}

	hover();
}

static void attackWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->mental <= 0)
		{
			self->action = &moveToRecharge;
		}

		else
		{
			self->action = &lookForPlayer;
		}
	}

	checkToMap(self);

	hover();
}

static void fireEnergy()
{
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = addProjectile("common/green_blob", self, self->x, self->y, (self->face == LEFT ? -6 : 6), 0);

		e->x += self->w / 2;
		e->y += self->h / 2;

		e->x -= e->w / 2;
		e->y -= e->h / 2;

		calculatePath(e->x, e->y, player.x - player.w / 2, player.y + player.h / 2, &e->dirX, &e->dirY);

		e->flags |= FLY;

		e->dirX *= 8;
		e->dirY *= 8;

		self->mental--;

		self->thinkTime = 30;

		self->action = &attackWait;

		if (self->mental <= 0)
		{
			self->touch = &entityTouch;
		}
	}

	hover();
}

static void moveToRecharge()
{
	if (fabs(self->targetX - self->x) <= fabs(self->dirX) && fabs(self->targetY - self->y) <= fabs(self->dirY))
	{
		self->dirX = 0;
		self->dirY = 0;

		self->x = self->targetX;
		self->y = self->targetY;

		self->startY = self->y;

		self->startX = 0;

		self->thinkTime = 60;

		self->action = &recharge;

		self->takeDamage = &entityTakeDamageNoFlinch;
	}

	else
	{
		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= self->speed;
		self->dirY *= self->speed;
	}

	self->face = self->dirX < 0 ? LEFT : RIGHT;

	checkToMap(self);
}

static void recharge()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->mental++;

		if (self->mental == 5)
		{
			self->action = &lookForPlayer;

			self->takeDamage = &takeDamage;

			createSpark();
		}

		else
		{
			self->thinkTime = 60;
		}
	}

	checkToMap(self);

	hover();
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

	self->targetX -= self->w / 2;
	self->targetY -= self->h / 2;

	self->target = e;

	self->y = self->startY;

	if (self->mental > 0)
	{
		self->action = &lookForPlayer;

		createSpark();
	}

	else
	{
		self->action = &moveToRecharge;
	}
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

			/* Don't become stunned if you have no mental, init? */

			if (self->mental > 0)
			{
				self->dirX = 0;

				self->action = &stunned;

				self->touch = &entityTouch;

				self->flags &= ~FLY;

				self->maxThinkTime = self->mental;

				self->mental = 0;

				self->thinkTime = 180;
			}
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

static void die()
{
	Entity *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add an Energy Drainer Ball\n");

		exit(1);
	}

	loadProperties("enemy/energy_drainer_ball", e);

	e->x = self->x + self->w / 2;
	e->y = self->y + self->h / 2;

	e->startX = e->x;
	e->startY = e->y;

	e->targetX = self->targetX;
	e->targetY = self->targetY;

	e->target = self->target;

	e->x -= e->w / 2;
	e->y -= e->h / 2;

	e->action = &ballWait;
	e->draw = &drawLoopingAnimationToMap;
	e->die = NULL;
	e->touch = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	e->thinkTime = 120;

	self->mental = 0;

	entityDie();
}

static void ballWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->y = self->startY;

		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= self->speed;
		self->dirY *= self->speed;

		self->action = &returnToGenerator;
	}

	else
	{
		self->endY++;

		self->y = self->startY + cos(DEG_TO_RAD(self->endY)) * 16;
	}
}

static void returnToGenerator()
{
	self->x += self->dirX;
	self->y += self->dirY;

	if (fabs(self->x - self->targetX) <= fabs(self->dirX) && fabs(self->y - self->targetY) <= fabs(self->dirY))
	{
		self->target->frameSpeed = 1;

		self->inUse = FALSE;
	}
}

static void hover()
{
	self->startX++;

	self->y = self->startY + cos(DEG_TO_RAD(self->startX)) * 8;
}

static void createSpark()
{
	Entity *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add an Energy Drainer Spark\n");

		exit(1);
	}

	loadProperties("enemy/energy_drainer_spark", e);

	e->action = &sparkWait;
	e->draw = &drawLoopingAnimationToMap;
	e->takeDamage = NULL;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	e->head = self;

	e->x = e->head->x + e->head->w / 2;
	e->y = e->head->y + e->head->h / 2;

	e->x -= e->w / 2;
	e->y -= e->h / 2;
}

static void sparkWait()
{
	if (self->head->mental <= 0)
	{
		self->inUse = FALSE;
	}

	else
	{
		self->x = self->head->x + self->head->w / 2;
		self->y = self->head->y + self->head->h / 2;

		self->x -= self->w / 2;
		self->y -= self->h / 2;
	}
}

static void stunned()
{
	int i;
	long onGround = self->flags & ON_GROUND;
	Entity *e;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		if (onGround == 0)
		{
			for (i=0;i<15;i++)
			{
				e = addSmoke(self->x + (prand() % self->w), self->y + self->h, "decoration/dust");

				if (e != NULL)
				{
					e->y -= prand() % e->h;
				}
			}
		}

		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->action = &stunFinish;

			self->flags |= FLY;

			self->dirY = -2.5;

			self->thinkTime = 60;
		}
	}
}

static void stunFinish()
{
	checkToMap(self);

	if (self->y < self->startY)
	{
		self->thinkTime--;

		self->y = self->startY;

		self->dirY = 0;

		if (self->thinkTime <= 0)
		{
			self->mental = self->maxThinkTime;

			createSpark();

			self->action = &lookForPlayer;
		}
	}
}

static void groundShockStart()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags &= ~FLY;

		self->action = &groundShock;
	}

	hover();
}

static void groundShock()
{
	long onGround = self->flags & ON_GROUND;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		if (onGround == 0)
		{
			self->thinkTime = 60;

			shakeScreen(LIGHT, 15);

			self->mental -= 3;

			if (self->mental <= 0)
			{
				self->touch = &entityTouch;
			}
		}

		else
		{
			self->thinkTime--;

			if (self->thinkTime <= 0)
			{
				self->flags |= FLY;

				self->dirY = -2.5;

				self->action = &groundShockFinish;
			}
		}
	}
}

static void groundShockFinish()
{
	checkToMap(self);

	if (self->y < self->startY)
	{
		self->thinkTime--;

		self->y = self->startY;

		self->dirY = 0;

		if (self->mental <= 0)
		{
			self->action = &moveToRecharge;
		}

		else
		{
			self->action = &lookForPlayer;
		}
	}
}
