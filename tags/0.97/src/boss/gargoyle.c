/*
Copyright (C) 2009-2012 Parallel Realities

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
#include "../audio/music.h"
#include "../collisions.h"
#include "../custom_actions.h"
#include "../enemy/enemies.h"
#include "../enemy/rock.h"
#include "../entity.h"
#include "../event/script.h"
#include "../game.h"
#include "../geometry.h"
#include "../graphics/animation.h"
#include "../graphics/decoration.h"
#include "../graphics/graphics.h"
#include "../hud.h"
#include "../inventory.h"
#include "../item/key_items.h"
#include "../map.h"
#include "../player.h"
#include "../projectile.h"
#include "../system/error.h"
#include "../system/properties.h"
#include "../system/random.h"
#include "../world/target.h"
#include "../weather.h"

extern Entity *self, player;

static void initialise(void);
static void addStoneCoat(void);
static void init(void);
static void doIntro(void);
static void introFinish(void);
static void attackFinished(void);
static void coatWait(void);

Entity *addGargoyle(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Gargoyle");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &init;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->die = &die;
	e->takeDamage = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void init()
{
	if (self->mental == 0)
	{
		addStoneCoat();

		self->action = &initialise;
	}

	else
	{
		self->action = &introFinish;
	}
}

static void initialise()
{
	Target *t;

	if (self->active == TRUE)
	{
		if (cameraAtMinimum())
		{
			if (self->mental == 0)
			{
				t = getTargetByName("GARGOYLE_INTRO_TARGET");

				if (t == NULL)
				{
					showErrorAndExit("Gargoyle cannot find target");
				}

				calculatePath(self->x, self->y, self->targetX, self->targetY, &self->dirX, &self->dirY);

				self->dirX *= self->speed;
				self->dirY *= self->speed;

				self->thinkTime = 30;

				self->action = &doIntro;
			}
		}
	}

	checkToMap(self);
}

static void doIntro()
{
	int i;
	Entity *e;

	if (atTarget())
	{
		self->thinkTime--;

		if (self->thinkTime <= 0)
		{
			self->flags &= ~FLY;
		}
	}

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		shakeScreen(MEDIUM, 30);

		playSoundToMap("sound/common/crash.ogg", BOSS_CHANNEL, self->x, self->y, 0);

		for (i=0;i<30;i++)
		{
			e = addSmoke(self->x + (prand() % self->w), self->y + self->h, "decoration/dust");

			if (e != NULL)
			{
				e->y -= prand() % e->h;
			}
		}

		self->thinkTime = 30;

		self->action = &introFinish;
	}
}

static void introFinish()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		self->maxThinkTime = 0;

		self->mental = 2;

		setContinuePoint(FALSE, self->name, NULL);

		initBossHealthBar();

		facePlayer();

		playDefaultBossMusic();

		self->action = &attackFinished;
	}
}

static void attackFinished()
{
	self->thinkTime = 30;

	self->action = &entityWait;

	checkToMap(self);
}

static void entityWait()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		if (self->target == NULL)
		{
			self->action = &createLanceInit;
		}

		else
		{
			swtich (self->maxThinkTime)
			{
				case 0:
					switch (prand() % 4)
					{
						/* Stab */
						/* Vertical throw */
						/* Weapon remove blast */
					}
				break;

				case 1:
					switch (prand() % 4)
					{
						/* Portal attack */
						/* Wind attack */
						/* Weapon remove blast */
					}
				break;

				case 2:
					switch (prand() % 4)
					{
						/* Invisible drop */
						/* Weapon remove blast */
					}
				break;

				default:
					switch (prand() % 4)
					{
						/* Drop attack */
						/* Weapon remove blast */
						/* Punch */
					}
				break;
			}
		}
	}

	checkToMap(self);
}

static void createLanceInit()
{
	self->flags &= ~FLY;

	checkToMap(self);

	if (self->flags & ON_GROUND)
	{
		self->thinkTime = 15;

		self->action = &createLance;
	}
}

static void createLance()
{
	Entity *e;

	setEntityAnimation(self, "CREATE_LANCE");

	self->mental = 1;

	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Gargoyle");
	}

	switch (self->maxThinkTime)
	{
		case 0:
			loadProperties("boss/gargoyle_lance_1", e);
		break;

		case 1:
			loadProperties("boss/gargoyle_lance_2", e);
		break;

		default:
			loadProperties("boss/gargoyle_lance_3", e);
		break;
	}

	e->action = &lanceAppearWait;

	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;

	e->head = self;

	setEntityAnimation(e, "LANCE_APPEAR");

	e->animationCallback = &lanceAppearFinish;

	self->target = e;

	self->action = &createLanceWait;
}

static void createLanceWait()
{
	if (self->mental == 0)
	{
		self->action = &attackFinished;
	}

	checkToMap(self);
}

static void lanceAppearWait()
{
	self->face = self->head->face;

	if (self->face == LEFT)
	{
		e->x = self->x + self->w - e->w - self->offsetX;
	}

	else
	{
		e->x = self->x + self->offsetX;
	}

	e->y = self->y + self->offsetY;
}

static void lanceAppearFinish()
{
	self->head->mental = 0;

	self->action = &lanceWait;
}

static void lanceWait()
{
	self->face = self->head->face;

	if (self->face == LEFT)
	{
		e->x = self->x + self->w - e->w - self->offsetX;
	}

	else
	{
		e->x = self->x + self->offsetX;
	}

	e->y = self->y + self->offsetY;
}

static void addStoneCoat()
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Gargoyle Stone Coat");
	}

	loadProperties("boss/gargoyle_stone", e);

	e->x = self->x;
	e->y = self->y;

	e->action = &coatWait;

	e->draw = &drawLoopingAnimationToMap;
	e->touch = NULL;
	e->takeDamage = NULL;

	e->type = ENEMY;

	e->head = self;

	setEntityAnimation(e, "STAND");

	return e;
}

static void coatWait()
{
	if (self->head->mental == 1)
	{
		self->alpha--;

		if (self->alpha <= 0)
		{
			self->head->mental = 0;

			self->inUse = FALSE;
		}
	}

	self->x = self->head->x;
	self->y = self->head->y;
}
