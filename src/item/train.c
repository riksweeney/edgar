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
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"

extern Entity *self, player;
extern Game game;

static void touch(Entity *);
static void moveAlongTrack(void);

Entity *addTrain(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Train");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->face = RIGHT;

	e->action = &moveAlongTrack;

	e->touch = &touch;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void touch(Entity *other)
{
	int bottomBefore;
	float dirX;

	if ((other->dirY > 0 || (other->flags & ON_GROUND)) && other->touch != NULL)
	{
		/* Trying to move down */

		if (collision(other->x, other->y, other->w, other->h, self->x, self->y, self->w, self->h) == 1)
		{
			bottomBefore = other->y + other->h - other->dirY - 1;

			if (abs(bottomBefore - self->y) < self->h - 1)
			{
				if (self->dirY < 0)
				{
					other->y -= self->dirY;

					other->dirY = self->dirY;

					dirX = other->dirX;

					other->dirX = 0;

					checkToMap(other);

					other->dirX = dirX;
				}

				/* Place the player as close to the solid tile as possible */

				other->y = self->y;
				other->y -= other->h;

				other->standingOn = self;
				other->dirY = 0;
				other->flags |= ON_GROUND;
			}
		}
	}
}

static void moveAlongTrack()
{
	Entity *e;
	char trackName[MAX_VALUE_LENGTH];
	int track;

	if (self->dirX == 0 && self->dirY == 0)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			snprintf(trackName, MAX_VALUE_LENGTH, "%s_TRACK_%d", self->objectiveName, self->health);

			e = getEntityByObjectiveName(trackName);

			if (e != NULL)
			{
				if (self->mental == 1)
				{
					track = e->active == FALSE ? e->mental : e->health;

					snprintf(trackName, MAX_VALUE_LENGTH, "%s_TRACK_%d", self->objectiveName, track);

					e = getEntityByObjectiveName(trackName);

					if (e == NULL)
					{
						self->thinkTime = self->maxThinkTime;

						self->mental = -1;
					}

					else
					{
						self->health = track;

						self->endX = e->startX;
						self->endY = e->startY;

						calculatePath(self->x, self->y, self->endX, self->endY, &self->dirX, &self->dirY);

						self->dirX *= self->speed;
						self->dirY *= self->speed;
					}
				}

				else
				{
					track = e->active == FALSE ? e->endX : e->endY;

					snprintf(trackName, MAX_VALUE_LENGTH, "%s_TRACK_%d", self->objectiveName, track);

					e = getEntityByObjectiveName(trackName);

					if (e == NULL)
					{
						self->thinkTime = self->maxThinkTime;

						self->mental = 1;
					}

					else
					{
						self->health = track;

						self->endX = e->startX;
						self->endY = e->startY;

						calculatePath(self->x, self->y, self->endX, self->endY, &self->dirX, &self->dirY);

						self->dirX *= self->speed;
						self->dirY *= self->speed;
					}
				}
			}

			else
			{
				showErrorAndExit("Train %s could not find track %s", self->objectiveName, trackName);
			}
		}
	}

	else
	{
		checkToMap(self);

		if (fabs(self->endX - self->x) <= fabs(self->dirX) && fabs(self->endY - self->y) <= fabs(self->dirY))
		{
			self->x = self->endX;
			self->y = self->endY;

			self->dirX = 0;
			self->dirY = 0;
		}
	}
}
