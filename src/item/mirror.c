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
#include "../graphics/animation.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "item.h"
#include "light_beam.h"

extern Entity *self;

static void touch(Entity *);
static void entityWait(void);
static void fallout(void);
static void respawn(void);

Entity *addMirror(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Mirror");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &entityWait;
	e->touch = &touch;
	e->fallout = &fallout;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, "STAND");

	return e;
}

static void touch(Entity *other)
{
	int mirrorLeft, mirrorRight, beamMid;

	if (strcmpignorecase(other->name, "item/light_beam") == 0)
	{
		if (other != self->target)
		{
			/* Light moving vertically */

			if (other->dirY > 0 && self->mental == 0) /* Upright mirror */
			{
				other->endY = self->y + self->h / 2;

				other->x = other->startX;
				other->y = other->startY;

				other->box.h = other->endY - other->y;

				mirrorLeft = self->x + self->w / 2 - 4;

				mirrorRight = self->x + self->w / 2 + 4;

				beamMid = other->x + other->w / 2;

				if (beamMid >= mirrorLeft && beamMid <= mirrorRight)
				{
					if (self->face == LEFT)
					{
						if (self->target == NULL)
						{
							self->target = addLightBeam(0, 0, "item/light_beam");

							self->target->x = self->x + self->w / 2;
							self->target->y = self->y + self->h / 2 - 6;

							self->target->dirX = -self->target->speed;

							self->target->startX = self->target->x;
							self->target->startY = self->target->y;
						}

						else
						{
							self->target->startX = self->x + self->w / 2;

							self->target->box.w = self->target->startX - self->target->x;
						}
					}

					else
					{
						if (self->target == NULL)
						{
							self->target = addLightBeam(0, 0, "item/light_beam");

							self->target->x = self->x + self->w / 2;
							self->target->y = self->y + self->h / 2 - 6;

							self->target->dirX = self->target->speed;

							self->target->startX = self->target->x;
							self->target->startY = self->target->y;
						}

						else
						{
							self->target->x = self->x + self->w / 2;

							self->target->startX = self->target->x;

							self->target->box.w = self->target->endX - self->target->x;
						}
					}

					self->thinkTime = 2;
				}
			}

			else if (other->dirY < 0 && self->mental == 1) /* Upside down mirror */
			{
				other->y = self->y + self->h / 2;

				other->box.h = other->startY - other->y;

				mirrorLeft = self->x + self->w / 2 - 4;

				mirrorRight = self->x + self->w / 2 + 4;

				beamMid = other->x + other->w / 2;

				/*if (beamMid >= mirrorLeft && beamMid <= mirrorRight)*/
				{
					if (self->face == LEFT)
					{
						if (self->target == NULL)
						{
							self->target = addLightBeam(0, 0, "item/light_beam");

							self->target->x = self->x + self->w / 2 - 2;
							self->target->y = self->y + self->h / 2;

							self->target->dirX = -self->target->speed;

							self->target->startX = self->target->x;
							self->target->startY = self->target->y;
						}
					}

					else
					{
						if (self->target == NULL)
						{
							self->target = addLightBeam(0, 0, "item/light_beam");

							self->target->x = self->x + self->w / 2;
							self->target->y = self->y + self->h / 2 + 6;

							self->target->dirX = self->target->speed;

							self->target->startX = self->target->x;
							self->target->startY = self->target->y;
						}
					}

					self->thinkTime = 2;
				}
			}

			/* Lift moving horizontally */

			else if (other->dirX > 0 && self->face == LEFT)
			{
				other->endX = self->x + self->w / 2;

				other->box.w = other->endX - other->x;

				mirrorLeft = self->y + self->h / 2 - 4;

				mirrorRight = self->y + self->h / 2 + 4;

				beamMid = other->y + other->h / 2;

				/*if (beamMid >= mirrorLeft && beamMid <= mirrorRight)*/
				{
					if (self->mental == 0) /* Upright mirror */
					{
						if (self->target == NULL)
						{
							self->target = addLightBeam(0, 0, "item/light_beam");

							self->target->x = self->x + self->w / 2 - 2;
							self->target->y = self->y + self->h / 2;

							self->target->dirY = -self->target->speed;

							self->target->startX = self->target->x;
							self->target->startY = self->target->y;
						}

						else
						{
							self->target->x = self->x + self->w / 2 - 2;
							self->target->startX = self->x + self->w / 2;
						}
					}

					else /* Upside down mirror */
					{
						if (self->target == NULL)
						{
							self->target = addLightBeam(0, 0, "item/light_beam");

							self->target->x = self->x + self->w / 2 - 2;
							self->target->y = self->y + self->h / 2;

							self->target->dirY = self->target->speed;

							self->target->startX = self->target->x;
							self->target->startY = self->target->y;
						}
					}

					self->thinkTime = 2;
				}
			}

			else if (other->dirX < 0 && self->face == RIGHT)
			{
				other->x = self->x + self->w / 2;

				other->box.w = other->startX - other->x;

				mirrorLeft = self->y + self->h / 2 - 4;

				mirrorRight = self->y + self->h / 2 + 4;

				beamMid = other->y + other->h / 2;

				/*if (beamMid >= mirrorLeft && beamMid <= mirrorRight)*/
				{
					if (self->mental == 0) /* Upright mirror */
					{
						if (self->target == NULL)
						{
							self->target = addLightBeam(0, 0, "item/light_beam");

							self->target->x = self->x + self->w / 2 - 2;
							self->target->y = self->y + self->h / 2;

							self->target->dirY = -self->target->speed;

							self->target->startX = self->target->x;
							self->target->startY = self->target->y;
						}

						else
						{
							self->target->x = self->x + self->w / 2 - 2;
							self->target->startX = self->x + self->w / 2;
						}
					}

					else /* Upside down mirror */
					{
						if (self->target == NULL)
						{
							self->target = addLightBeam(0, 0, "item/light_beam");

							self->target->x = self->x + self->w / 2 - 2;
							self->target->y = self->y + self->h / 2;

							self->target->dirY = self->target->speed;

							self->target->startX = self->target->x;
							self->target->startY = self->target->y;
						}
					}

					self->thinkTime = 2;
				}
			}
		}
	}

	else
	{
		if (other->type != PLAYER)
		{
			self->flags |= OBSTACLE;
		}

		else
		{
			self->flags &= ~OBSTACLE;
		}

		pushEntity(other);

		self->flags &= ~OBSTACLE;
	}
}

static void entityWait()
{
	doNothing();

	if (self->target != NULL)
	{
		if (self->thinkTime <= 0)
		{
			self->target->inUse = FALSE;

			self->target = NULL;
		}

		else
		{
			if (self->target->dirX < 0)
			{
				self->target->x = self->x + self->w / 2;

				self->target->startX = self->target->x;
			}
		}
	}
}

static void fallout()
{
	self->thinkTime = 120;

	self->action = &respawn;
}

static void respawn()
{
	self->thinkTime--;

	checkToMap(self);

	if (self->thinkTime <= 0)
	{
		self->x = self->startX;
		self->y = self->startY;

		setCustomAction(self, &invulnerable, 60, 0, 0);

		self->action = &entityWait;
	}
}
