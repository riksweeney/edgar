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
#include "../hud.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "target.h"

extern Entity *self;
extern Game game;

static void autoMove(void);
static void entityWait(void);
static void findTarget(int);
static void entityWait(void);
static void setToStart(void);
static void moveToTarget(void);
static void touch(Entity *);
static Target *getLiftTarget(char *, int);

Entity *addLift(char *name, int startX, int startY, int type)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Lift");
	}

	loadProperties(name, e);

	e->type = type;

	e->activate = &findTarget;

	e->touch = &touch;

	e->action = &setToStart;

	e->draw = &drawLoopingAnimationToMap;

	e->x = e->startX = startX;
	e->y = e->startY = startY;

	e->flags |= OBSTACLE;

	setEntityAnimation(e, "STAND");

	return e;
}

static void touch(Entity *other)
{
	int bottomBefore;
	float dirX;
	Entity *temp;
	Target *t;

	/* Test the horizontal movement */

	if (other->type == PROJECTILE)
	{
		temp = self;

		self = other;

		self->die();

		self = temp;

		return;
	}

	else if (other->type == WEAPON && (other->flags & ATTACKING))
	{
		return;
	}

	else if ((other->dirY > 0 || (other->flags & ON_GROUND)) && other->touch != NULL)
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

					if (other->dirY == 0)
					{
						self->y = other->y + other->h;

						if (self->type == AUTO_LIFT)
						{
							self->targetX = (self->targetX == self->endX ? self->startX : self->endX);
							self->targetY = (self->targetY == self->endY ? self->startY : self->endY);
						}

						else
						{
							self->health--;

							t = getLiftTarget(self->objectiveName, self->health);

							self->targetX = t->x;
							self->targetY = t->y;
						}
					}
				}

				/* Place the player as close to the solid tile as possible */

				other->y = self->y;
				other->y -= other->h;

				other->standingOn = self;
				other->dirY = 0;
				other->flags |= ON_GROUND;

				if (self->type == MANUAL_LIFT && game.showHints == TRUE && other->type == PLAYER && self->dirY == 0)
				{
					setInfoBoxMessage(0, 255, 255, 255, _("Push Up or Down to use this lift"));
				}
			}
		}
	}
}

static Target *getLiftTarget(char *name, int targetID)
{
	char targetName[MAX_VALUE_LENGTH];

	snprintf(targetName, sizeof(targetName), "%s_TARGET_%d", name, targetID);

	/* Search for the lift's target */

	return getTargetByName(targetName);
}

static void findTarget(int val)
{
	Target *t;

	if (self->active == TRUE)
	{
		if (self->action == &moveToTarget || self->type == AUTO_LIFT)
		{
			return;
		}

		self->health += val;

		if (self->health < 0)
		{
			self->health = 0;
		}

		/* Search for the lift's target */

		t = getLiftTarget(self->objectiveName, self->health);

		if (t != NULL)
		{
			if (t->x == (int)self->x && t->y == (int)self->y)
			{
				self->action = &entityWait;
			}

			else
			{
				self->targetX = t->x;
				self->targetY = t->y;

				self->action = &moveToTarget;

				self->mental = playSoundToMap("sound/common/mine_lift", -1, self->x, self->y, 0);
			}
		}

		else
		{
			self->health -= val;
		}
	}

	else
	{
		setInfoBoxMessage(60, 255, 255, 255, _("This lift is not active"));
	}
}

static void moveToTarget()
{
	/* Allow Manual lifts to continue moving even if they're suddenly inactive */

	if (self->active == TRUE || self->type == MANUAL_LIFT)
	{
		if (abs(self->x - self->targetX) > self->speed)
		{
			self->dirX = (self->x < self->targetX ? self->speed : -self->speed);
		}

		else
		{
			self->x = self->targetX;
		}

		if (abs(self->y - self->targetY) > self->speed)
		{
			self->dirY = (self->y < self->targetY ? self->speed : -self->speed);
		}

		else
		{
			self->y = self->targetY;
		}

		if (self->x == self->targetX && self->y == self->targetY)
		{
			self->dirX = self->dirY = 0;

			if (self->type == AUTO_LIFT)
			{
				self->targetX = (self->targetX == self->endX ? self->startX : self->endX);
				self->targetY = (self->targetY == self->endY ? self->startY : self->endY);

				self->health = (self->x == self->endX && self->y == self->endY) ? 0 : 1;

				self->thinkTime = self->maxThinkTime;

				self->action = &autoMove;
			}

			else
			{
				stopSound(self->mental);

				self->action = &entityWait;
			}
		}

		else
		{
			self->x += self->dirX;
			self->y += self->dirY;
		}
	}

	else
	{
		self->dirX = self->dirY = 0;
	}
}

static void entityWait()
{

}

static void autoMove()
{
	if (self->active == TRUE)
	{
		if (self->thinkTime > 0)
		{
			self->thinkTime--;
		}

		else
		{
			self->action = &moveToTarget;
		}
	}

	else
	{
		self->dirY = self->dirX = 0;

		self->thinkTime = 0;
	}
}

static void setToStart()
{
	char targetName[MAX_VALUE_LENGTH];
	Target *t;

	self->face = RIGHT;

	if (self->type == AUTO_LIFT)
	{
		/* Auto lifts just use their start and end points */

		self->action = &autoMove;

		if (self->health == 1 || (self->x == self->startX && self->y == self->startY))
		{
			self->targetX = self->endX;
			self->targetY = self->endY;

			self->health = 2;
		}

		else
		{
			self->targetX = self->startX;
			self->targetY = self->startY;

			self->health = 1;
		}

		if (self->active == FALSE)
		{
			self->thinkTime = 0;
		}
	}

	else
	{
		snprintf(targetName, sizeof(targetName), "%s_TARGET_%d", self->objectiveName, self->health);

		/* Search for the lift's target */

		t = getTargetByName(targetName);

		if (t != NULL)
		{
			self->x = t->x;
			self->y = t->y;
		}

		else
		{
			printf("Could not find target %s for lift %s at %f %f!\n", targetName, self->objectiveName, self->x, self->y);

			addTarget(self->x, self->y, targetName);
		}

		self->action = &entityWait;
	}
}
