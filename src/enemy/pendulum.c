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
#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;

static void swing(void);
static int ringDraw(void);
static void init(void);

Entity *addPendulum(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Pendulum");
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
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Pendulum Ring");
	}

	loadProperties("enemy/pendulum_ring", e);

	setEntityAnimation(e, "STAND");

	e->x = self->startX + self->w / 2 - e->w / 2;
	e->y = self->startY;

	e->startX = e->x;
	e->startY = e->y;

	e->action = &doNothing;
	e->draw = &ringDraw;

	e->type = ENEMY;

	e->head = self;

	self->action = &swing;
}

static void swing()
{
	float x, y, radians, origX;

	x = 0;
	y = self->endY - self->startY;

	self->thinkTime += 2;

	radians = DEG_TO_RAD(self->thinkTime);

	self->dirX = (self->speed * cos(radians) - 0 * sin(radians));

	self->endX += self->dirX;

	radians = DEG_TO_RAD(self->endX);

	origX = self->x;

	self->x = (x * cos(radians) - y * sin(radians));
	self->y = (x * sin(radians) + y * cos(radians));

	self->x += self->startX;
	self->y += self->startY;

	if ((origX < self->startX && self->x >= self->startX) || (origX > self->startX && self->x <= self->startX))
	{
		playSoundToMap("sound/enemy/pendulum/swing", -1, self->x, self->y, 0);
	}
}

static int ringDraw()
{
	int i, endX, endY;
	float diffX, diffY;

	endX = self->head->x;
	endY = self->head->y + self->head->h / 2 - self->h / 2;

	diffX = (endX - self->head->startX) / self->head->mental;
	diffY = (endY - self->head->startY) / self->head->mental;

	drawLoopingAnimationToMap();

	for (i=0;i<self->head->mental;i++)
	{
		drawSpriteToMap();

		self->x += diffX;
		self->y += diffY;
	}

	self->x = self->startX;
	self->y = self->startY;

	return 1;
}
