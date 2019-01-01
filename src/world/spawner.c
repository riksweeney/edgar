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

#include "../enemy/enemies.h"
#include "../entity.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../item/item.h"
#include "../item/key_items.h"
#include "../player.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
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

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	Entity *e;
	char spawnList[MAX_VALUE_LENGTH], name[MAX_VALUE_LENGTH];
	char *token;

	#if DEV == 1
	if (strlen(self->objectiveName) == 0)
	{
		showErrorAndExit("Spawner at %f %f is not set correctly", self->x, self->y);
	}
	#endif

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
		if (strcmpignorecase(self->name, "common/spawner") == 0 && strstr(token, "/") == NULL)
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
			self->thinkTime = 0;

			if (self->health < 0)
			{
				/* Don't spawn if the player is too close or too far away */

				distance = self->health == -2 ? getDistanceFromPlayer(self) : 0;

				if (self->health == -1 || self->health == -3 || (self->health == -2 && distance > SCREEN_WIDTH && distance < SCREEN_WIDTH + TILE_SIZE))
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

					else if (strcmpignorecase(self->name, "common/item_spawner") == 0)
					{
						e = addPermanentItem(self->objectiveName, self->x, self->y);

						e->x += (self->w - e->w) / 2;

						e->face = self->face;
					}

					else if (strcmpignorecase(self->name, "common/key_item_spawner") == 0)
					{
						e = addKeyItem(self->objectiveName, self->x, self->y);

						e->x += (self->w - e->w) / 2;

						e->face = self->face;
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

						if (strstr(token, "/") == NULL)
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

						if (self->health == -2)
						{
							e->flags |= SPAWNED_IN;

							e->spawnTime = SPAWNED_IN_TIME;
						}
					}

					if (self->speed != 0)
					{
						e->speed = self->speed;
					}

					self->thinkTime = self->maxThinkTime;

					if (self->health == -3)
					{
						self->active = FALSE;
					}
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

				if (strstr(token, "/") == NULL)
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
