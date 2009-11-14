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
#include "../system/error.h"
#include "../system/random.h"

extern Entity *self;

static void spawn(void);
static void init(void);

Entity *addSpawner(int x, int y, char *entityToSpawn)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Spawner");
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
	char spawnList[MAX_VALUE_LENGTH], name[MAX_VALUE_LENGTH];
	char *token;

	if (strlen(self->objectiveName) == 0)
	{
		showErrorAndExit("Spawner at %f %f is not set correctly", self->x, self->y);
	}

	/* Precache the Entities to spawn */

	e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add spawner entity");
	}

	STRNCPY(spawnList, self->objectiveName, MAX_VALUE_LENGTH);

	token = strtok(spawnList, "|");

	while (token != NULL)
	{
		if (strcmpignorecase(self->name, "common/spawner") == 0 && strstr(token, "enemy/") == NULL)
		{
			snprintf(name, sizeof(name), "enemy/%s", token);
		}
		
		else
		{
			snprintf(name, sizeof(name), "%s", token);
		}
		
		loadProperties(name, e);

		token = strtok(NULL, "|");
	}

	e->inUse = FALSE;

	self->action = &spawn;

	self->action();
}

static void spawn()
{
	int distance;
	char spawnList[MAX_VALUE_LENGTH], name[MAX_VALUE_LENGTH];
	char *token;
	int spawnIndex = 0, spawnCount = 0;
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
						STRNCPY(spawnList, self->objectiveName, MAX_VALUE_LENGTH);

						token = strtok(spawnList, "|");

						while (token != NULL)
						{
							token = strtok(NULL, "|");

							spawnCount++;
						}

						if (spawnCount == 0)
						{
							showErrorAndExit("Spawner at %f %f has no spawn list", self->x, self->y);
						}

						spawnIndex = prand() % spawnCount;

						STRNCPY(spawnList, self->objectiveName, MAX_VALUE_LENGTH);

						spawnCount = 0;

						token = strtok(spawnList, "|");

						while (token != NULL)
						{
							if (spawnCount == spawnIndex)
							{
								break;
							}

							token = strtok(NULL, "|");

							spawnCount++;
						}
						
						if (strstr(token, "enemy/") == NULL)
						{
							snprintf(name, sizeof(name), "enemy/%s", token);
						}
						
						else
						{
							snprintf(name, sizeof(name), "%s", token);
						}

						e = addEnemy(name, self->x, self->y);

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
				STRNCPY(spawnList, self->objectiveName, MAX_VALUE_LENGTH);

				token = strtok(spawnList, "|");

				while (token != NULL)
				{
					token = strtok(NULL, "|");

					spawnCount++;
				}

				if (spawnCount == 0)
				{
					showErrorAndExit("Spawner at %f %f has no spawn list", self->x, self->y);
				}

				spawnIndex = prand() % spawnCount;

				STRNCPY(spawnList, self->objectiveName, MAX_VALUE_LENGTH);

				spawnCount = 0;

				token = strtok(spawnList, "|");

				while (token != NULL)
				{
					if (spawnCount == spawnIndex)
					{
						break;
					}

					token = strtok(NULL, "|");

					spawnCount++;
				}

				if (strstr(token, "enemy/") == NULL)
				{
					snprintf(name, sizeof(name), "enemy/%s", token);
				}
				
				else
				{
					snprintf(name, sizeof(name), "%s", token);
				}

				e = addEnemy(name, self->x, self->y);

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
