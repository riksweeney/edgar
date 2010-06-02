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
#include "../graphics/graphics.h"
#include "../audio/audio.h"
#include "../system/properties.h"
#include "../entity.h"
#include "../hud.h"
#include "../collisions.h"
#include "../system/error.h"

extern Entity *self;

static void entityWait(void);
static void touch(Entity *);
static void activate(int);
static int energyBarDraw(void);
static void energyBarWait(void);
static void init(void);

Entity *addSoulMergerControlPanel(int x, int y, char *name)
{
	Entity *e = getFreeEntity();

	if (e == NULL)
	{
		showErrorAndExit("No free slots to add a Soul Merger Control Panel");
	}

	loadProperties(name, e);

	e->x = x;
	e->y = y;

	e->type = KEY_ITEM;

	e->action = &init;
	e->touch = &touch;
	e->activate = &activate;

	e->draw = &drawLoopingAnimationToMap;

	setEntityAnimation(e, STAND);

	return e;
}

static void entityWait()
{
	self->thinkTime--;
	
	if (self->thinkTime <= 0)
	{
		self->thinkTime = 0;
	}
	
	checkToMap(self);
}

static void touch(Entity *other)
{
	if (self->target->active == TRUE && self->mental == 0 && self->thinkTime == 0)
	{
		setInfoBoxMessage(0, 255, 255, 255, _("Press Action to modify the Soul Merger"));
	}
}

static void activate(int val)
{
	if (self->target->active == TRUE && self->mental == 0)
	{
		self->health--;
		
		if (self->health <= 0)
		{
			self->activate = NULL;
			
			self->health = 0;
			
			self->mental = 1;
		}
		
		self->thinkTime = 5;
	}
}

static void init()
{
	Entity *e = getFreeEntity();
	
	if (e == NULL)
	{
		showErrorAndExit("No free slots to add the Soul Merger Control Panel Energy Bar");
	}

	loadProperties("boss/awesome_boss_energy_bar", e);

	e->action = &energyBarWait;

	e->draw = &energyBarDraw;

	e->type = ENEMY;
	
	e->head = self;

	setEntityAnimation(e, STAND);
	
	self->action = &entityWait;
	
	self->target = getEntityByObjectiveName(self->requires);
	
	if (self->target == NULL)
	{
		showErrorAndExit("Control Panel cannot find Soul Merger %s", self->requires);
	}
}

static void energyBarWait()
{
	self->x = self->head->x - (self->w - self->head->w) / 2;
	self->y = self->head->y - self->head->h;
	
	if (self->health < self->head->health)
	{
		self->health += (self->head->health / 100);

		if (self->health > self->head->health)
		{
			self->health = self->head->health;
		}
	}

	else if (self->head->health < self->health)
	{
		self->health -= 3;

		if (self->health < self->head->health)
		{
			self->health = self->head->health;
		}
	}
}

static int energyBarDraw()
{
	int width;
	float percentage;
	
	if (self->head->thinkTime != 0)
	{
		drawLoopingAnimationToMap();
		
		percentage = self->health;
		percentage /= self->head->maxHealth;
		
		width = self->w - 2;
		
		width *= percentage;
		
		drawBoxToMap(self->x + 1, self->y + 1, width, self->h - 2, 0, 220, 0);
	}
	
	return TRUE;
}
