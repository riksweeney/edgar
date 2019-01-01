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
#include "../graphics/animation.h"
#include "../map.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void die(void);
static void init(void);
static void flyAround(void);
static void dropWait(void);
static void walkAround(void);
static void flyStart(void);
static int safeToDrop(void);
static void podWait(void);
static void dropPod(void);
static void podTakeDamage(Entity *, int);
static void podExplode(void);
static void creditsMove(void);

Entity *addDragonFly(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Dragon Fly");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->die = &die;
	e->takeDamage = &entityTakeDamageNoFlinch;
	e->reactToBlock = &changeDirection;

	e->creditsAction = &creditsMove;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	switch (self->mental)
	{
		case 1:
			setEntityAnimation(self, "WALK");

			self->action = &walkAround;
		break;

		case 2:
			self->action = &dropWait;
		break;

		case 3:
			self->action = &flyStart;
		break;

		default:
			self->action = &flyAround;
		break;
	}
}

static void flyAround()
{
	if (self->dirX == 0)
	{
		self->x += self->face == LEFT ? self->box.x : -self->box.x;

		self->dirX = (self->face == RIGHT ? -self->speed : self->speed);

		self->face = (self->face == RIGHT ? LEFT : RIGHT);
	}

	self->startX += 5;

	self->dirY = cos(DEG_TO_RAD(self->startX));

	checkToMap(self);

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (safeToDrop() == TRUE)
		{
			self->dirX = 0;

			self->thinkTime = 30;

			self->action = &dropWait;

			self->mental = 2;

			self->endX = 0;
		}

		else
		{
			self->thinkTime = self->maxThinkTime;
		}
	}

	if (prand() % 600 == 0)
	{
		dropPod();
	}
}

static void dropWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirY = 3;
	}

	checkToMap(self);

	if ((self->flags & ON_GROUND) || self->standingOn != NULL)
	{
		self->flags &= ~FLY;

		self->action = &walkAround;

		self->creditsAction = &creditsMove;

		self->thinkTime = 600;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		setEntityAnimation(self, "WALK");

		self->mental = 1;
	}
}

static void walkAround()
{
	int face = self->face;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirX = 0;

		self->thinkTime = 30;

		self->dirY = -3;

		self->action = &flyStart;

		self->flags |= FLY;

		setEntityAnimation(self, "STAND");

		self->mental = 3;

		playSoundToMap("sound/enemy/bug/buzz", -1, self->x, self->y, 0);
	}

	else
	{
		moveLeftToRight();

		if (self->face != face)
		{
			self->endX++;

			if (self->endX >= 15)
			{
				self->thinkTime = 0;
			}
		}
	}
}

static void flyStart()
{
	checkToMap(self);

	if (self->dirY == 0 || self->y < self->startY)
	{
		self->flags &= ~FLY;

		self->action = &flyAround;

		self->creditsAction = &creditsMove;

		self->dirX = self->face == LEFT ? -self->speed : self->speed;

		self->thinkTime = 600;

		self->mental = 0;
	}

	else
	{
		self->dirY = -3;
	}
}

static int safeToDrop()
{
	int x, y, i, tile;

	x = self->x + self->w / 2;

	y = self->y + self->h - 1;

	x /= TILE_SIZE;
	y /= TILE_SIZE;

	y++;

	for (i=0;i<30;i++)
	{
		tile = mapTileAt(x, y);

		if (tile != BLANK_TILE && (tile < BACKGROUND_TILE_START || tile > FOREGROUND_TILE_START))
		{
			return tile < BACKGROUND_TILE_START ? TRUE : FALSE;
		}

		y++;
	}

	return FALSE;
}

static void dropPod()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Dragon Fly Pod");
	}

	loadProperties("enemy/dragon_fly_pod", e);

	setEntityAnimation(e, "STAND");

	e->x = self->x + self->w / 2 - e->w / 2;
	e->y = self->y + self->h / 2 - e->h / 2;

	e->action = &podWait;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;
	e->fallout = &entityDieNoDrop;
	e->die = &entityDieNoDrop;
	e->pain = &enemyPain;
	e->takeDamage = &podTakeDamage;

	e->head = self;

	e->type = ENEMY;
}

