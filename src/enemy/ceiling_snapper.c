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
#include "../collisions.h"
#include "../geometry.h"
#include "../projectile.h"
#include "../game.h"
#include "../system/error.h"

extern Entity *self, player;

static void wait(void);
static void init(void);
static void bite(void);
static void biteFinish(void);
static void touch(Entity *);
static void trapEntity(Entity *);

Entity *addCeilingSnapper(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Ceiling Snapper");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &touch;
	e->die = &entityDie;

	e->takeDamage = &entityTakeDamageNoFlinch;

	e->action = &init;

	e->type = ENEMY;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	self->endY = MIN(getMapFloor(self->x + self->w - 1, self->y), getMapFloor(self->x, self->y));

	self->endY -= self->startY;

	self->action = &wait;
}

static void wait()
{
	int x, y;
	Entity *e;

	if (prand() % 60 == 0)
	{
		x = self->x + self->w / 2 + ((prand() % 6) * (prand() % 2 == 0 ? -1 : 1));
		y = self->y + self->h - prand() % 10;

		e = addProjectile("enemy/slime_drip", self, x, y, 0, 0);

		e->x -= e->w / 2;

		e->touch = NULL;
	}

	x = self->x - 16;

	if (player.health > 0 && collision(x, self->y, self->w + 32, self->endY, player.x, player.y, player.w, player.h) == 1)
	{
		self->action = &bite;

		self->touch = &trapEntity;

		setEntityAnimation(self, WALK);

		self->flags &= ~FLY;

		self->dirY = self->speed;
	}
}

static void bite()
{
	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->action = &biteFinish;

		self->touch = &touch;

		setEntityAnimation(self, STAND);

		self->flags |= FLY;

		self->thinkTime = 120;

		playSoundToMap("sound/enemy/floating_snapper/chomp.ogg", -1, self->x, self->y, 0);
	}
}

static void biteFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->dirY = -self->speed;

		if (self->y <= self->startY)
		{
			self->y = self->startY;

			self->action = &wait;
		}
	}

	checkToMap(self);
}

static void touch(Entity *other)
{
	Entity *temp;

	if (other->type == PLAYER)
	{
		temp = self;

		self = other;

		self->takeDamage(temp, temp->damage);

		self = temp;
	}
}

static void trapEntity(Entity *other)
{
	if (other->type == PLAYER)
	{
		other->flags |= NO_DRAW;

		other->fallout();

		playSoundToMap("sound/enemy/floating_snapper/chomp.ogg", -1, self->x, self->y, 0);

		setEntityAnimation(self, STAND);

		self->thinkTime = 180;

		self->action = &biteFinish;

		self->touch = &touch;
	}
}
