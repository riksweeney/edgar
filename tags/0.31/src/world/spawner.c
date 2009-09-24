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
#include "../enemy/enemies.h"
#include "../entity.h"
#include "../system/properties.h"
#include "../player.h"
#include "../graphics/decoration.h"
#include "../projectile.h"

extern Entity *self;

static void spawn(void);
static void init(void);

Entity *addSpawner(int x, int y, char *entityToSpawn)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add a Spawner\n");

		exit(1);
	}

	loadProperties(entityToSpawn, e);

	e->x = x;
	e->y = y;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->action = &init;

	e->type = SPAWNER;

	setEntityAnimation(e, STAND);

	return e;
}

static void init()
{
	Entity *e;

	if (strlen(self->objectiveName) == 0)
	{
		printf("Spawner at %f %f is not set correctly\n", self->x, self->y);

		exit(1);
	}

	/* Precache the Entities to spawn */

	e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add spawner entity\n");

		exit(1);
	}

	loadProperties(self->objectiveName, e);

	e->inUse = FALSE;

	self->action = &spawn;

	self->action();
}

static void spawn()
{
	int distance;
	Entity *e;

	if (self->active == TRUE)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			if (self->health < 0)
			{
				/* Don't spawn if the player is too close or too far away */

				distance = self->health == -2 ? getDistanceFromPlayer(self) : 0;

				if (self->health == -1 || (self->health == -2 && distance > SCREEN_WIDTH && distance < SCREEN_WIDTH + TILE_SIZE))
				{
					if (strcmpignorecase(self->name, "common/decoration_spawner") == 0)
					{
						e = addDecoration(self->objectiveName, self->x, self->y);

						e->x += (self->w - e->w) / 2;
						e->y += (self->h - e->h) / 2;
					}

					else if (strcmpignorecase(self->name, "common/projectile_spawner") == 0)
					{
						e = addProjectile(self->objectiveName, self, self->x, self->y, 0, 0);

						e->x += (self->w - e->w) / 2;

						if (e->flags & FLY)
						{
							e->dirX = (self->face == LEFT ? -e->speed : e->speed);

							e->face = self->face;

							e->y += (self->h - e->h) / 2;
						}
					}

					else
					{
						e = addEnemy(self->objectiveName, self->x, self->y);

						e->x += (self->w - e->w) / 2;
						e->y += (self->h - e->h) / 2;

						e->startX = self->startX;
						e->startY = self->startY;

						e->endX = self->endX;
						e->endY = self->endY;

						e->face = self->face;
					}

					if (self->speed != 0)
					{
						e->speed = self->speed;
					}

					self->thinkTime = self->maxThinkTime;
				}
			}

			else
			{
				e = addEnemy(self->objectiveName, self->x, self->y);

				e->x += (self->w - e->w) / 2;
				e->y += (self->h - e->h) / 2;

				e->startX = self->startX;
				e->startY = self->startY;

				e->endX = self->endX;
				e->endY = self->endY;

				e->face = self->face;

				if (self->speed != 0)
				{
					e->speed = self->speed;
				}

				self->health--;

				if (self->health == 0)
				{
					self->inUse = FALSE;
				}

				self->thinkTime = self->maxThinkTime;
			}
		}
	}
}
