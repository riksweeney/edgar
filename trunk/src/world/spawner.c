#include "../headers.h"

#include "../graphics/animation.h"
#include "../enemy/enemies.h"
#include "../entity.h"
#include "../system/properties.h"
#include "../player.h"
#include "../decoration.h"

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
	self->action = &spawn;

	self->action();
}

static void spawn()
{
	Entity *e;

	if (self->active == TRUE)
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			if (self->health < 0)
			{
				/* Don't spawn if the player is too close */

				if (self->health == -1 || (self->health == -2 && getDistanceFromPlayer(self) > SCREEN_WIDTH))
				{
					if (strcmpignorecase(self->name, "common/decoration_spawner") == 0)
					{
						e = addDecoration(self->objectiveName, self->x, self->y);

						e->x += (self->w - e->w) / 2;
						e->y += (self->h - e->h) / 2;
					}

					else
					{
						e = addEnemy(self->objectiveName, self->x, self->y);

						e->x += (self->w - e->w) / 2;
						e->y += (self->h - e->h) / 2;

						e->startX = self->startX;
						e->startY = self->startX;

						e->endX = self->endX;
						e->endY = self->endX;

						e->face = self->face;
					}
				}
			}

			else
			{
				e = addEnemy(self->objectiveName, self->x, self->y);

				e->x += (self->w - e->w) / 2;
				e->y += (self->h - e->h) / 2;

				e->startX = self->startX;
				e->startY = self->startX;

				e->endX = self->endX;
				e->endY = self->endX;

				self->health--;

				if (self->health == 0)
				{
					self->inUse = FALSE;
				}
			}

			self->thinkTime = self->maxThinkTime;
		}
	}
}
