/*
Copyright (C) 2009-2011 Parallel Realities

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
#include "../system/random.h"
#include "../audio/audio.h"
#include "../collisions.h"
#include "../graphics/decoration.h"
#include "../audio/music.h"
#include "../event/trigger.h"
#include "../hud.h"
#include "../system/error.h"
#include "../event/script.h"
#include "../event/trigger.h"
#include "../event/global_trigger.h"

extern Entity *self;

static void entityWait(void);
static void touch(Entity *);
static void activate(int);

Entity *addBlackBook3(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Black Book");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &entityWait;

	e->draw = &drawLoopingAnimationToMap;
	e->die = NULL;
	e->takeDamage = NULL;

	e->type = ENEMY;

	setEntityAnimation(e, "STAND");

	return e;
}

static void entityWait()
{
	int i;
	
	checkToMap(self);
	
	if (self->flags & ON_GROUND)
	{
		if (self->active == TRUE)
		{
			self->touch = &touch;
			
			self->activate = &activate;
		}
		
		if (self->mental == 0)
		{
			fireTrigger(self->objectiveName);

			fireGlobalTrigger(self->objectiveName);

			fadeBossMusic();

			for (i=0;i<20;i++)
			{
				addSmoke(self->x + prand() % self->w, self->y + self->h - prand() % 10, "decoration/dust");
			}
			
			playSoundToMap("sound/enemy/red_grub/thud.ogg", BOSS_CHANNEL, self->x, self->y, 0);
			
			self->mental = 1;
		}
	}
}

static void touch(Entity *other)
{
	setInfoBoxMessage(0, 255, 255, 255, _("Press Action to interact"));
}

static void activate(int val)
{
	runScript("black_book_search");
}
