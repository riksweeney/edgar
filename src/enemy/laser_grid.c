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
#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"

extern Entity *self;

static void init(void);
static void entityWait(void);
static void addLaser(void);
static void laserWait(void);

Entity *addLaserGrid(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Laser Grid");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = &entityTouch;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	addLaser();

	self->action = &entityWait;

	self->health = self->active;
}

static void entityWait()
{
	if (self->mental != -1)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->active = self->active == TRUE ? FALSE : TRUE;

			if (self->active == TRUE)
			{
				playSoundToMap("sound/enemy/laser/zap", 7, self->x, self->y, 0);
			}

			self->thinkTime = self->maxThinkTime;
		}
	}

	else if (self->active != self->health)
	{
		self->health = self->active;
	}
}

static void addLaser()
{
	int i;
	Entity *e;

	if (strcmpignorecase(self->name, "enemy/horizontal_laser_grid") == 0)
	{
		for (i=self->startX;i<self->endX;i+=32)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add a Laser");
			}

			loadProperties("enemy/horizontal_laser", e);

			e->x = i;
			e->y = self->y;

			e->action = &laserWait;

			e->draw = &drawLoopingAnimationToMap;
			e->touch = &entityTouch;

			e->head = self;

			e->currentFrame = prand() % 6;

			e->face = RIGHT;

			setEntityAnimation(e, "STAND");
		}
	}

	else
	{
		for (i=self->startY;i<self->endY;i+=32)
		{
			e = getFreeEntity();

			if (e == NULL)
			{
				showErrorAndExit("No free slots to add a Laser");
			}

			loadProperties("enemy/laser", e);

			e->x = self->x;
			e->y = i;

			e->action = &laserWait;

			e->draw = &drawLoopingAnimationToMap;
			e->touch = &entityTouch;

			e->head = self;

			e->currentFrame = prand() % 6;

			e->face = RIGHT;

			setEntityAnimation(e, "STAND");
		}
	}
}

static void laserWait()
{
	if (self->head->active == TRUE)
	{
		self->flags &= ~NO_DRAW;

		self->touch = &entityTouch;
	}

	else
	{
		self->flags |= NO_DRAW;

		self->touch = NULL;
	}
}
