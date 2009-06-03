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
#include "../system/properties.h"
#include "../entity.h"
#include "../system/random.h"
#include "../audio/audio.h"
#include "../map.h"
#include "../audio/music.h"
#include "../event/trigger.h"
#include "../item/key_items.h"
#include "../collisions.h"

extern Entity *self, player;

static void initialise(void);
static void doIntro(void);
static void die(void);
static void takeDamage(Entity *, int);

Entity *addGolemBoss(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		printf("No free slots to add the Golem Boss\n");

		exit(1);
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->action = &initialise;

	e->draw = &drawLoopingAnimationToMap;
	e->takeDamage = &takeDamage;
	e->die = &die;

	e->type = ENEMY;

	e->flags |= NO_DRAW|FLY;

	e->active = FALSE;

	setEntityAnimation(e, STAND);

	return e;
}

static void initialise()
{
	int minX, minY;

	minX = getMapStartX();
	minY = getMapStartY();
	
	if (self->active == TRUE)
	{
		adjustMusicVolume(-1);

		if (minX == self->endX && minY == self->endY)
		{
			centerMapOnEntity(&player);

			self->dirX = self->speed;

			/*setEntityAnimation(self, ATTACK_2);*/

			self->action = &doIntro;

			self->flags &= ~NO_DRAW;
			self->flags &= ~FLY;

			self->thinkTime = 300;

			printf("Starting\n");
		}
	}
}

static void doIntro()
{
	self->thinkTime--;

	if (self->thinkTime <= 0)
	{
		die();
	}
}

static void die()
{
	Entity *e;

	self->thinkTime--;

	self->takeDamage = NULL;

	printf("Dying %d\n", self->thinkTime);

	if (self->thinkTime <= 0)
	{
		setMinMapX(0);
		setMinMapX(0);

		fireTrigger(self->objectiveName);

		e = addKeyItem("item/heart_container", self->x + self->w / 2, self->y);

		e->dirY = ITEM_JUMP_HEIGHT;

		self->inUse = FALSE;
	}

	checkToMap(self);
}

static void takeDamage(Entity *other, int damage)
{

}
