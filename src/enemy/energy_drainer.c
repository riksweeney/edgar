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

#include "../collisions.h"
#include "../custom_actions.h"
#include "../entity.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../graphics/graphics.h"
#include "../item/item.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void fly(void);
static void init(void);
static void takeDamage(Entity *, int);
static void die(void);
static void ballWait(void);
static void stunned(void);
static void stunFinish(void);
static void returnToGenerator(void);
static int draw(void);
static void createBeam(void);
static void beamWait(void);
static int beamDraw(void);
static void creditsMove(void);

Entity *addEnergyDrainer(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Energy Drainer");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;
	e->draw = &draw;
	e->die = &die;
	e->takeDamage = &takeDamage;
	e->reactToBlock = NULL;
	e->touch = &entityTouch;
	e->fallout = &die;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	Entity *e;

	/* Set target to power generator */

	e = getEntityByObjectiveName(self->requires);

	if (e == NULL)
	{
		showErrorAndExit("Energy Drainer could not find Entity %s", self->requires);
	}

	self->targetX = e->x + e->w / 2;
	self->targetY = e->y + e->h / 2;

	self->targetX -= self->w / 2;
	self->targetY -= self->h / 2;

	self->target = e;

	self->y = self->startY;

	self->action = &fly;

	self->mental = 1;

	createBeam();
}

static void fly()
{
	if (self->dirX == 0)
	{
		self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

		self->face = (self->face == RIGHT ? LEFT : RIGHT);
	}

	self->thinkTime += 5;

	self->dirY += cos(DEG_TO_RAD(self->thinkTime));

	self->dirY /= 3;

	checkToMap(self);

	if (self->mental == 1)
	{
		self->endX = MAX(getMapCeiling(self->x + self->w - 1, self->y), getMapCeiling(self->x, self->y));

		self->endY = MIN(getMapFloor(self->x + self->w - 1, self->y), getMapFloor(self->x, self->y));
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

			if (self->flags & FLY)
			{
				self->dirX = 0;

				self->action = &stunned;

				self->takeDamage = &entityTakeDamageNoFlinch;

				self->flags &= ~FLY;

				self->maxThinkTime = self->mental;

				self->mental = 0;

				self->thinkTime = 180;
			}
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

static void stunned()
{
	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->flags |= FLY;

		self->action = &stunFinish;

		self->dirY = -self->speed;
	}
}

static void stunFinish()
{
	checkToMap(self);

	if (self->dirY == 0 || self->y < self->startY)
	{
		self->dirY = 0;

		self->mental = 1;

		self->takeDamage = &takeDamage;

		self->action = &fly;
	}
}

static void die()
{
	Entity *e, *arrow;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Energy Drainer Ball");
	}

	loadProperties("enemy/energy_drainer_ball", e);

	e->x = self->x + self->w / 2;
	e->y = self->y + self->h / 2;

	e->targetX = self->targetX;
	e->targetY = self->targetY;

	e->target = self->target;

	e->x -= e->w / 2;
	e->y -= e->h / 2;

	e->startY = e->y;

	e->action = &ballWait;
	e->draw = &drawLoopingAnimationToMap;
	e->die = NULL;
	e->touch = NULL;
	e->fallout = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	e->thinkTime = 180;

	self->mental = 0;

	/* Drop between 1 and 3 arrows */

	arrow = addTemporaryItem("weapon/normal_arrow", self->x, self->y, RIGHT, 0, ITEM_JUMP_HEIGHT);

	arrow->health = 1 + (prand() % 3);

	entityDie();
}

static void ballWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

		self->dirX *= self->speed;
		self->dirY *= self->speed;

		self->action = &returnToGenerator;
	}

	else
	{
		self->endY += 5;

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

		self->target->active = TRUE;

		self->inUse = FALSE;
	}
}

static int draw()
{
	/*
	if ((self->mental == 1) && self->health > 0)
	{
		drawLine(self->x + self->w / 2, self->endX, self->x + self->w / 2, self->endY, 255, 0, 0);
	}
	*/
	drawLoopingAnimationToMap();

	return TRUE;
}

static void createBeam()
{
	Entity *e;

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add an Energy Beam");
	}

	loadProperties("enemy/energy_beam", e);

	e->head = self;

	e->x = self->x;
	e->y = self->y;

	e->action = &init;
	e->draw = &beamDraw;
	e->die = NULL;
	e->takeDamage = NULL;
	e->reactToBlock = NULL;
	e->touch = &entityTouch;

	e->type = ENEMY;

	e->face = RIGHT;

	setEntityAnimation(e, "STAND");

	e->action = &beamWait;

	e->creditsAction = &beamWait;
}

static void beamWait()
{
	if (self->head->inUse == TRUE)
	{
		if (self->head->mental == 0)
		{
			self->flags |= NO_DRAW;

			self->touch = NULL;
		}

		else
		{
			self->flags &= ~NO_DRAW;

			self->box.y = self->head->endX - self->y;
			self->box.h = self->head->endY - self->head->endX;

			self->touch = &entityTouch;
		}
	}

	else
	{
		self->inUse = FALSE;
	}
}

static int beamDraw()
{
	int drawn;
	Entity *e;

	if (self->head->mental == 1 && self->head->health > 0)
	{
		self->x = self->head->x;
		self->y = self->head->endX;

		drawn = drawLoopingAnimationToMap();

		if (drawn == TRUE)
		{
			e = addPixelDecoration(self->x + self->w / 2, self->y);

			if (e != NULL)
			{
				e->dirX = prand() % 20;
				e->dirY = prand() % 20;

				if (prand() % 2 == 0)
				{
					e->dirX *= -1;
				}

				e->dirX /= 10;
				e->dirY /= 10;

				e->thinkTime = 20 + (prand() % 30);

				e->health = 255;

				e->maxHealth = 0;

				e->mental = 255;
			}
		}

		while (self->y < self->head->endY - self->h)
		{
			self->y += self->h;

			drawn = drawSpriteToMap();
		}

		if (drawn == TRUE)
		{
			e = addPixelDecoration(self->x + self->w / 2, self->head->endY);

			if (e != NULL)
			{
				e->dirX = prand() % 20;
				e->dirY = -prand() % 20;

				if (prand() % 2 == 0)
				{
					e->dirX *= -1;
				}

				e->dirX /= 10;
				e->dirY /= 10;

				e->thinkTime = 20 + (prand() % 30);

				e->health = 255;

				e->maxHealth = 0;

				e->mental = 255;
			}
		}
	}

	return TRUE;
}

static void creditsMove()
{
	if (self->mental == 0)
	{
		createBeam();

		self->mental = 1;
	}

	self->thinkTime += 5;

	self->dirY += cos(DEG_TO_RAD(self->thinkTime));

	self->dirY /= 3;

	self->dirX = self->speed;

	checkToMap(self);

	if (self->mental == 1)
	{
		self->endX = MAX(getMapCeiling(self->x + self->w - 1, self->y), getMapCeiling(self->x, self->y));

		self->endY = MIN(getMapFloor(self->x + self->w - 1, self->y), getMapFloor(self->x, self->y));
	}

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}
}
