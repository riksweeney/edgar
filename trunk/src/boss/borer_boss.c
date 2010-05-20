/*
Copyright (C) 2009-2010 Parallel Realities

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
#include "../system/properties.h"
#include "../entity.h"
#include "../enemy/rock.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../custom_actions.h"
#include "../collisions.h"
#include "../projectile.h"
#include "../map.h"
#include "../game.h"
#include "../audio/music.h"
#include "../audio/audio.h"
#include "../graphics/gib.h"
#include "../item/key_items.h"
#include "../event/trigger.h"
#include "../hud.h"
#include "../inventory.h"
#include "../world/target.h"
#include "../player.h"
#include "../system/error.h"

extern Entity *self, player;

static void initialise(void);
static void doIntro(void);
static void introPause(void);
static void attackFinished(void);

Entity *addBorerBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Borer Boss");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;
	e->touch = NULL;

	e->draw = &drawLoopingAnimationToMap;

	e->type = ENEMY;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void initialise()
{
	if (self->active == TRUE)
	{
		self->action = &doIntro;
		
		setContinuePoint(FALSE, self->name, NULL);
		
		playSoundToMap("sound/boss/ant_lion/earthquake.ogg", BOSS_CHANNEL, self->x, self->y, -1);
		
		shakeScreen(LIGHT, -1);
	}
}

static void doIntro()
{
	self->x -=0.25;
	
	if (self->x <= self->startX)
	{
		self->x = self->startX;
		
		self->thinkTime = 60;
		
		shakeScreen(LIGHT, 0);
		
		stopSound(BOSS_CHANNEL);
		
		self->action = &introPause;
	}
}

static void introPause()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->takeDamage = NULL;

		self->action = &attackFinished;

		playBossMusic();

		initBossHealthBar();

		self->touch = &entityTouch;

		self->mental = 15;

		self->endY = self->y;
	}
}

static void attackFinished()
{
	
}