static void podWait()
{
	if (self->flags & ON_GROUND)
	{
		self->thinkTime--;

		if (self->thinkTime < 120)
		{
			if (self->thinkTime % 3 == 0)
			{
				self->flags ^= FLASH;
			}
		}

		if (self->thinkTime <= 0)
		{
			self->flags &= ~FLASH;

			self->action = &podExplode;

			self->flags |= FLY;

			self->dirY = -4;

			self->thinkTime = 5;
		}
	}

	checkToMap(self);
}

static void podExplode()
{
	int x, y;
	Entity *e;

	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		e = addProjectile("common/green_blob", self->head, 0, 0, -6, 0);

		x = self->x + self->w / 2 - e->w / 2;
		y = self->y;

		e->x = x;
		e->y = y;

		e->flags |= FLY;

		e->reactToBlock = &bounceOffShield;

		e = addProjectile("common/green_blob", self->head, x, y, -6, -6);

		e->flags |= FLY;

		e->reactToBlock = &bounceOffShield;

		e = addProjectile("common/green_blob", self->head, x, y, 0, -6);

		e->flags |= FLY;

		e->reactToBlock = &bounceOffShield;

		e = addProjectile("common/green_blob", self->head, x, y, 6, -6);

		e->flags |= FLY;

		e->reactToBlock = &bounceOffShield;

		e = addProjectile("common/green_blob", self->head, x, y, -6, 6);

		e->flags |= FLY;

		e->reactToBlock = &bounceOffShield;

		e = addProjectile("common/green_blob", self->head, x, y, 0, 6);

		e->flags |= FLY;

		e->reactToBlock = &bounceOffShield;

		e = addProjectile("common/green_blob", self->head, x, y, 6, 6);

		e->flags |= FLY;

		e->reactToBlock = &bounceOffShield;

		e = addProjectile("common/green_blob", self->head, x, y, 6, 0);

		e->flags |= FLY;

		e->reactToBlock = &bounceOffShield;

		playSoundToMap("sound/common/pop", -1, self->x, self->y, 0);

		self->inUse = FALSE;
	}

	checkToMap(self);
}

static void podTakeDamage(Entity *other, int damage)
{
	Entity *temp;

	if (self->flags & INVULNERABLE)
	{
		return;
	}

	if (damage != 0)
	{
		self->health -= damage;

		if (other->type == PROJECTILE)
		{
			temp = self;

			self = other;

			self->die();

			self = temp;
		}

		if (self->health > 0)
		{
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
		}

		else
		{
			self->damage = 0;

			self->die();
		}
	}
}

static void creditsMove()
{
	self->thinkTime++;

	if (self->mental == 1)
	{
		self->flags &= ~FLY;

		setEntityAnimation(self, "WALK");
	}

	else
	{
		self->dirY = 0;

		self->flags |= FLY;

		setEntityAnimation(self, "STAND");
	}

	setEntityAnimation(self, self->mental == 1 ? "WALK" : "STAND");

	self->dirX = self->speed;

	if (self->flags & FLY)
	{
		self->startX += 5;

		self->dirY = cos(DEG_TO_RAD(self->startX));
	}

	checkToMap(self);

	if (self->dirX == 0)
	{
		self->inUse = FALSE;
	}

	if (self->thinkTime != 0 && (self->thinkTime % 240) == 0)
	{
		if (self->flags & FLY)
		{
			self->dirX = 0;

			self->thinkTime = 30;

			self->creditsAction = &dropWait;

			self->mental = 2;

			self->endX = 0;
		}

		else
		{
			self->dirX = 0;

			self->thinkTime = 30;

			self->dirY = -3;

			self->creditsAction = &flyStart;

			self->flags |= FLY;

			setEntityAnimation(self, "STAND");

			self->mental = 3;

			playSoundToMap("sound/enemy/bug/buzz", -1, self->x, self->y, 0);
		}
	}
}

static void die()
{
	playSoundToMap("sound/enemy/wasp/wasp_die", -1, self->x, self->y, 0);

	entityDie();
}
