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
#include "../entity.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self;
extern Game game;

static void init(void);
static void activate(int);
static void entityWait(void);
static int draw(void);

Entity *addTrainTrack(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Train Track");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &init;

	e->activate = &activate;

	e->draw = &draw;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	int nextTrack;
	char targetTrackName[MAX_VALUE_LENGTH];

	/*
	Mental is the next track
	Health is the alternative next track

	endX is the previous track
	endY is the alternative previous track
	*/

	nextTrack = self->active == FALSE ? self->mental : self->health;

	if (nextTrack != -1)
	{
		snprintf(targetTrackName, MAX_VALUE_LENGTH, "%s_%d", self->requires, self->active == FALSE ? self->mental : self->health);

		self->target = getEntityByObjectiveName(targetTrackName);

		if (self->target == NULL)
		{
			showErrorAndExit("Track %s cannot find target %s", self->objectiveName, targetTrackName);
		}
	}

	snprintf(targetTrackName, MAX_VALUE_LENGTH, "%s_%d", self->requires, self->active == FALSE ? (int)self->endX : (int)self->endY);

	self->parent = getEntityByObjectiveName(targetTrackName);

	self->action = &entityWait;

	self->face = RIGHT;
}

static void entityWait()
{
	int nextTrack;
	char targetTrackName[MAX_VALUE_LENGTH];

	/*
	Mental is the next track
	Health is the alternative next track

	endX is the previous track
	endY is the alternative previous track
	*/

	if (self->active != self->damage)
	{
		self->damage = self->active;

		nextTrack = self->active == FALSE ? self->mental : self->health;

		if (nextTrack != -1)
		{
			snprintf(targetTrackName, MAX_VALUE_LENGTH, "%s_%d", self->requires, self->active == FALSE ? self->mental : self->health);

			self->target = getEntityByObjectiveName(targetTrackName);

			if (self->target == NULL)
			{
				showErrorAndExit("Track %s cannot find target %s", self->objectiveName, targetTrackName);
			}
		}

		snprintf(targetTrackName, MAX_VALUE_LENGTH, "%s_%d", self->requires, self->active == FALSE ? (int)self->endX : (int)self->endY);

		self->parent = getEntityByObjectiveName(targetTrackName);
	}
}

static int draw()
{
	int end, frame;
	int width, height;
	float timer;

	frame = self->currentFrame;
	timer = self->frameTimer;

	width = self->w / 2;
	height = self->h / 2;

	if (self->target != NULL && self->target->parent != NULL && self->target->parent == self)
	{
		if (self->startY == self->target->startY)
		{
			frame = self->currentFrame;
			timer = self->frameTimer;

			setEntityAnimation(self, "ACTIVE_HORIZ");

			self->currentFrame = frame;
			self->frameTimer = timer;

			self->x = self->startX < self->target->startX ? self->startX : self->target->startX;

			self->x += width;

			self->y += height - self->h / 2;

			end = self->startX < self->target->startX ? self->target->startX : self->startX;

			end += width - self->w / 2;

			while (self->x < end)
			{
				drawSpriteToMap();

				self->x += self->w;
			}
		}

		else
		{
			frame = self->currentFrame;
			timer = self->frameTimer;

			setEntityAnimation(self, "ACTIVE_VERT");

			self->currentFrame = frame;
			self->frameTimer = timer;

			self->y = self->startY < self->target->startY ? self->startY : self->target->startY;

			self->x += width - self->w / 2;

			self->y += height;

			end = self->startY < self->target->startY ? self->target->startY : self->startY;

			end += height - self->h / 2;

			while (self->y < end)
			{
				drawSpriteToMap();

				self->y += self->h;
			}
		}

		setEntityAnimation(self, "STAND");

		self->x = self->target->startX;
		self->y = self->target->startY;

		drawSpriteToMap();
	}

	setEntityAnimation(self, "STAND");

	self->currentFrame = frame;
	self->frameTimer = timer;

	self->x = self->startX;
	self->y = self->startY;

	drawLoopingAnimationToMap();

	return TRUE;
}

static void activate(int val)
{
	self->active = self->active == TRUE ? FALSE : TRUE;
}
